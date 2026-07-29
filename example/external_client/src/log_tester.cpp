/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/private/SkLog.h"
#include "include/utils/SkLogHandler.h"

#include <iostream>
#include <mutex>
#include <string>
#include <vector>

class LogAccumulator : public SkLogHandler {
public:
    struct LogEntry {
        SkLogPriority priority;
        std::string message;
    };

    void onLog(SkLogPriority priority, const char format[], va_list args) override {
        char buffer[1024];
        int written = vsnprintf(buffer, sizeof(buffer), format, args);
        if (written < 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(fMutex);
        fLogs.push_back({priority, buffer});
        fCounter++;
    }

    int getCount() const {
        return fCounter;
    }

    std::string getLatest() const {
        if (fLogs.empty()) {
            return "";
        }
        return fLogs.back().message;
    }

    std::string dumpLogs(SkLogPriority minimumLevel = SkLogPriority::kDebug) const {
        std::string output;
        for (const auto& entry : fLogs) {
            // Lower integer value means higher priority (kError=0, kDebug=3)
            if (entry.priority <= minimumLevel) {
                output += entry.message;
                if (output.empty() || output.back() != '\n') {
                    output += '\n';
                }
            }
        }
        return output;
    }

private:
    mutable std::mutex fMutex;
    std::vector<LogEntry> fLogs;
    int fCounter = 0;
};

int main() {
    // Setup the log accumulator
    auto accumulator = sk_make_sp<LogAccumulator>();
    SkLogHandler::SetInstance(accumulator);

    std::cout << "--- Starting Skia Operations ---\n";

    // Trigger some logs manually using the macros, but this should only be called from Skia.
    SKIA_LOG_I("Log tester started.");
    SKIA_LOG_W("This is a warning log.");
    SKIA_LOG_D("This is a debug log.");

    // Perform some Skia operations
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas.drawRect(SkRect::MakeWH(50, 50), paint);

    std::cout << "\n--- Latest Log ---\n";
    std::cout << accumulator->getLatest() << std::endl;

    std::cout << "\n--- All Logs (Level Warning and above) ---\n";
    std::cout << accumulator->dumpLogs(SkLogPriority::kWarning);

    std::cout << "\n--- All Logs (Level Debug and above) ---\n";
    std::cout << accumulator->dumpLogs(SkLogPriority::kDebug);

    return 0;
}
