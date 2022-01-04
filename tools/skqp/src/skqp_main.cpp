/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <sys/stat.h>

#include "tools/skqp/src/skqp.h"

#include "include/core/SkData.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"

////////////////////////////////////////////////////////////////////////////////

namespace {
class StdAssetManager : public SkQPAssetManager {
public:
    StdAssetManager(const char* p) : fPrefix(p) {
        SkASSERT(!fPrefix.empty());
        //TODO(halcanary): does this need to be changed if I run SkQP in Windows?
        fPrefix += "/";
    }
    sk_sp<SkData> open(const char* path) override {
        return SkData::MakeFromFileName((fPrefix + path).c_str());
    }
private:
    std::string fPrefix;
};

struct Args {
  char *assetDir;
  char *renderTests;
  char *outputDir;
};
}  // namespace

static constexpr char kSkipUsage[] =
    " TEST_MATCH_RULES:"
    "    [~][^]substring[$] [...] of name to run.\n"
    "    Multiple matches may be separated by spaces.\n"
    "    ~ causes a matching name to always be skipped\n"
    "    ^ requires the start of the name to match\n"
    "    $ requires the end of the name to match\n"
    "    ^ and $ requires an exact match\n"
    "    If a name does not match any list entry,\n"
    "    it is skipped unless some list entry starts with ~\n";

static bool should_skip(const char* const* rules, size_t count, const char* name) {
    size_t testLen = strlen(name);
    bool anyExclude = count == 0;
    for (size_t i = 0; i < count; ++i) {
        const char* matchName = rules[i];
        size_t matchLen = strlen(matchName);
        bool matchExclude, matchStart, matchEnd;
        if ((matchExclude = matchName[0] == '~')) {
            anyExclude = true;
            matchName++;
            matchLen--;
        }
        if ((matchStart = matchName[0] == '^')) {
            matchName++;
            matchLen--;
        }
        if ((matchEnd = matchName[matchLen - 1] == '$')) {
            matchLen--;
        }
        if (matchStart ? (!matchEnd || matchLen == testLen)
                && strncmp(name, matchName, matchLen) == 0
                : matchEnd ? matchLen <= testLen
                && strncmp(name + testLen - matchLen, matchName, matchLen) == 0
                : strstr(name, matchName) != nullptr) {
            return matchExclude;
        }
    }
    return !anyExclude;
}

static void parse_args(int argc, char *argv[], Args *args) {
  if (argc < 4) {
      std::cerr << "Usage:\n  " << argv[0]
                << " ASSET_DIR RENDER_TESTS OUTPUT_DIR [TEST_MATCH_RULES]\n"
                << kSkipUsage << '\n';
      exit(1);
  }
  args->assetDir = argv[1];
  args->renderTests = argv[2];
  args->outputDir = argv[3];
}

int main(int argc, char *argv[]) {
    Args args;
    parse_args(argc, argv, &args);

    SetResourcePath(std::string(args.assetDir + std::string("/resources")).c_str());
    if (!sk_mkdir(args.outputDir)) {
        std::cerr << "sk_mkdir(" << args.outputDir << ") failed.\n";
        return 2;
    }
    StdAssetManager mgr(args.assetDir);
    SkQP skqp;
    skqp.init(&mgr, args.renderTests, args.outputDir);
    int ret = 0;

    const char* const* matchRules = &argv[4];
    size_t matchRulesCount = (size_t)(argc - 4);

    // Rendering Tests
    std::ostream& out = std::cout;
    for (auto backend : skqp.getSupportedBackends()) {
        auto testPrefix = std::string(SkQP::GetBackendName(backend)) + "_";
        for (auto gmFactory : skqp.getGMs()) {
            auto testName = testPrefix + SkQP::GetGMName(gmFactory);
            if (should_skip(matchRules, matchRulesCount, testName.c_str())) {
                continue;
            }
            out << "Starting: " << testName << "  ";
            SkQP::RenderOutcome outcome;
            std::string except;

            std::tie(outcome, except) = skqp.evaluateGM(backend, gmFactory);
            if (!except.empty()) {
                out << "[ERROR: " << except << "]" << std::endl;
                ret = 1;
            } else if (outcome.fMaxError != 0) {
                out << "[FAILED: " << outcome.fMaxError << "]" << std::endl;
                ret = 1;
            } else {
                out << "[PASSED]" << std::endl;
            }
            out.flush();
        }
    }

    // Unit Tests
    for (auto test : skqp.getUnitTests()) {
        auto testName = std::string("unitTest_") +  SkQP::GetUnitTestName(test);
        if (should_skip(matchRules, matchRulesCount, testName.c_str())) {
            continue;
        }
        out << "Starting: " << testName << " ";
        std::vector<std::string> errors = skqp.executeTest(test);
        if (!errors.empty()) {
            out << "[FAILED: " << errors.size() << " error(s)]" << std::endl;
            for (const std::string& error : errors) {
                out << "  " <<  error << std::endl;
            }
            ret = 1;
        } else {
            out << "[PASSED]" << std::endl;
        }
        out.flush();
    }
    skqp.makeReport();

    return ret;
}
