#include "SkRecordAnalysis.h"

#include "SkShader.h"
#include "SkTLogic.h"

/** SkRecords visitor to determine whether an instance may require an
    "external" bitmap to rasterize. May return false positives.
    Does not return true for bitmap text.

    Expected use is to determine whether images need to be decoded before
    rasterizing a particular SkRecord.
 */
struct BitmapTester {
    // Helpers.  These create HasMember_bitmap and HasMember_paint.
    SK_CREATE_MEMBER_DETECTOR(bitmap);
    SK_CREATE_MEMBER_DETECTOR(paint);

    // Some commands have a paint, some have an optional paint.  Either way, get back a pointer.
    static const SkPaint* AsPtr(const SkPaint& p) { return &p; }
    static const SkPaint* AsPtr(const SkRecords::Optional<SkPaint>& p) { return p; }


    // Main entry for visitor:
    // If the command has a bitmap directly, return true.
    // If the command has a paint and the paint has a bitmap, return true.
    // Otherwise, return false.
    template <typename T>
    bool operator()(const T& r) { return CheckBitmap(r); }


    // If the command has a bitmap, of course we're going to play back bitmaps.
    template <typename T>
    static SK_WHEN(HasMember_bitmap<T>, bool) CheckBitmap(const T&) { return true; }

    // If not, look for one in its paint (if it has a paint).
    template <typename T>
    static SK_WHEN(!HasMember_bitmap<T>, bool) CheckBitmap(const T& r) { return CheckPaint(r); }

    // If we have a paint, dig down into the effects looking for a bitmap.
    template <typename T>
    static SK_WHEN(HasMember_paint<T>, bool) CheckPaint(const T& r) {
        const SkPaint* paint = AsPtr(r.paint);
        if (paint) {
            const SkShader* shader = paint->getShader();
            if (shader &&
                shader->asABitmap(NULL, NULL, NULL) == SkShader::kDefault_BitmapType) {
                return true;
            }
        }
        return false;
    }

    // If we don't have a paint, that non-paint has no bitmap.
    template <typename T>
    static SK_WHEN(!HasMember_paint<T>, bool) CheckPaint(const T&) { return false; }
};

bool SkRecordWillPlaybackBitmaps(const SkRecord& record) {
    BitmapTester tester;
    for (unsigned i = 0; i < record.count(); i++) {
        if (record.visit<bool>(i, tester)) {
            return true;
        }
    }
    return false;
}
