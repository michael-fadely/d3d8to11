#pragma once

#include <cstdint>

constexpr size_t LIGHT_COUNT           = 8;
constexpr size_t FVF_TEXCOORD_MAX      = 8;
constexpr size_t MAX_FRAGMENTS_DEFAULT = 32;

// preprocessor definition is used so that we can stringify this
#define TEXTURE_STAGE_MAX 8
