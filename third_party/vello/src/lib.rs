// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#[cxx::bridge(namespace = "vello_cpp")]
mod ffi {
    // === vello_shaders FFI types ===

    enum ShaderStage {
        Backdrop,
        BackdropDyn,
        BboxClear,
        Binning,
        ClipLeaf,
        ClipReduce,
        Coarse,
        DrawLeaf,
        DrawReduce,
        Fine,
        PathCoarse,
        PathCoarseFull,
        Pathseg,
        PathtagReduce,
        PathtagReduce2,
        PathtagScan1,
        PathtagScanLarge,
        PathtagScanSmall,
        TileAlloc,
    }

    enum BindType {
        Buffer,
        BufReadOnly,
        Uniform,
        Image,
        ImageRead,
    }

    struct WorkgroupBufferInfo {
        size_in_bytes: u32,
        index: u32,
    }

    // === vello_encoding FFI types ===

    /// Represents a 3x3 affine transformation matrix. The coordinates are laid out using the
    /// following column-major construction, where the entries
    ///
    ///    [a, b, c, d, e, f]
    ///
    /// correspond to the matrix
    ///
    ///    | a c e |
    ///    | b d f |
    ///    | 0 0 1 |
    ///
    struct Affine {
        matrix: [f32; 6],
    }

    struct Color {
        r: u8,
        g: u8,
        b: u8,
        a: u8,
    }

    enum BrushKind {
        Solid,
    }

    struct BrushData {
        solid: Color,
    }

    struct Brush {
        kind: BrushKind,
        data: BrushData,
    }

    enum Fill {
        NonZero,
        EvenOdd,
    }

    struct Stroke {
        width: f32,
    }

    #[derive(Copy, Clone, Default, Debug)]
    struct Point {
        x: f32,
        y: f32,
    }

    #[derive(Debug)]
    enum PathVerb {
        MoveTo,
        LineTo,
        QuadTo,
        CurveTo,
        Close,
    }

    #[derive(Default, Debug)]
    struct PathElement {
        verb: PathVerb,
        points: [Point; 4],
    }

    // Types that contain dispatch metadata

    struct WorkgroupSize {
        x: u32,
        y: u32,
        z: u32,
    }

    struct DispatchInfo {
        use_large_path_scan: bool,

        // Dispatch workgroup counts for each pipeline
        path_reduce: WorkgroupSize,
        path_reduce2: WorkgroupSize,
        path_scan1: WorkgroupSize,
        path_scan: WorkgroupSize,
        bbox_clear: WorkgroupSize,
        path_seg: WorkgroupSize,
        draw_reduce: WorkgroupSize,
        draw_leaf: WorkgroupSize,
        clip_reduce: WorkgroupSize,
        clip_leaf: WorkgroupSize,
        binning: WorkgroupSize,
        tile_alloc: WorkgroupSize,
        path_coarse: WorkgroupSize,
        backdrop: WorkgroupSize,
        coarse: WorkgroupSize,
        fine: WorkgroupSize,
    }

    /// Computed sizes for all buffers.
    struct BufferSizes {
        // Known size buffers
        path_reduced: u32,
        path_reduced2: u32,
        path_reduced_scan: u32,
        path_monoids: u32,
        path_bboxes: u32,
        cubics: u32,
        draw_reduced: u32,
        draw_monoids: u32,
        info: u32,
        clip_inps: u32,
        clip_els: u32,
        clip_bics: u32,
        clip_bboxes: u32,
        draw_bboxes: u32,
        bump_alloc: u32,
        bin_headers: u32,
        paths: u32,
        // Bump allocated buffers
        bin_data: u32,
        tiles: u32,
        segments: u32,
        ptcl: u32,
    }

    extern "Rust" {
        type Shader;
        fn shader(stage: ShaderStage) -> &'static Shader;
        fn name(self: &Shader) -> &str;
        fn workgroup_size(self: &Shader) -> WorkgroupSize;
        fn bindings(self: &Shader) -> Vec<BindType>;
        fn workgroup_buffers(self: &Shader) -> Vec<WorkgroupBufferInfo>;
        #[cfg(feature = "wgsl")]
        fn wgsl(self: &Shader) -> &str;
        #[cfg(feature = "msl")]
        fn msl(self: &Shader) -> &str;

        type Encoding;
        fn new_encoding() -> Box<Encoding>;
        fn is_empty(self: &Encoding) -> bool;
        fn reset(self: &mut Encoding);
        fn fill(
            self: &mut Encoding,
            style: Fill,
            transform: Affine,
            brush: &Brush,
            path_iter: Pin<&mut PathIterator>,
        );
        fn stroke(
            self: &mut Encoding,
            style: &Stroke,
            transform: Affine,
            brush: &Brush,
            path_iter: Pin<&mut PathIterator>,
        );
        fn begin_clip(self: &mut Encoding, transform: Affine, path_iter: Pin<&mut PathIterator>);
        fn end_clip(self: &mut Encoding);
        fn prepare_render(
            self: &Encoding,
            width: u32,
            height: u32,
            background: &Color,
        ) -> Box<RenderConfiguration>;

        /// The resolved scene encoding metadata that can be used to initiate pipeline dispatches.
        type RenderConfiguration;
        fn config_uniform_buffer_size(self: &RenderConfiguration) -> usize;
        fn scene_buffer_size(self: &RenderConfiguration) -> usize;
        fn write_config_uniform_buffer(self: &RenderConfiguration, out_buffer: &mut [u8]) -> bool;
        fn write_scene_buffer(self: &RenderConfiguration, out_buffer: &mut [u8]) -> bool;
        fn workgroup_counts(self: &RenderConfiguration) -> DispatchInfo;
        fn buffer_sizes(self: &RenderConfiguration) -> BufferSizes;
    }

    unsafe extern "C++" {
        include!("third_party/vello/cpp/path_iterator.h");

        type PathIterator;
        unsafe fn next_element(self: Pin<&mut PathIterator>, out_elem: *mut PathElement) -> bool;
    }
}

mod encoding;
mod shaders;

use {
    encoding::{new_encoding, Encoding, RenderConfiguration},
    shaders::{shader, Shader},
};
