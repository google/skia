/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTypes_DEFINED
#define GrDawnTypes_DEFINED

#ifdef Always
#undef Always
static constexpr int Always = 2;
#endif
#ifdef Success
#undef Success
static constexpr int Success = 0;
#endif
#ifdef None
#undef None
static constexpr int None = 0L;
#endif
#include "dawn/dawncpp.h"

struct GrDawnTextureInfo {
    wgpu::Texture       fTexture;
    wgpu::TextureFormat fFormat;
    uint32_t            fLevelCount;
    GrDawnTextureInfo() : fTexture(nullptr), fFormat(), fLevelCount(0) {
    }
    GrDawnTextureInfo(const GrDawnTextureInfo& other)
        : fTexture(other.fTexture)
        , fFormat(other.fFormat)
        , fLevelCount(other.fLevelCount) {
    }
    GrDawnTextureInfo& operator=(const GrDawnTextureInfo& other) {
        fTexture = other.fTexture;
        fFormat = other.fFormat;
        fLevelCount = other.fLevelCount;
        return *this;
    }
    bool operator==(const GrDawnTextureInfo& other) const {
        return fTexture.Get() == other.fTexture.Get() &&
               fFormat == other.fFormat &&
               fLevelCount == other.fLevelCount;
    }
};

struct GrDawnRenderTargetInfo {
    wgpu::TextureView   fTextureView;
    wgpu::TextureFormat fFormat;
    uint32_t            fLevelCount;
    GrDawnRenderTargetInfo() : fTextureView(nullptr), fFormat(), fLevelCount(0) {
    }
    GrDawnRenderTargetInfo(const GrDawnRenderTargetInfo& other)
        : fTextureView(other.fTextureView)
        , fFormat(other.fFormat)
        , fLevelCount(other.fLevelCount) {
    }
    explicit GrDawnRenderTargetInfo(const GrDawnTextureInfo& texInfo)
        : fTextureView(texInfo.fTexture.CreateView())
        , fFormat(texInfo.fFormat)
        , fLevelCount(texInfo.fLevelCount) {
    }
    GrDawnRenderTargetInfo& operator=(const GrDawnRenderTargetInfo& other) {
        fTextureView = other.fTextureView;
        fFormat = other.fFormat;
        fLevelCount = other.fLevelCount;
        return *this;
    }
    bool operator==(const GrDawnRenderTargetInfo& other) const {
        return fTextureView.Get() == other.fTextureView.Get() &&
               fFormat == other.fFormat &&
               fLevelCount == other.fLevelCount;
    }
};

#endif
