// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkPixelGeometry;

public class SkSurfaceProps {
    private native void nativeInit();

    public SkSurfaceProps() {
        nativeInit();
    }

    public SkSurfaceProps(SkPixelGeometry pixelGeometry) {
        this();
        this.pixelGeometry = pixelGeometry;
    }

    public SkPixelGeometry pixelGeometry;

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        SkPixelGeometry.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkSurfaceProps(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
