package org.skia.androidkit;

public class ToyFont {
    private long mNativeInstance;

    public ToyFont(String fontFamily, float size) {
        mNativeInstance = nCreate(fontFamily, size);
    }

    /**
     * Releases any resources associated with this Paint.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate(String fontFamily, float size);
    private static native void nRelease(long nativeFont);

}