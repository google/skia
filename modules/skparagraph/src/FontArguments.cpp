// Copyright 2019 Google LLC.

#include "modules/skparagraph/src/FontArguments.h"

bool operator==(const SkFontArguments& a, const SkFontArguments& b) {
    if (a.getCollectionIndex() != b.getCollectionIndex()) {
        return false;
    }

    const SkFontArguments::VariationPosition posA = a.getVariationDesignPosition();
    const SkFontArguments::VariationPosition posB = b.getVariationDesignPosition();
    if (posA.coordinateCount != posB.coordinateCount) {
        return false;
    }
    for (int i = 0; i < posA.coordinateCount; ++i) {
        const SkFontArguments::VariationPosition::Coordinate& coordA = posA.coordinates[i];
        const SkFontArguments::VariationPosition::Coordinate& coordB = posB.coordinates[i];
        if (coordA.axis != coordB.axis || coordA.value != coordB.value) {
            return false;
        }
    }

    const SkFontArguments::Palette palA = a.getPalette();
    const SkFontArguments::Palette palB = b.getPalette();
    if (palA.index != palB.index || palA.overrideCount != palB.overrideCount) {
        return false;
    }
    for (int i = 0; i < palA.overrideCount; ++i) {
        const SkFontArguments::Palette::Override& overA = palA.overrides[i];
        const SkFontArguments::Palette::Override& overB = palB.overrides[i];
        if (overA.index != overB.index || overA.color != overB.color) {
            return false;
        }
    }

    return true;
}

bool operator!=(const SkFontArguments& a, const SkFontArguments& b) {
    return !(a == b);
}

namespace std {
    size_t hash<SkFontArguments>::operator()(const SkFontArguments& args) const {
        size_t hash = 0;
        hash ^= std::hash<int>()(args.getCollectionIndex());
        const SkFontArguments::VariationPosition pos = args.getVariationDesignPosition();
        for (int i = 0; i < pos.coordinateCount; ++i) {
            hash ^= std::hash<SkFourByteTag>()(pos.coordinates[i].axis);
            hash ^= std::hash<float>()(pos.coordinates[i].value);
        }
        const SkFontArguments::Palette pal = args.getPalette();
        hash ^= std::hash<int>()(pal.index);
        for (int i = 0; i < pal.overrideCount; ++i) {
            hash ^= std::hash<int>()(pal.overrides[i].index);
            hash ^= std::hash<SkColor>()(pal.overrides[i].color);
        }
        return hash;
    }
}
