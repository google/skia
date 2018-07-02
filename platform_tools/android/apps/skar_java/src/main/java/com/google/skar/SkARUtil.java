package com.google.skar;

import android.graphics.ColorFilter;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;

import java.util.Arrays;

public class SkARUtil {

    private static final float MIDDLE_GRAY_GAMMA = 0.466f;

    /**
     * Returns a ColorFilter that can be used on a Paint to apply color correction effects
     * as documented by ARCore in
     * <a href="https://developers.google.com/ar/reference/java/com/google/ar/core/LightEstimate">LightEstimate</a>
     *
     * @param colorCorr output array of
     *                  <a href="https://developers.google.com/ar/reference/java/com/google/ar/core/LightEstimate.html#getColorCorrection(float[],%20int)">getColorCorrection()</a>
     * @return ColorFilter with effects applied
     */
    public static ColorFilter createLightCorrectionColorFilter(float[] colorCorr) {
        float[] colorCorrCopy = Arrays.copyOf(colorCorr, 4);
        for (int i = 0; i < 3; i++) {
            colorCorrCopy[i] *= colorCorrCopy[3] / MIDDLE_GRAY_GAMMA;
        }
        ColorMatrix m = new ColorMatrix();
        m.setScale(colorCorrCopy[0], colorCorrCopy[1], colorCorrCopy[2], 1);
        return new ColorMatrixColorFilter(m);
    }
}
