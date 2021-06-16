package org.skia.androidkit;

public class PathBuilder {
    private long mNativeInstance;

    public PathBuilder() {
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

    public void lineTo(float x, float y) {
        nLineTo(mNativeInstance, x, y);
    }

    public void quadTo(float x1, float y1, float x2, float y2) {
        nQuadTo(mNativeInstance, x1, y1, x2, y2);
    }
    public void conicTo(float x1, float y1, float x2, float y2, float w) {
        nConicTo(mNativeInstance, x1, y1, x2, y2, w);
    }
    public void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
        nCubicTo(mNativeInstance, x1, y1, x2, y2, x3, y3);
    }

    public void close() {
        nClose(mNativeInstance);
    }

    public enum FillType {
        WINDING           (0),
        EVEN_ODD          (1),
        INVERSE_WINDING   (2),
        INVERSE_EVEN_ODD  (3);


        FillType(int nativeInt) {
            this.nativeInt = nativeInt;
        }
        final int nativeInt;
    }
    public void setFillType(FillType fillType) {
        nSetFillType(mNativeInstance, fillType.nativeInt);
    }

    /*
     * Returns a path from the builder, resets the builder to empty.
     * Wrapper for SkPath::detach()
     */
    public Path makePath() {
        return new Path(nMake(mNativeInstance));
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate();
    private static native void nRelease(long nativeInstance);
    private static native void nMoveTo(long mNativeInstance, float x, float y);
    private static native void nLineTo(long mNativeInstance, float x, float y);
    private static native void nQuadTo(long mNativeInstance, float x1, float y1, float x2, float y2);
    private static native void nConicTo(long mNativeInstance, float x1, float y1, float x2, float y2, float w);
    private static native void nCubicTo(long mNativeInstance, float x1, float y1, float x2, float y2, float x3, float y3);
    private static native void nClose(long mNativeInstance);
    private static native void nSetFillType(long mNativeInstance, int fillType);
    private static native long nMake(long mNativeInstance);


}
