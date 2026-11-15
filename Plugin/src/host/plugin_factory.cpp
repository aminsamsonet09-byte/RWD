// RWD COLOR LAB - plug-in registration, describe and describeInContext.
//
// The exported OFX entry points (OfxGetNumberOfPlugins / OfxGetPlugin /
// OfxSetHost) are provided by the official OpenFX Support library, which calls
// OFX::Plugin::getPluginIDs() below.

#include "host/plugin_info.h"
#include "host/rwd_effect.h"
#include "ofxsImageEffect.h"
#include "params/param_definitions.h"

namespace {

mDeclarePluginFactory(RwdPluginFactory, {}, {});

void RwdPluginFactory::describe(OFX::ImageEffectDescriptor& desc) {
    desc.setLabels(rwd::kPluginLabel, rwd::kPluginLabel, rwd::kPluginLabel);
    desc.setPluginGrouping(rwd::kPluginGrouping);
    desc.setPluginDescription(rwd::kPluginDescription);

    desc.addSupportedContext(OFX::eContextFilter);
    desc.addSupportedContext(OFX::eContextGeneral);

    desc.addSupportedBitDepth(OFX::eBitDepthFloat);
    desc.addSupportedBitDepth(OFX::eBitDepthUShort);
    desc.addSupportedBitDepth(OFX::eBitDepthUByte);

    desc.setSingleInstance(false);
    desc.setHostFrameThreading(false);
    desc.setSupportsMultiResolution(true);
    desc.setSupportsTiles(true);
    desc.setTemporalClipAccess(false);
    desc.setRenderTwiceAlways(false);
    desc.setSupportsMultipleClipPARs(false);
    desc.setSupportsMultipleClipDepths(false);
    // render() keeps no mutable per-instance state, so the host may call it concurrently.
    desc.setRenderThreadSafety(OFX::eRenderFullySafe);
}

void RwdPluginFactory::describeInContext(OFX::ImageEffectDescriptor& desc, OFX::ContextEnum /*context*/) {
    OFX::ClipDescriptor* srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->addSupportedComponent(OFX::ePixelComponentRGBA);
    srcClip->setTemporalClipAccess(false);
    srcClip->setSupportsTiles(true);
    srcClip->setIsMask(false);

    OFX::ClipDescriptor* dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(OFX::ePixelComponentRGBA);
    dstClip->setSupportsTiles(true);

    rwd::define_parameters(desc);
}

OFX::ImageEffect* RwdPluginFactory::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum /*context*/) {
    return new rwd::RwdEffect(handle);
}

}  // namespace

void OFX::Plugin::getPluginIDs(OFX::PluginFactoryArray& ids) {
    static RwdPluginFactory factory(rwd::kPluginIdentifier, rwd::kPluginVersionMajor, rwd::kPluginVersionMinor);
    ids.push_back(&factory);
}
