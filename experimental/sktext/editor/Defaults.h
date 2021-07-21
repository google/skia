// Copyright 2021 Google LLC.
#ifndef Defaults_DEFINED
#define Defaults_DEFINED

#include "experimental/sktext/include/Types.h"
#include "include/core/SkColor.h"

namespace skia {
namespace editor {

using namespace skia::text;

const TextDirection DEFAULT_TEXT_DIRECTION = TextDirection::kLtr;
const TextAlign DEFAULT_TEXT_ALIGN = TextAlign::kLeft;
const SkColor DEFAULT_TEXT_FOREGROUND = SK_ColorBLACK;
const SkColor DEFAULT_TEXT_BACKGROUND = SK_ColorWHITE;
const SkColor DEFAULT_STATUS_FOREGROUND = SK_ColorRED;
const SkColor DEFAULT_STATUS_BACKGROUND = SK_ColorLTGRAY;
const SkScalar DEFAULT_CURSOR_WIDTH = 2;
const SkColor DEFAULT_CURSOR_COLOR = SK_ColorGRAY;
const SkColor DEFAULT_SELECTION_COLOR = SK_ColorCYAN;
const SkScalar DEFAULT_STATUS_HEIGHT = 25;

}   // namespace editor
}   // namespace skia
#endif // Defaults_DEFINED
