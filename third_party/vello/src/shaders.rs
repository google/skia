// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use crate::ffi;
use vello_shaders::{BindType, WorkgroupBufferInfo};

pub(crate) struct Shaders(&'static vello_shaders::Shaders<'static>);

macro_rules! match_stage_field {
    ($shaders:ident, $stage:ident, $field:tt) => {
        match $stage {
            ffi::ShaderStage::Backdrop => &$shaders.0.backdrop.$field,
            ffi::ShaderStage::BackdropDyn => &$shaders.0.backdrop_dyn.$field,
            ffi::ShaderStage::BboxClear => &$shaders.0.bbox_clear.$field,
            ffi::ShaderStage::Binning => &$shaders.0.binning.$field,
            ffi::ShaderStage::ClipLeaf => &$shaders.0.clip_leaf.$field,
            ffi::ShaderStage::ClipReduce => &$shaders.0.clip_reduce.$field,
            ffi::ShaderStage::Coarse => &$shaders.0.coarse.$field,
            ffi::ShaderStage::DrawLeaf => &$shaders.0.draw_leaf.$field,
            ffi::ShaderStage::DrawReduce => &$shaders.0.draw_reduce.$field,
            ffi::ShaderStage::Fine => &$shaders.0.fine.$field,
            ffi::ShaderStage::PathCoarse => &$shaders.0.path_coarse.$field,
            ffi::ShaderStage::PathCoarseFull => &$shaders.0.path_coarse_full.$field,
            ffi::ShaderStage::Pathseg => &$shaders.0.pathseg.$field,
            ffi::ShaderStage::PathtagReduce => &$shaders.0.pathtag_reduce.$field,
            ffi::ShaderStage::PathtagReduce2 => &$shaders.0.pathtag_reduce2.$field,
            ffi::ShaderStage::PathtagScan1 => &$shaders.0.pathtag_scan1.$field,
            ffi::ShaderStage::PathtagScanLarge => &$shaders.0.pathtag_scan_large.$field,
            ffi::ShaderStage::PathtagScanSmall => &$shaders.0.pathtag_scan_small.$field,
            ffi::ShaderStage::TileAlloc => &$shaders.0.tile_alloc.$field,
            _ => unreachable!(),
        }
    };
}

impl Shaders {
    pub fn name(&self, stage: ffi::ShaderStage) -> &str {
        match_stage_field!(self, stage, name)
    }

    pub fn code(&self, stage: ffi::ShaderStage) -> &[u8] {
        match_stage_field!(self, stage, code)
    }

    pub fn workgroup_size(&self, stage: ffi::ShaderStage) -> ffi::WorkgroupSize {
        match_stage_field!(self, stage, workgroup_size).into()
    }

    pub fn bindings(&self, stage: ffi::ShaderStage) -> Vec<ffi::BindType> {
        match_stage_field!(self, stage, bindings)
            .iter()
            .map(|t| t.into())
            .collect()
    }

    pub fn workgroup_buffers(
        self: &Shaders,
        stage: ffi::ShaderStage,
    ) -> Vec<ffi::WorkgroupBufferInfo> {
        match_stage_field!(self, stage, workgroup_buffers)
            .iter()
            .map(|t| t.into())
            .collect()
    }
}

const WGSL_SHADERS: Shaders = Shaders(&vello_shaders::wgsl::SHADERS);
const MSL_SHADERS: Shaders = Shaders(&vello_shaders::msl::SHADERS);

pub(crate) fn wgsl() -> &'static Shaders {
    &WGSL_SHADERS
}

pub(crate) fn msl() -> &'static Shaders {
    &MSL_SHADERS
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

impl From<&[u32; 3]> for ffi::WorkgroupSize {
    fn from(src: &[u32; 3]) -> Self {
        Self {
            x: src[0],
            y: src[1],
            z: src[2],
        }
    }
}
