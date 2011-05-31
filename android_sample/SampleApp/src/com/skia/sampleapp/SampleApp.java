/*
 * Copyright (C) 2011 Skia
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

package com.skia.sampleapp;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SampleApp extends Activity
{
    private TextView mTitle;

    public class SampleView extends View {
        public SampleView(Context context) {
            super(context);
            createOSWindow(this);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            drawToCanvas(canvas);
        }

        @Override
        protected void onSizeChanged(int w, int h, int oldw, int oldh) {
            updateSize(w, h);
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            final int x = (int) event.getX();
            final int y = (int) event.getY();
            final int action = event.getAction();
            handleClick(x, y, action);
            return true;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        init();
        setContentView(R.layout.layout);
        mTitle = (TextView) findViewById(R.id.title_view);
        LinearLayout holder = (LinearLayout) findViewById(R.id.holder);
        View view = new SampleView(this);
        holder.addView(view, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
    }

    @Override
    public void onDestroy()
    {
        term();
        super.onDestroy();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                int uni = event.getUnicodeChar(event.getMetaState());
                return handleKeyDown(event.getKeyCode(), uni);
            case KeyEvent.ACTION_UP:
                return handleKeyUp(event.getKeyCode());
            default:
                return false;
        }
    }

    @Override
    public void setTitle(CharSequence title) {
        mTitle.setText(title);
    }

    private native void drawToCanvas(Canvas canvas);
    private native void init();
    private native void term();
    // Currently depends on init having already been called.
    private native void createOSWindow(SampleView view);
    private native void updateSize(int w, int h);
    private native void handleClick(int x, int y, int state);
    private native boolean handleKeyDown(int key, int uni);
    private native boolean handleKeyUp(int key);

    static {
        System.loadLibrary("skia-sample");
    }
}
