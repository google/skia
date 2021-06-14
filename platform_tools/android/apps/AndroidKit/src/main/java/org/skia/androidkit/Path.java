package org.skia.androidkit;

public class Path {
    private long mNativeInstance;

    public Path() {
        mNativeInstance = nCreate();
    }

    /**
     * Releases any resources associated with this Path.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    public void moveTo(float x, float y) {
        nMoveTo(mNativeInstance, x, y);
    }

    public void quadTo(float x1, float y1, float x2, float y2) {
        nQuadTo(mNativeInstance, x1, y1, x2, y2);
    }

    public void close() {
        nClose(mNativeInstance);
    }

    public enum FillType {
        WINDING           (0),
        EVEN_ODD          (1),
        INVERSE_WINDING   (2);

        FillType(int nativeInt) {
            this.nativeInt = nativeInt;
        }
        final int nativeInt;
    }
    public void setFillType(FillType fillType) {
        nSetFillType(mNativeInstance, fillType.nativeInt);
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate();
    private static native void nRelease(long nativeInstance);
    private static native void nMoveTo(long mNativeInstance, float x, float y);
    private static native void nQuadTo(long mNativeInstance, float x1, float y1, float x2, float y2);
    private static native void nClose(long mNativeInstance);
    private static native void nSetFillType(long mNativeInstance, int fillType);

}
