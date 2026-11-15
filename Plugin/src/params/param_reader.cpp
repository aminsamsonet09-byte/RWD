#include "params/param_reader.h"

#include "params/param_ids.h"

namespace rwd {

GradeParams::GradeParams(OFX::ImageEffect& e)
    : camera(e.fetchChoiceParam(param::kCamera)),
      output(e.fetchChoiceParam(param::kOutput)),
      look(e.fetchChoiceParam(param::kLook)),
      intensity(e.fetchDoubleParam(param::kIntensity)),
      exposure(e.fetchDoubleParam(param::kExposure)),
      contrast(e.fetchDoubleParam(param::kContrast)),
      saturation(e.fetchDoubleParam(param::kSaturation)),
      temperature(e.fetchDoubleParam(param::kTemperature)),
      tint(e.fetchDoubleParam(param::kTint)),
      highlights(e.fetchDoubleParam(param::kHighlights)),
      shadows(e.fetchDoubleParam(param::kShadows)),
      master(e.fetchDoubleParam(param::kMasterAmount)),
      bypass(e.fetchBooleanParam(param::kBypass)),
      status(e.fetchStringParam(param::kStatus)) {}

GradeSettings GradeParams::read(double t) const {
    GradeSettings s;
    { int v = 0; camera->getValueAtTime(t, v); s.camera = v; }
    { int v = 0; output->getValueAtTime(t, v); s.output = v; }
    { int v = 0; look->getValueAtTime(t, v); s.look = v; }
    s.intensityPercent = intensity->getValueAtTime(t);
    s.exposureStops = exposure->getValueAtTime(t);
    s.contrast = contrast->getValueAtTime(t);
    s.saturationPercent = saturation->getValueAtTime(t);
    s.temperature = temperature->getValueAtTime(t);
    s.tint = tint->getValueAtTime(t);
    s.highlights = highlights->getValueAtTime(t);
    s.shadows = shadows->getValueAtTime(t);
    s.masterAmountPercent = master->getValueAtTime(t);
    s.bypass = bypass->getValueAtTime(t);
    return s;
}

void GradeParams::reset_all() {
    camera->setValue(0);
    output->setValue(0);
    look->setValue(0);
    intensity->setValue(100.0);
    exposure->setValue(kExposureDefaultStops);
    contrast->setValue(0.0);
    saturation->setValue(100.0);
    temperature->setValue(0.0);
    tint->setValue(0.0);
    highlights->setValue(0.0);
    shadows->setValue(0.0);
    master->setValue(kMasterAmountDefaultPercent);
    bypass->setValue(false);
}

void GradeParams::step_look(int delta) {
    int cur = 0;
    look->getValue(cur);
    int v = cur + delta;
    v %= kLookChoiceCount;
    if (v < 0) v += kLookChoiceCount;
    look->setValue(v);
}

void GradeParams::update_status(const GradeSettings& s) {
    status->setValue(profile_warning(s));
}

}  // namespace rwd
