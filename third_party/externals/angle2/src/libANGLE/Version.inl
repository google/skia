//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.inl: Encapsulation of a GL version.

namespace gl
{

inline Version::Version()
    : Version(0, 0)
{
}

inline Version::Version(GLuint major_, GLuint minor_)
{
    major = major_;
    minor = minor_;
}

inline bool operator>=(const Version &a, const Version &b)
{
    return a.major > b.major || (a.major == b.major && a.minor >= b.minor);
}

inline bool operator<(const Version &a, const Version &b)
{
    return !(a >= b);
}

}
