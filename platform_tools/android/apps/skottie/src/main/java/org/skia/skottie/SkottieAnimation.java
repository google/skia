package org.skia.skottie;

import android.support.annotation.FloatRange;

public interface SkottieAnimation {
    /**
     * Starts the animation.
     */
    void start();

    /**
     * Stops the animation.
     */
    void stop();

    /**
     * Indicates whether the animation is running.
     *
     * @return True if the animation is running, false otherwise.
     */
    boolean isRunning();

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
     * Gets current sanimation progress.
     *
     * @return animation progress.
     */
    @FloatRange(from = 0, to = 1) float getProgress();
}

