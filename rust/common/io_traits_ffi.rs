// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! FFI bridge for common I/O traits shared across codec modules.
//!
//! This module provides CXX bindings for stream adapters that can be used
//! across different codec implementations (PNG, BMP, JPEG, etc.).
//!
//! This file will be replaced by the implementation in:
//! https://skia-review.googlesource.com/c/skia/+/1093916

#[cxx::bridge(namespace = "rust::stream")]
pub mod ffi {
    // Placeholder module. Real bindings land with
    // https://skia-review.googlesource.com/c/skia/+/1093916.
}
