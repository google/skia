/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_CONFIG_H
#define SK_COMMON_FLAGS_CONFIG_H

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

DECLARE_string(config);

class SkCommandLineConfigGpu;
class SkCommandLineConfigSvg;

// SkCommandLineConfig represents a Skia rendering configuration string.
// The string has following form:
// tag:
//   [via-]*backend
// where 'backend' consists of chars excluding hyphen
// and each 'via' consists of chars excluding hyphen.
class SkCommandLineConfig {
public:
    SkCommandLineConfig(const SkString&           tag,
                        const SkString&           backend,
                        const SkTArray<SkString>& viaParts);
    virtual ~SkCommandLineConfig();
    virtual const SkCommandLineConfigGpu* asConfigGpu() const { return nullptr; }
    virtual const SkCommandLineConfigSvg* asConfigSvg() const { return nullptr; }
    const SkString&                       getTag() const { return fTag; }
    const SkString&                       getBackend() const { return fBackend; }
    const SkTArray<SkString>&             getViaParts() const { return fViaParts; }

private:
    SkString           fTag;
    SkString           fBackend;
    SkTArray<SkString> fViaParts;
};

// SkCommandLineConfigGpu is a SkCommandLineConfig that extracts information out of the backend
// part of the tag. It is constructed tags that have:
// * backends of form "gpu[option=value,option2=value,...]"
// * backends that represent a shorthand of above (such as "glmsaa16" representing
// "gpu(api=gl,samples=16)")
class SkCommandLineConfigGpu : public SkCommandLineConfig {
public:
    enum class SurfType { kDefault, kBackendTexture, kBackendRenderTarget };
    typedef sk_gpu_test::GrContextFactory::ContextType      ContextType;
    typedef sk_gpu_test::GrContextFactory::ContextOverrides ContextOverrides;

    SkCommandLineConfigGpu(const SkString&           tag,
                           const SkTArray<SkString>& viaParts,
                           ContextType               contextType,
                           bool                      useNVPR,
                           bool                      useDIText,
                           int                       samples,
                           SkColorType               colorType,
                           SkAlphaType               alphaType,
                           sk_sp<SkColorSpace>       colorSpace,
                           bool                      useStencilBuffers,
                           bool                      testThreading,
                           int                       testPersistentCache,
                           SurfType);

    const SkCommandLineConfigGpu* asConfigGpu() const override { return this; }
    ContextType                   getContextType() const { return fContextType; }
    ContextOverrides              getContextOverrides() const { return fContextOverrides; }
    bool                          getUseNVPR() const {
        SkASSERT(!(fContextOverrides & ContextOverrides::kRequireNVPRSupport) ||
                 !(fContextOverrides & ContextOverrides::kDisableNVPR));
        return fContextOverrides & ContextOverrides::kRequireNVPRSupport;
    }
    bool          getUseDIText() const { return fUseDIText; }
    int           getSamples() const { return fSamples; }
    SkColorType   getColorType() const { return fColorType; }
    SkAlphaType   getAlphaType() const { return fAlphaType; }
    SkColorSpace* getColorSpace() const { return fColorSpace.get(); }
    bool          getTestThreading() const { return fTestThreading; }
    int           getTestPersistentCache() const { return fTestPersistentCache; }
    SurfType      getSurfType() const { return fSurfType; }

private:
    ContextType         fContextType;
    ContextOverrides    fContextOverrides;
    bool                fUseDIText;
    int                 fSamples;
    SkColorType         fColorType;
    SkAlphaType         fAlphaType;
    sk_sp<SkColorSpace> fColorSpace;
    bool                fTestThreading;
    int                 fTestPersistentCache;
    SurfType            fSurfType;
};

// SkCommandLineConfigSvg is a SkCommandLineConfig that extracts information out of the backend
// part of the tag. It is constructed tags that have:
// * backends of form "svg[option=value,option2=value,...]"
class SkCommandLineConfigSvg : public SkCommandLineConfig {
public:
    SkCommandLineConfigSvg(const SkString& tag, const SkTArray<SkString>& viaParts, int pageIndex);
    const SkCommandLineConfigSvg* asConfigSvg() const override { return this; }

    int getPageIndex() const { return fPageIndex; }

private:
    int fPageIndex;
};

typedef SkTArray<std::unique_ptr<SkCommandLineConfig>, true> SkCommandLineConfigArray;
void ParseConfigs(const CommandLineFlags::StringArray& configList,
                  SkCommandLineConfigArray*            outResult);

#endif
