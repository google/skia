// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkImageInfo;
import org.skia.SkData;
import org.skia.SkImage;
import org.skia.SkIRect;

public class SkImage {
    public native SkData encode();

    public static native SkImage newRasterCopy(SkImageInfo info, byte[] pixels);

    public static native SkImage newFromEncoded(SkData encoded, SkIRect subset);

    public native int getWidth();

    public native int getHeight();

    public native int getUniqueId();

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkImageInfo.referenced();
        SkData.referenced();
        SkImage.referenced();
        SkIRect.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkImage(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
