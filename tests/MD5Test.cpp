/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMD5.h"
#include "Test.h"

static bool digests_equal(const SkMD5::Digest& expectedDigest, const SkMD5::Digest& computedDigest) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(expectedDigest.data); ++i) {
        if (expectedDigest.data[i] != computedDigest.data[i]) {
            return false;
        }
    }
    return true;
}

static void md5_test(const char* string, const SkMD5::Digest& expectedDigest, skiatest::Reporter* reporter) {
    size_t len = strlen(string);

    // All at once
    {
        SkMD5 context;
        context.write(string, len);
        SkMD5::Digest digest = context.finish();

        REPORTER_ASSERT(reporter, digests_equal(expectedDigest, digest));
    }

    // One byte at a time.
    {
        SkMD5 context;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(string);
        const uint8_t* end = reinterpret_cast<const uint8_t*>(string + len);
        for (; data < end; ++data) {
            context.write(data, 1);
        }
        SkMD5::Digest digest = context.finish();

        REPORTER_ASSERT(reporter, digests_equal(expectedDigest, digest));
    }
}

static struct MD5Test {
    const char* message;
    SkMD5::Digest digest;
} md5_tests[] = {
    // Reference tests from RFC1321 Section A.5 ( http://www.ietf.org/rfc/rfc1321.txt )
    { "", {{ 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e }} },
    { "a", {{ 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 }} },
    { "abc", {{ 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 }} },
    { "message digest", {{ 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 }} },
    { "abcdefghijklmnopqrstuvwxyz", {{ 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b }} },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", {{ 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f }} },
    { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", {{ 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a }} },
};

DEF_TEST(MD5, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(md5_tests); ++i) {
        md5_test(md5_tests[i].message, md5_tests[i].digest, reporter);
    }
}
