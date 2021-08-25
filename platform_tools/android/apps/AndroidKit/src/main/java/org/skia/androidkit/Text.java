/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class Text {
    private long mNativeInstance;
    private String mRawText;

    public Text(String text) {
        mRawText = text;
    }

    public void setText(String text) {
        mRawText = text;
    }

    public void renderText(Canvas canvas, Paint foreground, float x, float y) {
        nRenderText(mRawText, canvas.getNativeInstance(), foreground.getNativeInstance(), x, y);
    }

    private static native void nRenderText(String text, long nativeCanvas, long foreground, float x, float y);
}
