
// A Record mutator that replaces SkImages backed by GPU textures with non-texture copies.
//
// TODO(nifong):
// If an image with the same gen id shows up again, it is replaced with the same non-texture
// copy as the first.

class SkImageCopyMutator {
public:

    // Defined for every record in SkRecord.h
    // Only has an effect for records that contain images.
    template <typename T>
    void operator()(T* record);
    sk_sp<SkImage> frozenImage(sk_sp<SkImage> im);
};