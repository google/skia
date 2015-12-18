/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_CONFIG_H
#define SK_COMMON_FLAGS_CONFIG_H

#include "SkCommandLineFlags.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif

DECLARE_string(config);

#if SK_SUPPORT_GPU
class SkCommandLineConfigGpu;
#endif

// SkCommandLineConfig represents a Skia rendering configuration string.
// The string has following form:
//   [via-]*tag
// where 'tag' consists of chars excluding hyphen or "angle-gl"
// and each 'via' consists of chars excluding hyphen.
class SkCommandLineConfig {
  public:
    SkCommandLineConfig(const SkString& tag, const SkTArray<SkString>& viaParts);
    virtual ~SkCommandLineConfig();
#if SK_SUPPORT_GPU
    virtual const SkCommandLineConfigGpu* asConfigGpu() const { return nullptr; }
#endif
    const SkString& getTag() const { return fTag; }
    const SkTArray<SkString>& getViaParts() const { return fViaParts; }
  private:
    SkString fTag;
    SkTArray<SkString> fViaParts;
};

#if SK_SUPPORT_GPU
// SkCommandLineConfigGpu is a SkCommandLineConfig that extracts information out of the tag.  It is
// constructed configs that have:
// * tags of form "gpu(option=value,option2=value,...)"
// * tags that represent a shorthand of above (such as "msaa16" representing "gpu(samples=16)")
class SkCommandLineConfigGpu : public SkCommandLineConfig {
  public:
    typedef GrContextFactory::GLContextType ContextType;
    SkCommandLineConfigGpu(const SkString& tag, const SkTArray<SkString>& viaParts,
                           ContextType contextType, bool useNVPR, bool useDIText, int samples);
    const SkCommandLineConfigGpu* asConfigGpu() const override { return this; }
    ContextType getContextType() const { return fContextType; }
    bool getUseNVPR() const { return fUseNVPR; }
    bool getUseDIText() const { return fUseDIText; }
    int getSamples() const { return fSamples; }

  private:
    ContextType fContextType;
    bool fUseNVPR;
    bool fUseDIText;
    int fSamples;
};
#endif

typedef SkTArray<SkAutoTDelete<SkCommandLineConfig>, true> SkCommandLineConfigArray;
void ParseConfigs(const SkCommandLineFlags::StringArray& configList,
                  SkCommandLineConfigArray* outResult);

#endif
