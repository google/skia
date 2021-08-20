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

    public void renderText(Canvas canvas, float x, float y) {
        nRenderText(mRawText, canvas.getNativeInstance(), x, y);
    }

    private static native void nRenderText(String text, long nativeCanvas, float x, float y);
}
