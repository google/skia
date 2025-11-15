/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustPngFFI_DEFINED
#define SkRustPngFFI_DEFINED

#include <stddef.h>
#include <stdint.h>

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_png {

// Implementing the abstract C++ class below gives a rough equivalent of
// `dyn std::io::Write` from Rust.
class WriteTrait {
public:
    // The `virtual` method below is a rough equivalent of the
    // `std::io::Write::write` method in Rust. See
    // https://doc.rust-lang.org/nightly/std/io/trait.Write.html#tymethod.write
    // for guidance on the desired implementation and behavior of this method.
    //
    // Implementation should return `true` if the whole buffer has been
    // successfully written (and return `false` to indicate an error).
    // This mimics how `SkStreamW::write` communicates errors.
    virtual bool write(rust::Slice<const uint8_t> buffer) = 0;

    // The `virtual` method below is a rough equivalent of the
    // `std::io::Write::flush` method in Rust. See
    // https://doc.rust-lang.org/nightly/std/io/trait.Write.html#tymethod.flush
    // for guidance on the desired implementation and behavior of this method.
    //
    // Note that (unlike in Rust) this method is infallible.  This aspect of the
    // design mimics `SkStreamW::flush`.
    virtual void flush() = 0;

    // `drop` is not part of the `std::io::Write` trait, but we expose the
    // destructor through public API to make it possible to pass
    // `std::unique_ptr<WriteTrait>` over the FFI boundary.
    virtual ~WriteTrait() = default;

    // This type is non-copyable and non-movable.
    WriteTrait(const WriteTrait&) = delete;
    WriteTrait(WriteTrait&&) = delete;
    WriteTrait& operator=(const WriteTrait&) = delete;
    WriteTrait& operator=(WriteTrait&&) = delete;

protected:
    WriteTrait() = default;
};

}  // namespace rust_png

#endif  // SkRustPngFFI_DEFINED
