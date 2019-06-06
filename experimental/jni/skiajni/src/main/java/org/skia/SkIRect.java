// This is an automatically generated file. Please do not edit.

package org.skia;


public class SkIRect {
    private native void nativeInit();

    public SkIRect() {
        nativeInit();
    }

    public SkIRect(int left, int top, int right, int bottom) {
        this();
        this.left = left;
        this.top = top;
        this.right = right;
        this.bottom = bottom;
    }

    public int left;

    public int top;

    public int right;

    public int bottom;

    @Override
    protected void finalize() throws Throwable {
        try {
          nativeFinalize();
        } finally {
          super.finalize();
        }
    }

    static {
        staticInit();
    }

    static void referenced() {}

    private SkIRect(boolean invokeFromNative) {}

    private static native void staticInit();

    private native void nativeFinalize();

    private long _this;
}
