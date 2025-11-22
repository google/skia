// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Common FFI utilities for Rust codec integrations.
//!
//! This module provides the SkStreamAdapter type and its Read/Seek trait
//! implementations. Codec modules should use a type alias to reference this type.

use std::io::{ErrorKind, Read, Seek, SeekFrom};
use std::pin::Pin;

#[cxx::bridge(namespace = "rust::stream")]
mod ffi {
    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        // Declare SkStreamAdapter with rust::stream namespace for type unification
        #[namespace = "rust::stream"]
        type SkStreamAdapter;

        fn read(self: Pin<&mut SkStreamAdapter>, buffer: &mut [u8]) -> usize;

        fn seek_from_start(
            self: Pin<&mut SkStreamAdapter>,
            requested_pos: u64,
            final_pos: &mut u64,
        ) -> bool;

        fn seek_from_end(
            self: Pin<&mut SkStreamAdapter>,
            requested_offset: i64,
            final_pos: &mut u64,
        ) -> bool;

        fn seek_relative(
            self: Pin<&mut SkStreamAdapter>,
            requested_offset: i64,
            final_pos: &mut u64,
        ) -> bool;
    }

    // Explicitly declare UniquePtr of the type so cxx bridge materializes
    // UniquePtr support
    impl UniquePtr<SkStreamAdapter> {}
}

// Re-export the SkStreamAdapter type for use in other modules
pub use ffi::SkStreamAdapter;

impl<'a> Read for Pin<&'a mut SkStreamAdapter> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        let bytes_read = self.as_mut().read(buf);
        Ok(bytes_read)
    }
}

impl<'a> Seek for Pin<&'a mut SkStreamAdapter> {
    fn seek(&mut self, pos: SeekFrom) -> std::io::Result<u64> {
        let mut final_pos: u64 = 0;
        let success = match pos {
            SeekFrom::Start(requested_pos) => {
                self.as_mut().seek_from_start(requested_pos, &mut final_pos)
            }
            SeekFrom::End(requested_offset) => self
                .as_mut()
                .seek_from_end(requested_offset, &mut final_pos),
            SeekFrom::Current(requested_offset) => self
                .as_mut()
                .seek_relative(requested_offset, &mut final_pos),
        };
        if success {
            Ok(final_pos)
        } else {
            Err(ErrorKind::Other.into())
        }
    }
}
