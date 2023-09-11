// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_VELLO_INCLUDE_VELLO_CPP_PATH_ITERATOR_H_
#define THIRD_PARTY_VELLO_INCLUDE_VELLO_CPP_PATH_ITERATOR_H_

namespace vello_cpp {

struct PathElement;

class PathIterator {
public:
    virtual ~PathIterator() = default;

    virtual bool next_element(PathElement* out_elem) = 0;
};

}  // namespace vello_cpp

#endif  // THIRD_PARTY_VELLO_INCLUDE_VELLO_CPP_PATH_ITERATOR_H_
