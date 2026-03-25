/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustExifFFI_DEFINED
#define SkRustExifFFI_DEFINED

#include <cstdint>

// Forward declare SkExif types. Callers must also include
// "include/private/SkExif.h" to access the full Metadata definition.
namespace SkExif {
struct Metadata;
}  // namespace SkExif

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T>
class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_exif {

struct ExifMetadata;

/// Convert rust_exif::ExifMetadata to SkExif::Metadata.
///
/// Maps each `has_*` / value pair in `rust_meta` to the corresponding
/// `std::optional` field in `*out`. The `out` pointer must be non-null.
///
/// There are no pointer relationships between rust_meta and out; all values
/// are copied, so there are no lifetime constraints between the two structs.
void ToSkExifMetadata(const ExifMetadata& rust_meta, SkExif::Metadata* out);

}  // namespace rust_exif

#endif  // SkRustExifFFI_DEFINED
