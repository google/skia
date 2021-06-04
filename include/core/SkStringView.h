/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStringView_DEFINED
#define SkStringView_DEFINED

#include <string>

namespace skstd {

class string_view {
public:
    using value_type = char;
    using traits_type = std::char_traits<value_type>;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using iterator = const_pointer;
    using const_iterator = iterator;
    using size_type = size_t;

    string_view()
        : fData(nullptr)
        , fLength(0) {}

    string_view(const string_view&) = default;

    string_view(const_pointer data, size_type length)
        : fData(data)
        , fLength(length) {}

    string_view(const_pointer data)
        : string_view(data, strlen(data)) {}

    string_view& operator=(const string_view&) = default;

    iterator begin() const {
        return fData;
    }

    iterator end() const {
        return fData + fLength;
    }

    const_reference operator[](size_type idx) const {
        return fData[idx];
    }

    const_reference front() const {
        return fData[0];
    }

    const_reference back() const {
        return fData[fLength - 1];
    }

    const_pointer data() const {
        return fData;
    }

    size_type size() const {
        return fLength;
    }

    size_type length() const {
        return fLength;
    }

    bool empty() const {
        return fLength == 0;
    }

    bool starts_with(string_view s) const {
        if (s.length() > fLength) {
            return false;
        }
        return s.length() == 0 || !memcmp(fData, s.fData, s.length());
    }

    bool starts_with(value_type c) const {
        return !this->empty() && this->front() == c;
    }

    bool ends_with(string_view s) const {
        if (s.length() > fLength) {
            return false;
        }
        return s.length() == 0 || !memcmp(this->end() - s.length(), s.fData, s.length());
    }

    bool ends_with(value_type c) const {
        return !this->empty() && this->back() == c;
    }

private:
    const_pointer fData;
    size_type fLength;
};

bool operator==(string_view left, string_view right);

bool operator!=(string_view left, string_view right);

} // namespace skstd

#endif
