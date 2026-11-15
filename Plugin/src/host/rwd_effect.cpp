#include "host/rwd_effect.h"

#include <cstddef>
#include <memory>

#include "params/param_ids.h"
#include "render/cpu/cpu_renderer.h"

namespace rwd {

namespace {

// Maps an OFX image to the engine's pixel format. Returns false for anything the
// CPU engine does not support (half float, non-RGBA, custom formats).
bool to_engine_format(const OFX::ImageBase& img, PixelFormat& out) {
    if (img.getPixelComponents() != OFX::ePixelComponentRGBA) return false;
    switch (img.getPixelDepth()) {
        case OFX::eBitDepthFloat: out = PixelFormat::Float32; return true;
        case OFX::eBitDepthUShort: out = PixelFormat::UInt16; return true;
        case OFX::eBitDepthUByte: out = PixelFormat::UInt8; return true;
        default: return false;
    }
}

Bounds to_bounds(const OfxRectI& r) { return Bounds{r.x1, r.y1, r.x2, r.y2}; }

bool poll_abort(void* effect) { return static_cast<OFX::ImageEffect*>(effect)->abort(); }

}  // namespace

RwdEffect::RwdEffect(OfxImageEffectHandle handle)
    : OFX::ImageEffect(handle),
      dstClip_(fetchClip(kOfxImageEffectOutputClipName)),
      srcClip_(fetchClip(kOfxImageEffectSimpleSourceClipName)),
      params_(*this) {}

void RwdEffect::changedParam(const OFX::InstanceChangedArgs& args, const std::string& name) {
    if (name == param::kReset) {
        beginEditBlock("RWD COLOR LAB reset");
        params_.reset_all();
        endEditBlock();
    } else if (name == param::kPrevLook) {
        params_.step_look(-1);
    } else if (name == param::kNextLook) {
        params_.step_look(+1);
    }
    if (name == param::kReset || name == param::kCamera || name == param::kOutput) {
        const GradeSettings s = params_.read(args.time);
        params_.update_status(s);
        clearPersistentMessage();
        const char* w = profile_warning(s);
        if (w[0] != '\0') setPersistentMessage(OFX::Message::eMessageWarning, "", w);
    }
}

bool RwdEffect::isIdentity(const OFX::IsIdentityArguments& args, OFX::Clip*& identityClip,
                           double& identityTime) {
    const PreparedGrade grade = prepare_grade(params_.read(args.time));
    if (grade_is_noop(grade) && srcClip_ != nullptr && srcClip_->isConnected()) {
        identityClip = srcClip_;
        identityTime = args.time;
        return true;
    }
    return false;
}

void RwdEffect::render(const OFX::RenderArguments& args) {
    // 1. Acquire images. unique_ptr releases them (clipImageReleaseImage) on every path.
    std::unique_ptr<OFX::Image> dst(dstClip_->fetchImage(args.time));
    if (!dst) OFX::throwSuiteStatusException(kOfxStatFailed);

    std::unique_ptr<OFX::Image> src;
    if (srcClip_ != nullptr && srcClip_->isConnected()) src.reset(srcClip_->fetchImage(args.time));

    // 2. Validate pixel formats (never process what we do not understand).
    PixelFormat dstFormat = PixelFormat::Float32;
    if (!to_engine_format(*dst, dstFormat)) OFX::throwSuiteStatusException(kOfxStatErrFormat);

    ConstImageView srcView;
    const ConstImageView* srcPtr = nullptr;
    if (src) {
        PixelFormat srcFormat = PixelFormat::Float32;
        if (!to_engine_format(*src, srcFormat) || srcFormat != dstFormat)
            OFX::throwSuiteStatusException(kOfxStatErrFormat);
        srcView.data = static_cast<const unsigned char*>(src->getPixelData());
        srcView.bounds = to_bounds(src->getBounds());
        srcView.rowBytes = static_cast<std::ptrdiff_t>(src->getRowBytes());
        srcView.format = srcFormat;
        srcPtr = &srcView;
    }

    ImageView dstView;
    dstView.data = static_cast<unsigned char*>(dst->getPixelData());
    dstView.bounds = to_bounds(dst->getBounds());
    dstView.rowBytes = static_cast<std::ptrdiff_t>(dst->getRowBytes());
    dstView.format = dstFormat;

    // 3. Evaluate parameters at the frame time and render the requested window.
    const GradeSettings settings = params_.read(args.time);
    const PreparedGrade grade = prepare_grade(settings);
    if (grade.unavailable) setPersistentMessage(OFX::Message::eMessageWarning, "", profile_warning(settings));
    AbortCheck abort;
    abort.fn = &poll_abort;
    abort.user = this;

    const RenderResult result = render_window(srcPtr, dstView, to_bounds(args.renderWindow), grade, abort);
    if (result != RenderResult::Ok) {
        setPersistentMessage(OFX::Message::eMessageError, "", to_string(result));
        OFX::throwSuiteStatusException(kOfxStatFailed);
    }
}

}  // namespace rwd
