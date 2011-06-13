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
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class SampleApp extends Activity
{
    private TextView mTitle;
    private SampleView mView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.layout);
        mTitle = (TextView) findViewById(R.id.title_view);
        LinearLayout holder = (LinearLayout) findViewById(R.id.holder);
        mView = new SampleView(this);

        holder.addView(mView, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        
        mTitle.setVisibility(View.GONE);
        getActionBar().setDisplayShowHomeEnabled(false);
        
    }

    @Override
    public void onDestroy() {
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                term();
            }
        });
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.sample, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.overview:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyDown(KeyEvent.KEYCODE_BACK, 0);
                    }
                });
                return true;
            case R.id.next:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyDown(KeyEvent.KEYCODE_DPAD_RIGHT, 0);
                    }
                });
                return true;
            default:
                return false;
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        final int keycode = event.getKeyCode();
        if (keycode == KeyEvent.KEYCODE_BACK) {
            if (event.getAction() == KeyEvent.ACTION_UP) {
                finish();
            }
            return true;
        }
        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                final int uni = event.getUnicodeChar(event.getMetaState());
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyDown(keycode, uni);
                    }
                });
                
                return true;
            case KeyEvent.ACTION_UP:
                mView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        handleKeyUp(keycode);
                    }
                });
                return true;
            default:
                return false;
        }
    }

    private static final int SET_TITLE = 1;
    
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_TITLE:
                    mTitle.setText((String) msg.obj);
                    SampleApp.this.getActionBar().setSubtitle((String) msg.obj);
                    break;
                default:
                    break;
            }
        }
    };
    
    @Override
    public void setTitle(CharSequence title) {
        mHandler.obtainMessage(SET_TITLE, title).sendToTarget();
    }

    native void draw();
    native void init();
    native void term();
    // Currently depends on init having already been called.
    native void createOSWindow(GLSurfaceView view);
    native void updateSize(int w, int h);
    native void handleClick(int x, int y, int state);
    native boolean handleKeyDown(int key, int uni);
    native boolean handleKeyUp(int key);
    native void zoom(float factor);

    static {
        System.loadLibrary("skia-sample");
    }
}
