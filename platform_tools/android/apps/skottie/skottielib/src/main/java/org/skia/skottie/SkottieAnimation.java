package org.skia.skottie;

import android.graphics.drawable.Animatable;
import android.support.annotation.FloatRange;

public interface SkottieAnimation extends Animatable {
    /**
     * Gets animation duration.
     *
     * @return Animation duration in milliseconds.
     */
    long getDuration();

    /**
     * Sets current animation progress.
     *
     *  @param progress animation progress
     *
     */
    void setProgress(@FloatRange(from = 0, to = 1) float progress);


    /**
     * Gets current animation progress.
     *
     * @return animation progress.
     */
    @FloatRange(from = 0, to = 1) float getProgress();
}

