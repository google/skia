package org.skia.jetski;

public abstract class FontChain {
    long mNativeAdapter;

    FontChain() {
        mNativeAdapter = nCreate(this);
    }

    /**
     * Releases any resources associated with this FontChain.
     */
    public void release() {
        nRelease(mNativeAdapter);
        mNativeAdapter = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    abstract int count();
    abstract long getAt(int i);
    abstract float fontSize();
    abstract String locale();

    private static native long nCreate(FontChain fontChain);
    private static native void nRelease(long nativeInstance);
}
