/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "SkTypes.h"

class SkStream;
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
    private:
        struct Slot;

    public:
        Object();
        Object(const Object&);
        ~Object();

        /**
         *  Create a new slot with the specified name and value. The name
         *  parameter is copied, but ownership of the Object parameter is
         *  transferred. The Object parameter may be null, but the name must
         *  not be null.
         */
        void addObject(const char name[], Object* value);

        /**
         *  Create a new slot with the specified name and value. The name
         *  parameter is copied, but ownership of the Array parameter is
         *  transferred. The Array parameter may be null, but the name must
         *  not be null.
         */
        void addArray(const char name[], Array* value);

        /**
         *  Create a new slot with the specified name and value. Both parameters
         *  are copied. The value parameter may be null, but the name must
         *  not be null.
         */
        void addString(const char name[], const char value[]);

        /**
         *  Create a new slot with the specified name and value. The name
         *  parameter is copied, and must not be null.
         */
        void addInt(const char name[], int32_t value);

        /**
         *  Create a new slot with the specified name and value. The name
         *  parameter is copied, and must not be null.
         */
        void addFloat(const char name[], float value);

        /**
         *  Create a new slot with the specified name and value. The name
         *  parameter is copied, and must not be null.
         */
        void addBool(const char name[], bool value);

        /**
         *  Return the number of slots/fields in this object. These can be
         *  iterated using Iter.
         */
        int count() const;

        /**
         *  Returns true if a slot matching the name and Type is found.
         */
        bool find(const char name[], Type) const;
        bool findObject(const char name[], Object** = NULL) const;
        bool findArray(const char name[], Array** = NULL) const;
        bool findString(const char name[], SkString* = NULL) const;
        bool findInt(const char name[], int32_t* = NULL) const;
        bool findFloat(const char name[], float* = NULL) const;
        bool findBool(const char name[], bool* = NULL) const;

        /**
         *  Finds the first slot matching the name and Type and removes it.
         *  Returns true if found, false if not.
         */
        bool remove(const char name[], Type);

        void toDebugf() const;

        /**
         *  Iterator class which returns all of the fields/slots in an Object,
         *  in the order that they were added.
         */
        class Iter {
        public:
            Iter(const Object&);

            /**
             *  Returns true when there are no more entries in the iterator.
             *  In this case, no other methods should be called.
             */
            bool done() const;

            /**
             *  Moves the iterator to the next element. Should only be called
             *  if done() returns false.
             */
            void next();

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false.
             */
            Type type() const;

            /**
             *  Returns the name of the current element. Should only be called
             *  if done() returns false.
             */
            const char* name() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kObject.
             */
            Object* objectValue() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kArray.
             */
            Array* arrayValue() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kString.
             */
            const char* stringValue() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kInt.
             */
            int32_t intValue() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kFloat.
             */
            float floatValue() const;

            /**
             *  Returns the type of the current element. Should only be called
             *  if done() returns false and type() returns kBool.
             */
            bool boolValue() const;

        private:
            Slot* fSlot;
        };

    private:
        Slot* fHead;
        Slot* fTail;

        const Slot* findSlot(const char name[], Type) const;
        Slot* addSlot(Slot*);
        void dumpLevel(int level) const;

        friend class Array;
    };

    class Array {
    public:
        /**
         *  Creates an array with the specified Type and element count. All
         *  entries are initialized to NULL/0/false.
         */
        Array(Type, int count);

        /**
         *  Creates an array of ints, initialized by copying the specified
         *  values.
         */
        Array(const int32_t values[], int count);

        /**
         *  Creates an array of floats, initialized by copying the specified
         *  values.
         */
        Array(const float values[], int count);

        /**
         *  Creates an array of bools, initialized by copying the specified
         *  values.
         */
        Array(const bool values[], int count);

        Array(const Array&);
        ~Array();

        int count() const { return fCount; }
        Type type() const { return fType; }

        /**
         *  Replace the element at the specified index with the specified
         *  Object (which may be null). Ownership of the Object is transferred.
         *  Should only be called if the Array's type is kObject.
         */
        void setObject(int index, Object*);

        /**
         *  Replace the element at the specified index with the specified
         *  Array (which may be null). Ownership of the Array is transferred.
         *  Should only be called if the Array's type is kArray.
         */
        void setArray(int index, Array*);

        /**
         *  Replace the element at the specified index with a copy of the
         *  specified string (which may be null). Should only be called if the
         *  Array's type is kString.
         */
        void setString(int index, const char str[]);

        Object* const* objects() const {
            SkASSERT(kObject == fType);
            return fArray.fObjects;
        }
        Array* const* arrays() const {
            SkASSERT(kObject == fType);
            return fArray.fArrays;
        }
        const char* const* strings() const {
            SkASSERT(kString == fType);
            return fArray.fStrings;
        }
        int32_t* ints() const {
            SkASSERT(kInt == fType);
            return fArray.fInts;
        }
        float* floats() const {
            SkASSERT(kFloat == fType);
            return fArray.fFloats;
        }
        bool* bools() const {
            SkASSERT(kBool == fType);
            return fArray.fBools;
        }

    private:
        int fCount;
        Type fType;
        union {
            void*    fVoids;
            Object** fObjects;
            Array**  fArrays;
            char**   fStrings;
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
