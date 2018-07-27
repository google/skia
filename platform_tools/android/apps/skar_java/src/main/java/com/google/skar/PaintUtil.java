/*
 * Copyright 2018 Google LLC All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.skar;

import android.graphics.ColorFilter;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;

import java.util.Arrays;

public class PaintUtil {

    private static final float MIDDLE_GRAY_GAMMA = 0.466f;

    /**
     * Returns a ColorFilter that can be used on a Paint to apply color correction effects
     * as documented by ARCore.
     *
     * @param colorCorr output array of getColorCorrection() from ARCore
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
