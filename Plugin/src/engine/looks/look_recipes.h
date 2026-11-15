// RWD COLOR LAB - 47 original creative look recipes (parametric, scene-linear Rec.2020 working space).
// These are artistic "inspired-by" looks authored for this plug-in, NOT measured film emulations.
#pragma once

namespace rwd {

struct LookRecipe {
    const char* collection;
    const char* name;
    float exposure;                 // stops
    float contrast;                 // power about 0.18 (1 = none)
    float saturation;               // global
    float satShadows, satHighlights;// saturation multipliers inside shadow / highlight masks
    float shadowTint[3];            // stops per channel, weighted by shadow mask
    float midTint[3];
    float highTint[3];
    float fade;                     // black lift 0..~0.05
    float shoulder;                 // highlight roll-off strength
    float hueDegrees;               // rotation about the grey axis
};

constexpr int kLookCount = 47;
constexpr int kCollectionCount = 6;
const LookRecipe& look_recipe(int zeroBasedIndex);
const char* collection_name(int collection);

// Applies one recipe to a scene-linear working-space triple (finite in, finite out; negatives kept).
void apply_recipe(const LookRecipe& r, float rgb[3]);

}  // namespace rwd
