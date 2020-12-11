#include "src/core/SkImageCopyMutator.h"

sk_sp<SkImage> SkImageCopyMutator::frozenImage(sk_sp<SkImage> im) {
    if (im->isTextureBacked()) {
        return replacement = im->makeNonTextureImage();
    }
};

// operations that don't reference images
template <> void SkImageCopyMutator::operator(const NoOp& r) {}
template <> void SkImageCopyMutator::operator(const Flush& r) {}
template <> void SkImageCopyMutator::operator(const Restore& r) {}
template <> void SkImageCopyMutator::operator(const Save& r) {}
template <> void SkImageCopyMutator::operator(const SaveBehind& r) {}
template <> void SkImageCopyMutator::operator(const MarkCTM& r) {}
template <> void SkImageCopyMutator::operator(const SetMatrix& r) {}
template <> void SkImageCopyMutator::operator(const SetM44& r) {}
template <> void SkImageCopyMutator::operator(const Concat& r) {}
template <> void SkImageCopyMutator::operator(const Concat44& r) {}
template <> void SkImageCopyMutator::operator(const Translate& r) {}
template <> void SkImageCopyMutator::operator(const Scale& r) {}
template <> void SkImageCopyMutator::operator(const ClipPath& r) {}
template <> void SkImageCopyMutator::operator(const ClipRRect& r) {}
template <> void SkImageCopyMutator::operator(const ClipRegion& r) {}

// operations that directly reference images
template <> void SkImageCopyMutator::operator(const DrawImage& r) {
    r.image = frozenImage(r.image);
}
template <> void SkImageCopyMutator::operator(const DrawImageLattice& r) {
    r.image = frozenImage(r.image);
}
template <> void SkImageCopyMutator::operator(const DrawImageRect& r) {
    r.image = frozenImage(r.image);
}
template <> void SkImageCopyMutator::operator(const DrawImageNine& r) {
    r.image = frozenImage(r.image);
}
template <> void SkImageCopyMutator::operator(const DrawAtlas& r) { // image, paint
    r.atlas = frozenImage(r.atlas);
}
template <> void SkImageCopyMutator::operator(const DrawEdgeAAImageSet& r) {} // paint, contains array of ImageSetEntry which contains image

// operations that have paints, which can have shaders, which can reference images
template <> void SkImageCopyMutator::operator(const DrawArc& r) {} // Paint
template <> void SkImageCopyMutator::operator(const DrawDRRect& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawOval& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawBehind& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawPath& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawPoints& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawRRect& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawRect& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawRegion& r) {}  // paint
template <> void SkImageCopyMutator::operator(const DrawTextBlob& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawPatch& r) {} // paint
template <> void SkImageCopyMutator::operator(const DrawVertices& r) {} // paint

// other cases where images could be present
template <> void SkImageCopyMutator::operator(const SaveLayer& r) {} // backdrop is an image filter, and there is a type of image filter that can contain an image
template <> void SkImageCopyMutator::operator(const ClipShader& r) {} // images may be present in shaders
template <> void SkImageCopyMutator::operator(const DrawPicture& r) {} // paint and indirect
template <> void SkImageCopyMutator::operator(const DrawDrawable& r) {} // indirect