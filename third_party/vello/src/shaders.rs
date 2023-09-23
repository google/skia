// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use crate::ffi;
use vello_shaders::{BindType, WorkgroupBufferInfo};

pub(crate) struct Shader(&'static vello_shaders::ComputeShader<'static>);

impl Shader {
    pub fn name(&self) -> &str {
        &self.0.name
    }

    pub fn workgroup_size(&self) -> ffi::WorkgroupSize {
        self.0.workgroup_size.into()
    }

    pub fn bindings(&self) -> Vec<ffi::BindType> {
        self.0.bindings.iter().map(|t| t.into()).collect()
    }

    pub fn workgroup_buffers(&self) -> Vec<ffi::WorkgroupBufferInfo> {
        self.0.workgroup_buffers.iter().map(|t| t.into()).collect()
    }

    #[cfg(feature = "wgsl")]
    pub fn wgsl(&self) -> &str {
        &self.0.wgsl.code
    }

    #[cfg(feature = "msl")]
    pub fn msl(&self) -> &str {
        &self.0.msl.code
    }
}

macro_rules! decl_shader {
    ($name:ident, $field:tt) => {
        const $name: Shader = Shader(&vello_shaders::SHADERS.$field);
    };
}

decl_shader!(BACKDROP, backdrop);
decl_shader!(BACKDROP_DYN, backdrop_dyn);
decl_shader!(BBOX_CLEAR, bbox_clear);
decl_shader!(BINNING, binning);
decl_shader!(CLIP_LEAF, clip_leaf);
decl_shader!(CLIP_REDUCE, clip_reduce);
decl_shader!(COARSE, coarse);
decl_shader!(DRAW_LEAF, draw_leaf);
decl_shader!(DRAW_REDUCE, draw_reduce);
decl_shader!(FINE, fine);
decl_shader!(PATH_COARSE, path_coarse);
decl_shader!(PATH_COARSE_FULL, path_coarse_full);
decl_shader!(PATHSEG, pathseg);
decl_shader!(PATHTAG_REDUCE, pathtag_reduce);
decl_shader!(PATHTAG_REDUCE2, pathtag_reduce2);
decl_shader!(PATHTAG_SCAN1, pathtag_scan1);
decl_shader!(PATHTAG_SCAN_LARGE, pathtag_scan_large);
decl_shader!(PATHTAG_SCAN_SMALL, pathtag_scan_small);
decl_shader!(TILE_ALLOC, tile_alloc);

pub(crate) fn shader(stage: ffi::ShaderStage) -> &'static Shader {
    match stage {
        ffi::ShaderStage::Backdrop => &BACKDROP,
        ffi::ShaderStage::BackdropDyn => &BACKDROP_DYN,
        ffi::ShaderStage::BboxClear => &BBOX_CLEAR,
        ffi::ShaderStage::Binning => &BINNING,
        ffi::ShaderStage::ClipLeaf => &CLIP_LEAF,
        ffi::ShaderStage::ClipReduce => &CLIP_REDUCE,
        ffi::ShaderStage::Coarse => &COARSE,
        ffi::ShaderStage::DrawLeaf => &DRAW_LEAF,
        ffi::ShaderStage::DrawReduce => &DRAW_REDUCE,
        ffi::ShaderStage::Fine => &FINE,
        ffi::ShaderStage::PathCoarse => &PATH_COARSE,
        ffi::ShaderStage::PathCoarseFull => &PATH_COARSE_FULL,
        ffi::ShaderStage::Pathseg => &PATHSEG,
        ffi::ShaderStage::PathtagReduce => &PATHTAG_REDUCE,
        ffi::ShaderStage::PathtagReduce2 => &PATHTAG_REDUCE2,
        ffi::ShaderStage::PathtagScan1 => &PATHTAG_SCAN1,
        ffi::ShaderStage::PathtagScanLarge => &PATHTAG_SCAN_LARGE,
        ffi::ShaderStage::PathtagScanSmall => &PATHTAG_SCAN_SMALL,
        ffi::ShaderStage::TileAlloc => &TILE_ALLOC,
        _ => unreachable!(),
    }
}

impl From<&BindType> for ffi::BindType {
    fn from(src: &BindType) -> Self {
        match src {
            BindType::Buffer => Self::Buffer,
            BindType::BufReadOnly => Self::BufReadOnly,
            BindType::Uniform => Self::Uniform,
            BindType::Image => Self::Image,
            BindType::ImageRead => Self::ImageRead,
        }
    }
}

impl From<&WorkgroupBufferInfo> for ffi::WorkgroupBufferInfo {
    fn from(src: &WorkgroupBufferInfo) -> Self {
        Self {
            size_in_bytes: src.size_in_bytes,
            index: src.index,
        }
    }
}

impl From<[u32; 3]> for ffi::WorkgroupSize {
    fn from(src: [u32; 3]) -> Self {
        Self {
            x: src[0],
            y: src[1],
            z: src[2],
        }
    }
}
