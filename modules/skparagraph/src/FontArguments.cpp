// Copyright 2019 Google LLC.

#include "modules/skparagraph/include/FontArguments.h"

static bool operator==(const SkFontArguments::VariationPosition::Coordinate& a,
                       const SkFontArguments::VariationPosition::Coordinate& b) {
   return a.axis == b.axis && a.value == b.value;
}

static bool operator==(const SkFontArguments::Palette::Override& a,
                       const SkFontArguments::Palette::Override& b) {
   return a.index == b.index && a.color == b.color;
}

namespace std {

size_t hash<skia::textlayout::FontArguments>::operator()(const skia::textlayout::FontArguments& args) const {
    size_t hash = 0;
    hash ^= std::hash<int>()(args.fCollectionIndex);
    for (const auto& coord : args.fCoordinates) {
        hash ^= std::hash<SkFourByteTag>()(coord.axis);
        hash ^= std::hash<float>()(coord.value);
    }
    hash ^= std::hash<int>()(args.fPaletteIndex);
    for (const auto& override : args.fPaletteOverrides) {
        hash ^= std::hash<int>()(override.index);
        hash ^= std::hash<SkColor>()(override.color);
    }
    return hash;
}

}  // namespace std

namespace skia {
namespace textlayout {

FontArguments::FontArguments(const SkFontArguments& args)
        : fCollectionIndex(args.getCollectionIndex()),
          fCoordinates(args.getVariationDesignPosition().coordinates,
                       args.getVariationDesignPosition().coordinates +
                       args.getVariationDesignPosition().coordinateCount),
          fPaletteIndex(args.getPalette().index),
          fPaletteOverrides(args.getPalette().overrides,
                            args.getPalette().overrides +
                            args.getPalette().overrideCount) {}

bool operator==(const FontArguments& a, const FontArguments& b) {
    return a.fCollectionIndex == b.fCollectionIndex &&
           a.fCoordinates == b.fCoordinates &&
           a.fPaletteIndex == b.fPaletteIndex &&
           a.fPaletteOverrides == b.fPaletteOverrides;
}

bool operator!=(const skia::textlayout::FontArguments& a, const skia::textlayout::FontArguments& b) {
    return !(a == b);
}

sk_sp<SkTypeface> FontArguments::CloneTypeface(sk_sp<SkTypeface> typeface) const {
    SkFontArguments::VariationPosition position{
        fCoordinates.data(),
        static_cast<int>(fCoordinates.size())
    };

    SkFontArguments::Palette palette{
        fPaletteIndex,
        fPaletteOverrides.data(),
        static_cast<int>(fPaletteOverrides.size())
    };

    SkFontArguments args;
    args.setCollectionIndex(fCollectionIndex);
    args.setVariationDesignPosition(position);
    args.setPalette(palette);

    return typeface->makeClone(args);
}

}  // namespace textlayout
}  // namespace skia
