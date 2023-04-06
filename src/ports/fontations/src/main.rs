// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

use read_fonts::{FileRef, FileRef::Font, TableProvider};
use std::fs;

fn main() {
    let contents = fs::read("resources/fonts/test_glyphs-glyf_colr_1_variable.ttf")
        .expect("File read should have succeeded.");
    if let Font(font) = FileRef::new(&contents).unwrap() {
        match font.maxp() {
            Ok(t) => println!("Number of glyphs in font: {}", t.num_glyphs()),
            Err(e) => println!("Could not parse or retrieve maxp table. {:?}", e),
        }
    }
}
