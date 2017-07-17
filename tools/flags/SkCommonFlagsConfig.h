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
// tag:
//   [via-]*backend
// where 'backend' consists of chars excluding hyphen
// and each 'via' consists of chars excluding hyphen.
class SkCommandLineConfig {
  public:
    SkCommandLineConfig(const SkString& tag, const SkString& backend,
                        const SkTArray<SkString>& viaParts);
    virtual ~SkCommandLineConfig();
#if SK_SUPPORT_GPU
    virtual const SkCommandLineConfigGpu* asConfigGpu() const { return nullptr; }
#endif
    const SkString& getTag() const { return fTag; }
    const SkString& getBackend() const { return fBackend; }
    const SkTArray<SkString>& getViaParts() const { return fViaParts; }
  private:
    SkString fTag;
    SkString fBackend;
    SkTArray<SkString> fViaParts;
};

#if SK_SUPPORT_GPU
// SkCommandLineConfigGpu is a SkCommandLineConfig that extracts information out of the backend
// part of the tag. It is constructed tags that have:
// * backends of form "gpu[option=value,option2=value,...]"
// * backends that represent a shorthand of above (such as "glmsaa16" representing
// "gpu(api=gl,samples=16)")
class SkCommandLineConfigGpu : public SkCommandLineConfig {
  public:
    typedef sk_gpu_test::GrContextFactory::ContextType ContextType;
    typedef sk_gpu_test::GrContextFactory::ContextOverrides ContextOverrides;
    SkCommandLineConfigGpu(const SkString& tag, const SkTArray<SkString>& viaParts,
                           ContextType contextType, bool useNVPR, bool useInstanced, bool useDIText,
                           int samples, SkColorType colorType, SkAlphaType alphaType,
                           sk_sp<SkColorSpace> colorSpace, bool useStencilBuffers);
    const SkCommandLineConfigGpu* asConfigGpu() const override { return this; }
    ContextType getContextType() const { return fContextType; }
    ContextOverrides getContextOverrides() const { return fContextOverrides; }
    bool getUseNVPR() const {
        SkASSERT(!(fContextOverrides & ContextOverrides::kRequireNVPRSupport) ||
                 !(fContextOverrides & ContextOverrides::kDisableNVPR));
        return fContextOverrides & ContextOverrides::kRequireNVPRSupport;
    }
    bool getUseInstanced() const { return fContextOverrides & ContextOverrides::kUseInstanced; }
    bool getUseDIText() const { return fUseDIText; }
    int getSamples() const { return fSamples; }
    SkColorType getColorType() const { return fColorType; }
    SkAlphaType getAlphaType() const { return fAlphaType; }
    SkColorSpace* getColorSpace() const { return fColorSpace.get(); }

  private:
    ContextType fContextType;
    ContextOverrides fContextOverrides;
    bool fUseDIText;
    int fSamples;
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fColorSpace;
};
#endif

typedef SkTArray<std::unique_ptr<SkCommandLineConfig>, true> SkCommandLineConfigArray;
void ParseConfigs(const SkCommandLineFlags::StringArray& configList,
                  SkCommandLineConfigArray* outResult);

#endif
