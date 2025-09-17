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
// `dyn std::io::Read + std::io::Seek` from Rust.
//
// TODO(https://crbug.com/399894620): Implement `BufRead` trait once/if this is
// supported by the `SkStream` API.
class ReadAndSeekTraits {
public:
    // The `virtual` method below is a rough equivalent of the
    // `std::io::Read::read` method in Rust. See
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    //
    // Note that (unlike in Rust) this method is infallible.  This aspect of the
    // design is used tentatively and based on the following considerations:
    // * `SkStream::read` is also infallible
    // * `SkStream::read` communicates no-more-input by reporting that 0 bytes
    //   have been read (just like Rust's `Read` trait), and this condition
    //   corresponds to the only IO error covered by `SkCodec::Result`:
    //   `kIncompleteInput`.
    // * FFI based on the `cxx` crate doesn't currently support Rust's
    //   `std::result::Result`, `std::option::Option`, nor C++'s
    //   `std::optional` or `std::expected`.
    virtual size_t read(rust::Slice<uint8_t> buffer) = 0;

    // The 3 `virtual` methods below are roughly equivalent to the
    // `std::io::Seek::seek` method in Rust. See
    // https://doc.rust-lang.org/beta/std/io/trait.Seek.html#tymethod.seek
    // for guidance on the desired implementation and behavior of this method.
    //
    // These methods should return `true` when successful, and `false` upon
    // failure.  When successful the methods should set `final_pos` to the new
    // position from the start of the stream.
    virtual bool seek_from_start(uint64_t requested_pos, uint64_t& final_pos) = 0;
    virtual bool seek_from_end(int64_t requested_offset, uint64_t& final_pos) = 0;
    virtual bool seek_relative(int64_t requested_offset, uint64_t& final_pos) = 0;

    // `drop` is not part of the `std::io::Read` nor `std::io::Seek` trait, but
    // we expose the destructor through public API to make it possible to pass
    // `std::unique_ptr<ReadAndSeekTraits>` over the FFI boundary.
    virtual ~ReadAndSeekTraits() = default;

    // This type is non-copyable and non-movable.
    ReadAndSeekTraits(const ReadAndSeekTraits&) = delete;
    ReadAndSeekTraits(ReadAndSeekTraits&&) = delete;
    ReadAndSeekTraits& operator=(const ReadAndSeekTraits&) = delete;
    ReadAndSeekTraits& operator=(ReadAndSeekTraits&&) = delete;

protected:
    ReadAndSeekTraits() = default;
};

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

    // `drop` is not part of the `std::io::Read` trait, but we expose the
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
