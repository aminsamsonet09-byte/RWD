// RWD COLOR LAB - OFX effect instance (one per node in Resolve).
#pragma once

#include <memory>

#include "ofxsImageEffect.h"
#include "params/param_reader.h"

namespace rwd {

class RwdEffect : public OFX::ImageEffect {
public:
    explicit RwdEffect(OfxImageEffectHandle handle);
    ~RwdEffect() override = default;

    void render(const OFX::RenderArguments& args) override;
    void changedParam(const OFX::InstanceChangedArgs& args, const std::string& paramName) override;
    bool isIdentity(const OFX::IsIdentityArguments& args, OFX::Clip*& identityClip,
                    double& identityTime) override;

private:
    OFX::Clip* dstClip_ = nullptr;
    OFX::Clip* srcClip_ = nullptr;
    GradeParams params_;
};

}  // namespace rwd
