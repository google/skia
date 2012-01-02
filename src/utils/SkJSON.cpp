/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"
#include "SkString.h"

struct SkJSON::Object::Slot {
    Slot(const char name[], Type type) {
        fNext = NULL;
        
        size_t len = strlen(name);
        char* str = new char[len + 2];
        str[0] = (char)type;
        memcpy(str + 1, name, len + 1);
        fName = str;
        
        // fValue is uninitialized
    }
    ~Slot();
    
    const char* name() const {
        return fName ? &fName[1] : "";
    }
    
    Type type() const {
        return (Type)fName[0];
    }
    
    Slot*   fNext;
    char*   fName;    // fName[0] is the type
    union {
        SkJSON::Object* fObject;
        SkJSON::Array*  fArray;
        char*           fString;
        int32_t         fInt;
        float           fFloat;
        bool            fBool;
        intptr_t        fIntPtr;    // for generic getter
    } fValue;
};

SkJSON::Object::Slot::~Slot() {
    switch (this->type()) {
        case kObject:
            delete fValue.fObject;
            break;
        case kArray:
        case kString:
            delete[] fValue.fString;
            break;
        default:
            break;
    }
    delete[] fName;
}

///////////////////////////////////////////////////////////////////////////////

SkJSON::Object::~Object() {
    Slot* slot = fHead;
    while (slot) {
        Slot* next = slot->fNext;
        delete slot;
        slot = next;
    }
}

SkJSON::Object::Slot* SkJSON::Object::addSlot(Slot* slot) {
    SkASSERT(NULL == slot->fNext);
    if (NULL == fHead) {
        SkASSERT(NULL == fTail);
        fHead = fTail = slot;
    } else {
        SkASSERT(fTail);
        SkASSERT(NULL == fTail->fNext);
        fTail->fNext = slot;
        fTail = slot;
    }
}

void SkJSON::Object::addObject(const char name[], SkJSON::Object* value) {
    Slot* slot = addSlot(new Slot(name, kObject));
    fTail->fValue.fObject = value;
}

void SkJSON::Object::addArray(const char name[], SkJSON::Array* value) {
    Slot* slot = addSlot(new Slot(name, kArray));
    fTail->fValue.fArray = value;
}

void SkJSON::Object::addString(const char name[], const char value[]) {
    Slot* slot = addSlot(new Slot(name, kString));
    size_t len = strlen(value);
    char* str = new char[len + 1];
    memcpy(str, value, len + 1);
    slot->fValue.fString = str;
}

void SkJSON::Object::addInt(const char name[], int32_t value) {
    Slot* slot = addSlot(new Slot(name, kInt));
    fTail->fValue.fInt = value;
}

void SkJSON::Object::addFloat(const char name[], float value) {
    Slot* slot = addSlot(new Slot(name, kFloat));
    fTail->fValue.fFloat = value;
}

void SkJSON::Object::addBool(const char name[], bool value) {
    Slot* slot = addSlot(new Slot(name, kBool));
    fTail->fValue.fBool = value;
}

///////////////////////////////////////////////////////////////////////////////

const SkJSON::Object::Slot* SkJSON::Object::findSlot(const char name[]) const {
    for (const Slot* slot = fHead; slot; slot = slot->fNext) {
        if (!strcmp(slot->name(), name)) {
            return slot;
        }
    }
    return NULL;
}

const SkJSON::Object::Slot* SkJSON::Object::findSlotAndType(const char name[],
                                                        Type t) const {
    const Slot* slot = this->findSlot(name);
    if (slot && (slot->type() != t)) {
        slot = NULL;
    }
    return slot;
}

