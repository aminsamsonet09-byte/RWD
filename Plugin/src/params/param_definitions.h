// RWD COLOR LAB - declares the OFX parameters (single source of option lists = the engine tables).
#pragma once

#include <string>

#include "ofxsImageEffect.h"

namespace rwd {

void define_parameters(OFX::ImageEffectDescriptor& desc);
std::string look_choice_label(int choice);   // 0 = "None"

}  // namespace rwd
