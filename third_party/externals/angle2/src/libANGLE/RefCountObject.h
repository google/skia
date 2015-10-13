//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RefCountObject.h: Defines the gl::RefCountObject base class that provides
// lifecycle support for GL objects using the traditional BindObject scheme, but
// that need to be reference counted for correct cross-context deletion.
// (Concretely, textures, buffers and renderbuffers.)

#ifndef LIBANGLE_REFCOUNTOBJECT_H_
#define LIBANGLE_REFCOUNTOBJECT_H_

#include "common/debug.h"

#include "angle_gl.h"

#include <cstddef>

class RefCountObject : angle::NonCopyable
{
  public:
    explicit RefCountObject(GLuint id);
    virtual ~RefCountObject();

    virtual void addRef() const;
    virtual void release() const;

    GLuint id() const { return mId; }

    size_t getRefCount() const { return mRefCount; }

  private:
    GLuint mId;

    mutable std::size_t mRefCount;
};

template <class ObjectType>
class BindingPointer
{
  public:
    BindingPointer()
        : mObject(nullptr)
    {
    }

    BindingPointer(const BindingPointer<ObjectType> &other)
        : mObject(nullptr)
    {
        set(other.mObject);
    }

    void operator=(const BindingPointer<ObjectType> &other)
    {
        set(other.mObject);
    }

    virtual ~BindingPointer()
    {
        // Objects have to be released before the resource manager is destroyed, so they must be explicitly cleaned up.
        ASSERT(mObject == nullptr);
    }

    virtual void set(ObjectType *newObject)
    {
        // addRef first in case newObject == mObject and this is the last reference to it.
        if (newObject != nullptr) reinterpret_cast<const RefCountObject*>(newObject)->addRef();
        if (mObject != nullptr) reinterpret_cast<const RefCountObject*>(mObject)->release();
        mObject = newObject;
    }

    ObjectType *get() const { return mObject; }
    ObjectType *operator->() const { return mObject; }

    GLuint id() const { return (mObject != nullptr) ? mObject->id() : 0; }

  private:
    ObjectType *mObject;
};

template <class ObjectType>
class OffsetBindingPointer : public BindingPointer<ObjectType>
{
  public:
    OffsetBindingPointer() : mOffset(0), mSize(0) { }

    void set(ObjectType *newObject) override
    {
        BindingPointer<ObjectType>::set(newObject);
        mOffset = 0;
        mSize = 0;
    }

    void set(ObjectType *newObject, GLintptr offset, GLsizeiptr size)
    {
        BindingPointer<ObjectType>::set(newObject);
        mOffset = offset;
        mSize = size;
    }

    GLintptr getOffset() const { return mOffset; }
    GLsizeiptr getSize() const { return mSize; }

  private:
    GLintptr mOffset;
    GLsizeiptr mSize;
};

#endif   // LIBANGLE_REFCOUNTOBJECT_H_
