
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLNameAllocator_DEFINED
#define GrGLNameAllocator_DEFINED

#include "SkRefCnt.h"
#include "gl/GrGLTypes.h"

/**
 * This class assumes ownership of an explicit range of OpenGL object names and
 * manages allocations within that range. This allows the app to generate new
 * objects on the client side without making round trips to the GL server.
 */
class GrGLNameAllocator {
public:
    /**
     * Constructs a name allocator that produces names within the explicit
     * half-open range [firstName, end). Note that the caller will most likely
     * need to call glGen* beforehand to reserve a range within the GL driver,
     * and then invoke this constructor with that range.
     *
     * @param firstName The first name in the range owned by this class. Must be
                        greater than zero.
     * @param endName   The first past-the-end name beyond the range owned by
                        this class. Must be >= firstName.
     */
    GrGLNameAllocator(GrGLuint firstName, GrGLuint endName);

    /**
     * Destructs the name allocator. The caller is responsible for calling the
     * appropriate glDelete* on the range if necessary.
     */
    ~GrGLNameAllocator();

    /**
     * Return the beginning of this class's range.
     *
     * @return The first name in the range owned by this class.
     */
    GrGLuint firstName() const { return fFirstName; }

    /**
     * Return the end of this class's range. Note that endName() is not owned by
     * this class.
     *
     * @return One plus the final name in the range owned by this class.
     */
    GrGLuint endName() const { return fEndName; }

    /**
     * Allocate an OpenGL object name from within this class's range.
     *
     * @return The name if one was available,
               0 if all the names in the range were already in use.
     */
    GrGLuint allocateName();

    /**
     * Free an OpenGL object name, allowing it to be returned by a future call
     * to allocateName(). Note that the caller should most likely redefine the
     * object as empty to deallocate any underlying GPU memory before calling
     * this method (but not call glDelete*, since that would free up the name
     * within the driver itself).
     *
     * @param name The object name to free. Not-allocated names are silently
     *             ignored the same way they are in the OpenGL spec.
     */
    void free(GrGLuint name);

private:
    class SparseNameRange;
    class SparseNameTree;
    class ContiguousNameRange;

    const GrGLuint fFirstName;
    const GrGLuint fEndName;
    SkAutoTUnref<SparseNameRange> fAllocatedNames;
};

#endif
