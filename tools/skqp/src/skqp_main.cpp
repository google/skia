/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <sys/stat.h>

#include "skqp.h"

#include "Resources.h"
#include "SkData.h"

////////////////////////////////////////////////////////////////////////////////

namespace {
struct StdAssetManager : public SkQPAssetManager {
    std::string fPrefix;
    StdAssetManager(const char* p) : fPrefix(p) {
        SkASSERT(!fPrefix.empty());
        fPrefix += "/";
    }
    sk_sp<SkData> open(const char* path) override {
        return SkData::MakeFromFileName((fPrefix + path).c_str());
    }
};
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage:\n  " << argv[0]
                  << " ASSET_DIRECTORY_PATH SKQP_REPORT_PATH\n\n";
        return 1;
    }
    SetResourcePath((std::string(argv[1]) + "/resources").c_str());
    (void)mkdir(argv[2], 0777);
    std::unique_ptr<SkQPAssetManager> mgr(new StdAssetManager(argv[1]));
    SkQP skqp;
    skqp.init(std::move(mgr), argv[2], false);
    int ret = 0;

    // Rendering Tests
    std::ostream& out = std::cout;
    for (auto backend : skqp.getSupportedBackends()) {
        auto testPrefix = std::string("skqp_") + SkQP::GetBackendName(backend) + "_";
        for (auto gmFactory : skqp.getGMs()) {
            auto testName = testPrefix + SkQP::GetGMName(gmFactory);
            out << "Starting: " << testName << std::endl;
            int maxError;
            int errorCount;
            std::string except;
            std::tie(maxError, errorCount, except) = skqp.evaluateGM(backend, gmFactory);
            if (!except.empty()) {
                out << "ERROR:    " << testName << " (" << except << ")\n";
                ret = 1;
            } else if (maxError != 0) {
                out << "FAILED:   " << testName << " (" << maxError << ")\n";
                ret = 1;
            } else {
                out << "Passed:   " << testName << "\n";
            }
            out.flush();
        }
    }

    // Unit Tests
    for (auto test : skqp.getUnitTests()) {
        auto testName = std::string("skqp_unitTest_") +  SkQP::GetUnitTestName(test);
        out << "Starting test: " << testName << std::endl;
        std::vector<std::string> errors = skqp.executeTest(test);
        if (!errors.empty()) {
            out << "TEST FAILED (" << errors.size() << "): " << testName << "\n";
            for (const std::string& error : errors) {
                out << error << "\n";
            }
            ret = 1;
        } else {
            out << "Test passed:   " << testName << "\n";
        }
        out.flush();
    }
    skqp.makeReport();

    return ret;
}
