// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkSurfaceProps;
import org.skia.SkSurface;
import org.skia.SkImageInfo;
import org.skia.SkCanvas;
import org.skia.SkImage;

public class SkSurface {
    public static native SkSurface newRaster(SkImageInfo info, SkSurfaceProps props);

    public native SkCanvas getCanvas();

    public native SkImage newImageSnapshot();

    public static native SkSurface newRasterDirect(SkImageInfo info, byte[] pixels, SkSurfaceProps props);

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkSurfaceProps.referenced();
        SkSurface.referenced();
        SkImageInfo.referenced();
        SkCanvas.referenced();
        SkImage.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkSurface(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
