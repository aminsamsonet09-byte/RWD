// RWD COLOR LAB - reads OFX parameter values into the engine's GradeSettings.
#pragma once

#include "engine/grade_pipeline.h"
#include "ofxsImageEffect.h"

namespace rwd {

class GradeParams {
public:
    explicit GradeParams(OFX::ImageEffect& effect);

    GradeSettings read(double t) const;   // evaluated at timeline time t
    void reset_all();                     // every control back to its default
    void step_look(int delta);            // Prev/Next, wraps through "None" + 47 looks
    void update_status(const GradeSettings& s);

    OFX::ChoiceParam* camera = nullptr;
    OFX::ChoiceParam* output = nullptr;
    OFX::ChoiceParam* look = nullptr;
    OFX::DoubleParam *intensity = nullptr, *exposure = nullptr, *contrast = nullptr, *saturation = nullptr,
                     *temperature = nullptr, *tint = nullptr, *highlights = nullptr, *shadows = nullptr,
                     *master = nullptr;
    OFX::BooleanParam* bypass = nullptr;
    OFX::StringParam* status = nullptr;
};

}  // namespace rwd
