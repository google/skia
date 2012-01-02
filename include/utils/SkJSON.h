/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "SkTypes.h"

class SkString;

class SkJSON {
public:
    enum Type {
        kObject,
        kArray,
        kString,
        kInt,
        kFloat,
        kBool,
    };
    
    class Array;
    
    class Object {
    public:
        Object() : fHead(NULL), fTail(NULL) {}
        Object(const Object&);
        ~Object();
        
        void addObject(const char name[], Object* value);
        void addArray(const char name[], Array* value);
        void addString(const char name[], const char value[]);
        void addInt(const char name[], int32_t value);
        void addFloat(const char name[], float value);
        void addBool(const char name[], bool value);
        
        bool findObject(const char name[], Object** = NULL) const;
        bool findArray(const char name[], Array** = NULL) const;
        bool findString(const char name[], SkString* = NULL) const;
        bool findInt(const char name[], int32_t* = NULL) const;
        bool findFloat(const char name[], float* = NULL) const;
        bool findBool(const char name[], bool* = NULL) const;
        
        void dump() const;
        
    private:
        struct Slot;
        Slot* fHead;
        Slot* fTail;
        
        const Slot* findSlot(const char name[]) const;
        const Slot* findSlotAndType(const char name[], Type) const;
        Slot* addSlot(Slot*);
        void dumpLevel(int level) const;
        
        friend class Array;
    };
    
    class Array {
    public:
        // do I support strings, objects, arrays?
        Array(Type, int count);
        Array(const int32_t values[], int count);
        Array(const float values[], int count);
        Array(const bool values[], int count);
        Array(const Array&);
        ~Array();
        
        int count() const { return fCount; }
        Type type() const { return fType; }
        
        int32_t* ints() const { return fArray.fInts; }
        float* floats() const { return fArray.fFloats; }
        bool* bools() const { return fArray.fBools; }
        
    private:
        int fCount;
        Type fType;
        union {
            void*    fVoids;
            int32_t* fInts;
            float*   fFloats;
            bool*    fBools;
        } fArray;
        
        void init(Type, int count, const void* src);
        void dumpLevel(int level) const;
        
        friend class Object;
    };
};

#endif
