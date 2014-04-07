/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFakeRefObj_DEFINED
#define GrFakeRefObj_DEFINED

#include "SkTypes.h"
#include "gl/GrGLInterface.h"

////////////////////////////////////////////////////////////////////////////////
// This object is used to track the OpenGL objects. We don't use real
// reference counting (i.e., we don't free the objects when their ref count
// goes to 0) so that we can detect invalid memory accesses. The refs we
// are tracking in this class are actually OpenGL's references to the objects
// not "ours"
// Each object also gets a unique globally identifying ID
class GrFakeRefObj : SkNoncopyable {
public:
    GrFakeRefObj()
        : fRef(0)
        , fHighRefCount(0)
        , fMarkedForDeletion(false)
        , fDeleted(false) {

        // source for globally unique IDs - 0 is reserved!
        static int fNextID = 0;

        fID = ++fNextID;
    }
    virtual ~GrFakeRefObj() {};

    void ref() {
        fRef++;
        if (fHighRefCount < fRef) {
            fHighRefCount = fRef;
        }
    }
    void unref() {
        fRef--;
        GrAlwaysAssert(fRef >= 0);

        // often in OpenGL a given object may still be in use when the
        // delete call is made. In these cases the object is marked
        // for deletion and then freed when it is no longer in use
        if (0 == fRef && fMarkedForDeletion) {
            this->deleteAction();
        }
    }
    int getRefCount() const             { return fRef; }
    int getHighRefCount() const         { return fHighRefCount; }

    GrGLuint getID() const              { return fID; }

    void setMarkedForDeletion()         { fMarkedForDeletion = true; }
    bool getMarkedForDeletion() const   { return fMarkedForDeletion; }

    bool getDeleted() const             { return fDeleted; }

    // The deleteAction fires if the object has been marked for deletion but
    // couldn't be deleted earlier due to refs
    virtual void deleteAction() {
        this->setDeleted();
    }

protected:
private:
    int         fRef;               // ref count
    int         fHighRefCount;      // high water mark of the ref count
    GrGLuint    fID;                // globally unique ID
    bool        fMarkedForDeletion;
    // The deleted flag is only set when OpenGL thinks the object is deleted
    // It is obviously still allocated w/in this framework
    bool        fDeleted;

    // setDeleted should only ever appear in the deleteAction method!
    void setDeleted()                   { fDeleted = true; }
};

////////////////////////////////////////////////////////////////////////////////
// Each class derived from GrFakeRefObj should use this macro to add a
// factory creation entry point. This entry point is used by the GrGLDebug
// object to instantiate the various objects
// all globally unique IDs
#define GR_DEFINE_CREATOR(className)                        \
    public:                                                 \
    static GrFakeRefObj *create ## className() {            \
        return SkNEW(className);                            \
    }

#endif // GrFakeRefObj_DEFINED
