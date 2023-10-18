/*
 * Copyright 2022 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMesh.h"
#include "include/private/base/SkTArray.h"

#include "fuzz/Fuzz.h"

using namespace skia_private;

template <typename T>
T extract(SkSpan<const uint8_t>& data) {
    T result = 0;
    size_t bytesToCopy = std::min(sizeof(T), data.size());
    if (bytesToCopy > 0) {
        memcpy(&result, &data.front(), bytesToCopy);
        data = data.subspan(bytesToCopy);
    }
    return result;
}

void FuzzSkMeshSpecification(const uint8_t *fuzzData, size_t fuzzSize) {
    using Attribute = SkMeshSpecification::Attribute;
    using Varying = SkMeshSpecification::Varying;

    SkSpan<const uint8_t> data(fuzzData, fuzzSize);
    STArray<SkMeshSpecification::kMaxAttributes, Attribute> attributes;
    STArray<SkMeshSpecification::kMaxVaryings,   Varying>   varyings;
    size_t vertexStride;
    SkString vs, fs;

    auto fuzzByteToASCII = [&](uint8_t c, SkString* str) -> bool {
        // Most control characters (including \0) and all high ASCII are treated as stop bytes.
        if ((c >= 32 && c <= 127) || c == '\r' || c == '\n' || c == '\t') {
            char ascii = c;
            str->append(&ascii, 1);
            return true;
        }
        return false;
    };

    auto fuzzByteToSkSL = [&](uint8_t c, SkString* str) -> bool {
        // In the 0x00 - 0x80 range, treat characters as ASCII.
        if (c < 128) {
            return fuzzByteToASCII(c, str);
        }
        c -= 128;

        // Dedicate a few bytes to injecting our attribute and varying names.
        if (c < SkMeshSpecification::kMaxAttributes) {
            if (!attributes.empty()) {
                str->append(attributes[c % attributes.size()].name);
            }
            return true;
        }
        c -= SkMeshSpecification::kMaxAttributes;

        if (c < SkMeshSpecification::kMaxVaryings) {
            if (!varyings.empty()) {
                str->append(varyings[c % varyings.size()].name);
            }
            return true;
        }
        c -= SkMeshSpecification::kMaxVaryings;

        // Replace the remaining high-ASCII bytes with valid SkSL operators and keywords in order to
        // improve our chances of generating a program. (We omit single-character operators since
        // single-byte versions of those already exist in the low-ASCII space.)
        static constexpr std::string_view kSkSLData[] = {
                " true ",
                " false ",
                " if ",
                " else ",
                " for ",
                " while ",
                " do ",
                " switch ",
                " case ",
                " default ",
                " break ",
                " continue ",
                " discard ",
                " return ",
                " in ",
                " out ",
                " inout ",
                " uniform ",
                " const ",
                " flat ",
                " noperspective ",
                " inline ",
                " noinline ",
                " $pure ",
                " readonly ",
                " writeonly ",
                " buffer ",
                " struct ",
                " layout ",
                " highp ",
                " mediump ",
                " lowp ",
                " $es3 ",
                " $export ",
                " workgroup ",
                " << ",
                " >> ",
                " && ",
                " || ",
                " ^^ ",
                " == ",
                " != ",
                " <= ",
                " >= ",
                " += ",
                " -= ",
                " *= ",
                " /= ",
                " %= ",
                " <<= ",
                " >>= ",
                " &= ",
                " |= ",
                " ^= ",
                " ++ ",
                " -- ",
                " //",
                " /*",
                "*/ ",
                " float",
                " half",
                " int",
                " uint",
                " short",
                " ushort",
                " bool",
                " void",
                " vec",
                " ivec",
                " bvec",
                " mat",
                " Attributes ",
                " Varyings ",
        };

        c %= std::size(kSkSLData);
        str->append(kSkSLData[c]);
        return true;
    };

    // Pick a vertex stride; intentionally allow some bad values through.
    vertexStride = extract<uint16_t>(data) % (SkMeshSpecification::kMaxStride + 2);

    while (!data.empty()) {
        uint8_t control = extract<uint8_t>(data) % 4;
        // A control code with no payload can be ignored.
        if (data.empty()) {
            break;
        }
        switch (control) {
            case 0: {
                // Add an attribute.
                Attribute& a = attributes.push_back();
                a.type = (Attribute::Type)(extract<uint8_t>(data) %
                                           ((int)Attribute::Type::kLast + 1));
                a.offset = extract<uint16_t>(data) % (SkMeshSpecification::kMaxStride + 2);
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToASCII(c, &a.name)) {
                        break;
                    }
                }
                break;
            }
            case 1: {
                // Add a varying.
                Varying& v = varyings.push_back();
                v.type = (Varying::Type)(extract<uint8_t>(data) % ((int)Varying::Type::kLast + 1));
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToASCII(c, &v.name)) {
                        break;
                    }
                }
                break;
            }
            case 2: {
                // Convert the following data into SkSL and add it into the vertex program.
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &vs)) {
                        break;
                    }
                }
                break;
            }
            case 3: {
                // Convert the following data into SkSL and add it into the fragment program.
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &fs)) {
                        break;
                    }
                }
                break;
            }
        }
    }

    auto result = SkMeshSpecification::Make(attributes, vertexStride, varyings, vs, fs);
    if (result.error.isEmpty()) {
        // TODO: synthesize a mesh with this specification and paint it.
        printf("----\n%s\n----\n\n----\n%s\n----\n\n\n", vs.c_str(), fs.c_str());
    }
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 8000) {
        return 0;
    }
    FuzzSkMeshSpecification(data, size);
    return 0;
}
#endif
