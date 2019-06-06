// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkMaskFilter;
import org.skia.SkBlurStyle;

public class SkMaskFilter {
    public static native SkMaskFilter newBlur(SkBlurStyle blur, float sigma);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkMaskFilter.referenced();
        SkBlurStyle.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkMaskFilter(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
