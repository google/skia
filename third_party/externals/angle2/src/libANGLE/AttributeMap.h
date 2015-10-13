//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_ATTRIBUTEMAP_H_
#define LIBANGLE_ATTRIBUTEMAP_H_


#include <EGL/egl.h>

#include <map>

namespace egl
{

class AttributeMap final
{
  public:
    AttributeMap();
    explicit AttributeMap(const EGLint *attributes);

    void insert(EGLint key, EGLint value);
    bool contains(EGLint key) const;
    EGLint get(EGLint key, EGLint defaultValue) const;

    typedef std::map<EGLint, EGLint>::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

  private:
    std::map<EGLint, EGLint> mAttributes;
};

}

#endif   // LIBANGLE_ATTRIBUTEMAP_H_