bool SkJSON::Object::findString(const char name[], SkString* value) const {
    const Slot* slot = this->findSlotAndType(name, kString);
    if (slot) {
        if (value) {
            value->set(slot->fValue.fString);
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findInt(const char name[], int32_t* value) const {
    const Slot* slot = this->findSlotAndType(name, kInt);
    if (slot) {
        if (value) {
            *value = slot->fValue.fInt;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findFloat(const char name[], float* value) const {
    const Slot* slot = this->findSlotAndType(name, kFloat);
    if (slot) {
        if (value) {
            *value = slot->fValue.fFloat;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findBool(const char name[], bool* value) const {
    const Slot* slot = this->findSlotAndType(name, kBool);
    if (slot) {
        if (value) {
            *value = slot->fValue.fBool;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findObject(const char name[], SkJSON::Object** value) const {
    const Slot* slot = this->findSlotAndType(name, kObject);
    if (slot) {
        if (value) {
            *value = slot->fValue.fObject;
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void tabForLevel(int level) {
    for (int i = 0; i < level; ++i) {
        SkDebugf("    ");
    }
}

void SkJSON::Object::dump() const {
    SkDebugf("{\n");
    this->dumpLevel(0);
    SkDebugf("}\n");
}

void SkJSON::Object::dumpLevel(int level) const {
    for (Slot* slot = fHead; slot; slot = slot->fNext) {
        Type t = slot->type();        
        tabForLevel(level + 1);
        SkDebugf("\"%s\" : ", slot->name());
        switch (slot->type()) {
            case kObject:
                if (slot->fValue.fObject) {
                    SkDebugf("{\n");
                    slot->fValue.fObject->dumpLevel(level + 1);
                    tabForLevel(level + 1);
                    SkDebugf("}");
                } else {
                    SkDebugf("null");
                }
                break;
            case kArray:
                if (slot->fValue.fArray) {
                    SkDebugf("[");
                    slot->fValue.fArray->dumpLevel(level + 1);
                    SkDebugf("]");
                } else {
                    SkDebugf("null");
                }
                break;
            case kString:
                SkDebugf("\"%s\"", slot->fValue.fString);
                break;
            case kInt:
                SkDebugf("%d", slot->fValue.fInt);
                break;
            case kFloat:
                SkDebugf("%g", slot->fValue.fFloat);
                break;
            case kBool:
                SkDebugf("%s", slot->fValue.fBool ? "true" : "false");
                break;
            default:
                SkASSERT(!"how did I get here");
                break;
        }
        if (slot->fNext) {
            SkDebugf(",");
        }
        SkDebugf("\n");
    }
}

void SkJSON::Array::dumpLevel(int level) const {
    if (0 == fCount) {
        return;
    }
    int last = fCount - 1;

    switch (this->type()) {
        case kInt: {
            for (int i = 0; i < last; ++i) {
                SkDebugf(" %d,", fArray.fInts[i]);
            }
            SkDebugf(" %d ", fArray.fInts[last]);
        } break;
        case kFloat: {
            for (int i = 0; i < last; ++i) {
                SkDebugf(" %g,", fArray.fFloats[i]);
            }
            SkDebugf(" %g ", fArray.fFloats[last]);
        } break;
        case kBool: {
            for (int i = 0; i < last; ++i) {
                SkDebugf(" %s,", fArray.fBools[i] ? "true" : "false");
            }
            SkDebugf(" %s ", fArray.fInts[last] ? "true" : "false");
        } break;
        default:
            SkASSERT(!"unsupported array type");
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const uint8_t gBytesPerType[] = {
    sizeof(SkJSON::Object*),
    sizeof(SkJSON::Array*),
    sizeof(char*),
    sizeof(int32_t),
    sizeof(float),
    sizeof(bool)
};

void SkJSON::Array::init(Type type, int count, const void* src) {
    if (count < 0) {
        count = 0;
    }
    size_t size = count * gBytesPerType[type];

    fCount = count;
    fType = type;
    fArray.fVoids = sk_malloc_throw(size);
    if (src) {
        memcpy(fArray.fVoids, src, size);
    }
}

SkJSON::Array::Array(Type type, int count) {
    this->init(type, count, NULL);
}

SkJSON::Array::Array(const int32_t values[], int count) {
    this->init(kInt, count, values);
}
    
SkJSON::Array::Array(const float values[], int count) {
    this->init(kFloat, count, values);
}

SkJSON::Array::Array(const bool values[], int count) {
    this->init(kBool, count, values);
}

SkJSON::Array::Array(const Array& src) {
    this->init(src.type(), src.count(), src.fArray.fVoids);
}

SkJSON::Array::~Array() {
    sk_free(fArray.fVoids);
}

