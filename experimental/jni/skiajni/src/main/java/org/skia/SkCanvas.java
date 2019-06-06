// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkPaint;
import org.skia.SkPath;
import org.skia.SkRect;
import org.skia.SkImage;
import org.skia.SkMatrix;
import org.skia.SkPicture;

public class SkCanvas {
    public native void drawPaint(SkPaint paint);

    public native void drawRect(SkRect rect, SkPaint paint);

    public native void drawPath(SkPath path, SkPaint paint);

    public native void drawOval(SkRect rect, SkPaint paint);

    public native void save();

    public native void saveLayer(SkRect rect, SkPaint paint);

    public native void restore();

    public native void translate(float dx, float dy);

    public native void scale(float sx, float sy);

    public native void rotateRadians(float radians);

    public native void skew(float sx, float sy);

    public native void concat(SkMatrix matrix);

    public native void clipRect(SkRect rect);

    public native void clipPath(SkPath path);

    public native void drawCircle(float cx, float cy, float rad, SkPaint paint);

    public native void drawImage(SkImage image, float x, float y, SkPaint paint);

    public native void drawImageRect(SkImage image, SkRect src, SkRect dst, SkPaint paint);

    public native void drawPicture(SkPicture picture, SkMatrix matrix, SkPaint paint);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkPaint.referenced();
        SkPath.referenced();
        SkRect.referenced();
        SkImage.referenced();
        SkMatrix.referenced();
        SkPicture.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkCanvas(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
