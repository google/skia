// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkMatrix;
import org.skia.SkShader;
import org.skia.SkPoint;
import org.skia.SkShaderTileMode;

public class SkShader {
    public static native SkShader newLinearGradient(SkPoint startPoint, SkPoint endPoint, int[] colors, float[] colorPos, int colorCount, SkShaderTileMode tileMode, SkMatrix localMatrix);

    public static native SkShader newRadialGradient(SkPoint center, float radius, int[] colors, float[] colorPos, int colorCount, SkShaderTileMode tileMode, SkMatrix localMatrix);

    public static native SkShader newSweepGradient(SkPoint center, int[] colors, float[] colorPos, int colorCount, SkMatrix localMatrix);

    public static native SkShader newTwoPointConicalGradient(SkPoint startPoint, float startRadius, SkPoint endPoint, float endRadius, int[] colors, float[] colorPos, int colorCount, SkShaderTileMode tileMode, SkMatrix localMatrix);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkMatrix.referenced();
        SkShader.referenced();
        SkPoint.referenced();
        SkShaderTileMode.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkShader(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
