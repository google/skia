/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustIccFFI_DEFINED
#define SkRustIccFFI_DEFINED

#include <stddef.h>
#include <stdint.h>

// Forward declare skcms types
struct skcms_Matrix3x3;
struct skcms_TransferFunction;
struct skcms_ICCProfile;

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_icc {

struct Matrix3x3;
struct TransferFunction;
struct IccProfile;

/// Wrapper for skcms_ApproximateCurve.
/// Used by cxx::bridge to avoid manual extern "C" declarations.
bool ApproximateCurveWrapper(rust::Slice<const uint16_t> table,
                             TransferFunction& out_approx,
                             float& out_max_error);

/// Convert rust_icc::Matrix3x3 to skcms_Matrix3x3.
void ToSkcmsMatrix3x3(const Matrix3x3& rust_matrix, skcms_Matrix3x3* out_skcms);

/// Convert rust_icc::TransferFunction to skcms_TransferFunction.
void ToSkcmsTransferFunction(const TransferFunction& rust_tf, skcms_TransferFunction* out_skcms);

/// Convert rust_icc::IccProfile to skcms_ICCProfile.
///
/// Populates all skcms fields from moxcms-parsed ICC data:
///   - Color space metadata
///   - toXYZD50 matrix and transfer curves (for simple RGB/Gray profiles)
///   - CICP metadata (for HDR)
///   - A2B/B2A transforms (for complex LUT-based profiles)
///
/// Note: The following skcms_ICCProfile members will NOT be populated:
///   - buffer: will be set to nullptr (raw ICC bytes not retained)
///   - size: will be set to 0
///   - tag_count: will be set to 0
///
/// All LUT data is owned by Rust; C++ accesses via pointers.
///
/// IMPORTANT: The caller must guarantee that the lifetime of `out_skcms` does not
/// exceed the lifetime of `rust_profile`. Pointers in `out_skcms` (e.g., for curve
/// table data) reference memory owned by the Rust-side `IccProfile`, which will be
/// invalidated when `rust_profile` is destroyed.
///
/// TODO: Consider propagating 'input lifetime to enforce this at compile time.
///
/// Returns true if successful, false if essential data is missing.
bool ToSkcmsIccProfile(const IccProfile& rust_profile, skcms_ICCProfile* out_skcms);

}  // namespace rust_icc

#endif  // SkRustIccFFI_DEFINED
