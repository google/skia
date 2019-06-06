// This is an automatically generated file. Please do not edit.

package org.skia;

import org.skia.SkRect;
import org.skia.SkCanvas;
import org.skia.SkPicture;

public class SkPictureRecorder {
    private native void nativeInit();

    public SkPictureRecorder() {
        nativeInit();
    }

    public native SkCanvas beginRecording(SkRect rect);

    public native SkPicture endRecording();

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
        SkCanvas.referenced();
        SkPicture.referenced();
        staticInit();
    }

    static void referenced() {}

    private SkPictureRecorder(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
