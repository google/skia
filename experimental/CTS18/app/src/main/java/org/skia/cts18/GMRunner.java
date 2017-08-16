package org.skia.cts18;

/**
 * Created by stephana on 8/14/17.
 */

public class GMRunner {
    static {
        System.loadLibrary("gmrunner");
    }

    public static class ImageStub {
        public byte[] pix;
        public int width;
        public int height;
        public int byteOrder;

        public ctsdriver.Image asGoImage() {
            ctsdriver.Image ret = new ctsdriver.Image();
            ret.setPix(pix);
            ret.setByteOrder(byteOrder);
            ret.setHeight(height);
            ret.setWidth(width);
            return ret;
        }
    }

    public static native String runGM(String testName, ImageStub image);

//    public static String runGM(String testName, ctsdriver.Image image) {
//        return "";
//    }
}
