package org.skia.androidkit;

import android.graphics.Bitmap;

public class Surface {
    private long mNativeInstance;

    /**
     * Create a Surface backed by the provided Bitmap.
     * 
     * The Bitmap must be mutable and its pixels are locked for the lifetime of the Surface.
     */
    public Surface(Bitmap bitmap) {
        if (!bitmap.isMutable()) {
            throw new IllegalStateException("Immutable bitmap passed to Surface constructor");
        }

        mNativeInstance = nCreateBitmap(bitmap);
    }

    /**
     * Releases any resources associated with this Surface.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable
    {
        release();
    }

    // package private
    long getNativeCanvas() {
        return nGetNativeCanvas(mNativeInstance);
    }

    private static native long nCreateBitmap(Bitmap bitmap);
    private static native void nRelease(long nativeInstance);
    private static native long nGetNativeCanvas(long nativeInstance);
}
