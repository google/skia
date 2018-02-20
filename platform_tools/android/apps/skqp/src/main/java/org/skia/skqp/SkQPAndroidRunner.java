/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.os.Bundle;
import android.support.test.runner.AndroidJUnitRunner;
import java.util.HashSet;

public class SkQPAndroidRunner extends AndroidJUnitRunner {
    @Override
    public void onCreate(Bundle args) {
        String filter =  args.getString("skqp_filter");
        if (filter != null) {
            gFilters = new HashSet<String>();
            for (String f : filter.split(",")) {
                gFilters.add(f);
            }
        }
        super.onCreate(args);
    }
    public static boolean filter(String s) { return gFilters == null || gFilters.contains(s); }
    private static HashSet<String> gFilters;
}
