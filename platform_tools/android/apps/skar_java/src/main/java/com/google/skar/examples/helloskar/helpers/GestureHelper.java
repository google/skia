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

package com.google.skar.examples.helloskar.helpers;

import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * Helper to detect gestures using Android GestureDetector, and pass the taps between UI thread and
 * render thread.
 */

public final class GestureHelper implements OnTouchListener {
    private final GestureDetector gestureDetector;
    private final BlockingQueue<MotionEvent> queuedSingleTaps = new ArrayBlockingQueue<>(16);
    private final BlockingQueue<ScrollEvent> queuedFingerHold = new ArrayBlockingQueue<>(16);
    private boolean isScrolling = false;
    private boolean previousScroll = true;

    // Struct holding a MotionEvent obtained from onScroll() callbacks, and a boolean evaluating to
    // true if the MotionEven was the start of the scrolling motion
    public static class ScrollEvent {
        public MotionEvent event;
        public boolean isStartOfScroll;

        public ScrollEvent(MotionEvent e, boolean isStartOfScroll) {
            this.event = e;
            this.isStartOfScroll = isStartOfScroll;
        }
    }

    /**
     * Creates the gesture helper.
     *
     * @param context the application's context.
     */
    public GestureHelper(Context context) {
        gestureDetector =
                new GestureDetector(
                        context,
                        new GestureDetector.SimpleOnGestureListener() {
                            @Override
                            public boolean onSingleTapUp(MotionEvent e) {
                                // Queue tap if there is space. Tap is lost if queue is full.
                                queuedSingleTaps.offer(e);
                                return true;
                            }

                            @Override
                            public boolean onScroll (MotionEvent e1, MotionEvent e2,
                                                     float distanceX, float distanceY) {
                                // Queue motion events when scrolling
                                if (e2.getPointerCount() == 1 && e1.getPointerCount() == 1) {
                                    previousScroll = isScrolling;
                                    isScrolling = true;

                                    queuedFingerHold.offer(new ScrollEvent(e2,
                                                                           isStartedScrolling()));

                                    return true;
                                }
                                return false;
                            }


                            @Override
                            public boolean onDown(MotionEvent e) {
                                return true;
                            }
                        });
    }

    /**
     * Polls for a tap.
     *
     * @return if a tap was queued, a MotionEvent for the tap. Otherwise null if no taps are queued.
     */
    public MotionEvent poll() {
        return queuedSingleTaps.poll();
    }

    /**
     * Polls for a scrolling motion.
     *
     * @return if a scrolling event was queued, a ScrollEvent for the gesture. Otherwise null
     */
    public ScrollEvent holdPoll() { return queuedFingerHold.poll(); }

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        boolean val = gestureDetector.onTouchEvent(motionEvent);

        // If finger is up + is scrolling: don't scroll anymore, and empty touch hold queue
        if (motionEvent.getAction() == MotionEvent.ACTION_UP && isScrolling) {
            previousScroll = true;
            isScrolling = false;
            queuedFingerHold.clear();
        }
        return val;
    }
    private boolean isStartedScrolling() { return isScrolling && !previousScroll; }
}
