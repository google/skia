/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Stats_DEFINED
#define Stats_DEFINED

#include <algorithm>
#include <vector>

#include "include/core/SkString.h"
#include "include/private/base/SkFloatingPoint.h"

#ifdef SK_BUILD_FOR_WIN
    static const char* kBars[] = { ".", "o", "O" };
#else
    static const char* kBars[] = { "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█" };
#endif

struct Stats {
    Stats(const SkTArray<double>& samples, bool want_plot) {
        int n = samples.size();
        if (!n) {
            min = max = mean = var = median = 0;
            return;
        }

        min = samples[0];
        max = samples[0];
        for (int i = 0; i < n; i++) {
            if (samples[i] < min) { min = samples[i]; }
            if (samples[i] > max) { max = samples[i]; }
        }

        double sum = 0.0;
        for (int i = 0 ; i < n; i++) {
            sum += samples[i];
        }
        mean = sum / n;

        double err = 0.0;
        for (int i = 0 ; i < n; i++) {
            err += (samples[i] - mean) * (samples[i] - mean);
        }
        var = sk_ieee_double_divide(err, n-1);

        std::vector<double> sorted(samples.begin(), samples.end());
        std::sort(sorted.begin(), sorted.end());
        median = sorted[n/2];

        // Normalize samples to [min, max] in as many quanta as we have distinct bars to print.
        for (int i = 0; want_plot && i < n; i++) {
            if (min == max) {
                // All samples are the same value.  Don't divide by zero.
                plot.append(kBars[0]);
                continue;
            }

            double s = samples[i];
            s -= min;
            s /= (max - min);
            s *= (std::size(kBars) - 1);
            const size_t bar = (size_t)(s + 0.5);
            SkASSERT_RELEASE(bar < std::size(kBars));
            plot.append(kBars[bar]);
        }
    }

    double min;
    double max;
    double mean;    // Estimate of population mean.
    double var;     // Estimate of population variance.
    double median;
    SkString plot;  // A single-line bar chart (_not_ histogram) of the samples.
};

#endif//Stats_DEFINED
