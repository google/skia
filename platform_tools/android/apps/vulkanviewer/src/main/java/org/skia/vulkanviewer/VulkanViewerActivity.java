/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.vulkanviewer;

import android.app.ActionBar;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.view.WindowManager;

public class VulkanViewerActivity extends android.app.NativeActivity {
    static {
        System.loadLibrary("skia_android");
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        ActionBar ab = this.getActionBar();
        ab.hide();
    }
}
