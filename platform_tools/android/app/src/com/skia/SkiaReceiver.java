// Copyright 2012 Google Inc. All Rights Reserved.

package com.skia;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import com.skia.SkiaIntentService;

/**
 * @author borenet@google.com (Eric Boren)
 *
 */
public class SkiaReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent skIntent = new Intent(context, SkiaIntentService.class);
        
        // Forward any command-line arguments to the background service
        skIntent.putExtras(intent.getExtras());
        
        // Launch executable
        context.startService(skIntent);
  }
}