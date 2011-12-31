/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSONObject_DEFINED
#define SkJSONObject_DEFINED

#include "SkTypes.h"

class SkString;

class SkJSONObject {
public:
    SkJSONObject() : fHead(NULL), fTail(NULL) {}
    ~SkJSONObject();
    
    enum Type {
        kInvalid,
        
        kObject,
        kString,
        kInt,
        kFloat,
        kBool,
    };
    
    void addString(const char name[], const char value[]);
    void addInt(const char name[], int32_t value);
    void addFloat(const char name[], float value);
    void addBool(const char name[], bool value);
    /**
     *  Add the value object to this object, taking over
     * ownership of the value object.
     */
    void addObject(const char name[], SkJSONObject* value);
    
    Type find(const char name[], intptr_t* = NULL) const;
    bool findString(const char name[], SkString* = NULL) const;
    bool findInt(const char name[], int32_t* = NULL) const;
    bool findFloat(const char name[], float* = NULL) const;
    bool findBool(const char name[], bool* = NULL) const;
    bool findObject(const char name[], SkJSONObject** = NULL) const;
    
    void dump() const;
    
private:
    struct Slot;
    Slot* fHead;
    Slot* fTail;

    const Slot* findSlot(const char name[]) const;
    const Slot* findSlotAndType(const char name[], Type) const;
    Slot* addSlot(Slot*);
    void dumpLevel(int level) const;
};

#endif
