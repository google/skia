/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

public class VisualBenchActivity extends android.app.NativeActivity {
    static {
        System.loadLibrary("skia_android");
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
    }
  
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (!hasFocus) {
            return;
        }
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | // hide nav bar
            View.SYSTEM_UI_FLAG_FULLSCREEN |// hide status bar
            View.SYSTEM_UI_FLAG_IMMERSIVE);
        
        // Disable backlight to keep the system as cool as possible
        // TODO make this configurable
        Settings.System.putInt(getContentResolver(),
            Settings.System.SCREEN_BRIGHTNESS_MODE,
            Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);
        
        WindowManager.LayoutParams lp = getWindow().getAttributes();
        lp.screenBrightness = 0; // 0f - no backlight
        getWindow().setAttributes(lp);
    }
}
