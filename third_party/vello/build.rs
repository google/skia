// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

fn main() {
    cxx_build::bridge("src/lib.rs")
        .flag_if_supported("-std=c++17")
        .include("../../")
        .compile("vello_cpp");
    println!("cargo:rerun-if-changed=src/");
    println!("cargo:rerun-if-changed=include/");
}
