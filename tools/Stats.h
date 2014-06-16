#ifndef Stats_DEFINED
#define Stats_DEFINED

struct Stats {
    Stats(const double samples[], int n) {
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
        var = err / (n-1);
    }

    double min;
    double max;
    double mean;  // Estimate of population mean.
    double var;   // Estimate of population variance.
};

#endif//Stats_DEFINED
