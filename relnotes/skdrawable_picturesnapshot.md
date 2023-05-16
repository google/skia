`SkDrawable::newPictureSnapshot` is removed. Instead, call `SkDrawable::makePictureSnapshot`.
The old method returned a bare (but ref-counted) pointer, which was easy for clients to get wrong.
The new method returns an `sk_sp<SkPicture>`, which is easier to handle, and consistent with the
rest of skia.