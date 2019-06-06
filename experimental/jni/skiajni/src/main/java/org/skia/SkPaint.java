// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkShader;
import org.skia.SkMaskFilter;
import org.skia.SkStrokeJoin;
import org.skia.SkStrokeCap;
import org.skia.SkXfermodemode;

public class SkPaint {
    public native void setColor(int color);

    public native void setAntiAlias(boolean a);

    public native void setStroke(boolean doStroke);

    public native void setStrokeWidth(float width);

    private native void nativeInit();

    public SkPaint() {
        nativeInit();
    }

    public native boolean isAntiAlias();

    public native int getColor();

    public native boolean isStroke();

    public native float getStrokeWidth();

    public native float getStrokeMiter();

    public native void setStrokeMiter(float miter);

    public native SkStrokeCap getStrokeCap();

    public native void setStrokeCap(SkStrokeCap cap);

    public native SkStrokeJoin getStrokeJoin();

    public native void setStrokeJoin(SkStrokeJoin cap);

    public native void setShader(SkShader shader);

    public native void setMaskFilter(SkMaskFilter filter);

    public native void setXfermodeMode(SkXfermodemode mode);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkShader.referenced();
        SkMaskFilter.referenced();
        SkStrokeJoin.referenced();
        SkStrokeCap.referenced();
        SkXfermodemode.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkPaint(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
