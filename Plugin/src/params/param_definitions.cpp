#include "params/param_definitions.h"

#include <string>

#include "engine/grade_pipeline.h"
#include "params/param_ids.h"

namespace rwd {

namespace {

OFX::GroupParamDescriptor* make_group(OFX::ImageEffectDescriptor& d, OFX::PageParamDescriptor* page,
                                      const char* id, const char* label, bool open) {
    OFX::GroupParamDescriptor* g = d.defineGroupParam(id);
    g->setLabel(label);
    g->setOpen(open);
    page->addChild(*g);
    return g;
}

void slider(OFX::ImageEffectDescriptor& d, OFX::PageParamDescriptor* page, OFX::GroupParamDescriptor* g,
            const char* id, const char* label, const char* script, const char* hint, double def, double lo,
            double hi, double inc) {
    OFX::DoubleParamDescriptor* p = d.defineDoubleParam(id);
    p->setLabel(label);
    p->setScriptName(script);
    p->setHint(hint);
    p->setDoubleType(OFX::eDoubleTypePlain);
    p->setDefault(def);
    p->setRange(lo, hi);
    p->setDisplayRange(lo, hi);
    p->setIncrement(inc);
    p->setAnimates(true);
    p->setParent(*g);
    page->addChild(*p);
}

}  // namespace

std::string look_choice_label(int choice) {
    if (choice <= 0 || choice > kLookCount) return "None";
    const LookRecipe& r = look_recipe(choice - 1);
    return std::string(r.collection) + " / " + r.name;
}

void define_parameters(OFX::ImageEffectDescriptor& desc) {
    OFX::PageParamDescriptor* page = desc.definePageParam(param::kPageMain);
    page->setLabel("RWD COLOR LAB");

    // --- Conversion -----------------------------------------------------------
    OFX::GroupParamDescriptor* conv = make_group(desc, page, param::kGroupConversion, "Conversion", true);
    {
        OFX::ChoiceParamDescriptor* p = desc.defineChoiceParam(param::kCamera);
        p->setLabel("Conversion LUT (input)");
        p->setScriptName("conversionInput");
        p->setHint("What the footage is: camera log/gamut, or already Rec.709. Entries marked experimental "
                   "or unavailable are explained in CAMERA_PROFILES.md.");
        for (int i = 0; i < kCameraProfileCount; ++i) p->appendOption(camera_profile(i).label);
        p->setDefault(0);
        p->setAnimates(false);
        p->setParent(*conv);
        page->addChild(*p);
    }
    {
        OFX::ChoiceParamDescriptor* p = desc.defineChoiceParam(param::kOutput);
        p->setLabel("Conversion output");
        p->setScriptName("conversionOutput");
        p->setHint("Display transform applied at the end. Stays active at any Master Amount or Intensity.");
        for (int i = 0; i < kOutputProfileCount; ++i) p->appendOption(output_profile(i).label);
        p->setDefault(0);
        p->setAnimates(false);
        p->setParent(*conv);
        page->addChild(*p);
    }
    {
        OFX::StringParamDescriptor* p = desc.defineStringParam(param::kStatus);
        p->setLabel("Profile status");
        p->setScriptName("profileStatus");
        p->setStringType(OFX::eStringTypeLabel);
        p->setDefault("");
        p->setEnabled(false);
        p->setEvaluateOnChange(false);
        p->setIsPersistant(false);
        p->setParent(*conv);
        page->addChild(*p);
    }

    // --- Creative look --------------------------------------------------------
    OFX::GroupParamDescriptor* look = make_group(desc, page, param::kGroupLook, "Creative LUT", true);
    {
        OFX::ChoiceParamDescriptor* p = desc.defineChoiceParam(param::kLook);
        p->setLabel("Creative LUT");
        p->setScriptName("creativeLut");
        p->setHint("47 original looks in 6 collections. Applied in the working space before the output transform.");
        for (int i = 0; i < kLookChoiceCount; ++i) p->appendOption(look_choice_label(i));
        p->setDefault(0);
        p->setAnimates(false);
        p->setParent(*look);
        page->addChild(*p);
    }
    {
        OFX::PushButtonParamDescriptor* p = desc.definePushButtonParam(param::kPrevLook);
        p->setLabel("Prev LUT");
        p->setScriptName("prevLut");
        p->setParent(*look);
        page->addChild(*p);
    }
    {
        OFX::PushButtonParamDescriptor* p = desc.definePushButtonParam(param::kNextLook);
        p->setLabel("Next LUT");
        p->setScriptName("nextLut");
        p->setParent(*look);
        page->addChild(*p);
    }
    slider(desc, page, look, param::kIntensity, "Intensity", "intensity",
           "Creative LUT strength, 0-100%. Does not affect camera or output conversion.", 100, 0, 100, 1);

    // --- Basic adjustments ------------------------------------------------------
    OFX::GroupParamDescriptor* basic = make_group(desc, page, param::kGroupBasic, "Basic Adjustments", true);
    slider(desc, page, basic, param::kExposure, "Exposure",   "exposure",
           "Exposure in stops (EV), applied on scene-linear data. -4 to +4.", kExposureDefaultStops, kExposureMinStops, kExposureMaxStops, 0.05);
    slider(desc, page, basic, param::kContrast, "Contrast", "contrast", "Contrast about mid grey (0.18 linear).", 0, -100, 100, 1);
    slider(desc, page, basic, param::kSaturation, "Saturation", "saturation", "0-200%. 100% = unchanged.", 100, 0, 200, 1);
    slider(desc, page, basic, param::kTemperature, "Temperature", "temperature", "Creative warm/cool shift (not Kelvin-accurate).", 0, -100, 100, 1);
    slider(desc, page, basic, param::kTint, "Tint", "tint", "Creative green/magenta shift.", 0, -100, 100, 1);
    slider(desc, page, basic, param::kHighlights, "Highlights", "highlights", "Negative recovers highlights, positive brightens them.", 0, -100, 100, 1);
    slider(desc, page, basic, param::kShadows, "Shadows", "shadows", "Positive lifts shadows, negative deepens them.", 0, -100, 100, 1);

    // --- Global -----------------------------------------------------------------
    OFX::GroupParamDescriptor* global = make_group(desc, page, param::kGroupGlobal, "Global Controls", true);
    slider(desc, page, global, param::kMasterAmount, "Master Amount", "masterAmount",
           "Scales the whole grade (adjustments + Creative LUT), 0-100%. Camera and output conversion stay active.",
           kMasterAmountDefaultPercent, kMasterAmountMinPercent, kMasterAmountMaxPercent, 1);
    {
        OFX::BooleanParamDescriptor* p = desc.defineBooleanParam(param::kBypass);
        p->setLabel("Bypass");
        p->setScriptName("bypass");
        p->setHint("Return the original input image, with no processing at all.");
        p->setDefault(false);
        p->setAnimates(true);
        p->setParent(*global);
        page->addChild(*p);
    }
    {
        OFX::PushButtonParamDescriptor* p = desc.definePushButtonParam(param::kReset);
        p->setLabel("Reset");
        p->setScriptName("reset");
        p->setHint("Reset every control to its default (conversion choices included).");
        p->setParent(*global);
        page->addChild(*p);
    }
}

}  // namespace rwd
