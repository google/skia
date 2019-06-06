// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkRect;

public class SkPicture {
    public native int getUniqueId();

    public native SkRect getBounds();

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkRect.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkPicture(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
