/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONObject.h"
#include "SkString.h"

struct SkJSONObject::Slot {
    Slot(const char name[], SkJSONObject::Type type) {
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

    SkJSONObject::Type type() const {
        return fName ? (SkJSONObject::Type)fName[0]
                     : SkJSONObject::kInvalid;
    }

    Slot*   fNext;
    char*   fName;    // fName[0] is the type
    union {
        SkJSONObject*   fObject;
        char*           fString;
        int32_t         fInt;
        float           fFloat;
        bool            fBool;
        intptr_t        fIntPtr;    // for generic getter
    } fValue;
};

SkJSONObject::Slot::~Slot() {
    switch (this->type()) {
        case SkJSONObject::kObject:
            delete fValue.fObject;
            break;
        case SkJSONObject::kString:
            delete[] fValue.fString;
            break;
        default:
            break;
    }
    delete[] fName;
}

///////////////////////////////////////////////////////////////////////////////

SkJSONObject::~SkJSONObject() {
    Slot* slot = fHead;
    while (slot) {
        Slot* next = slot->fNext;
        delete slot;
        slot = next;
    }
}

SkJSONObject::Slot* SkJSONObject::addSlot(Slot* slot) {
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

void SkJSONObject::addString(const char name[], const char value[]) {
    Slot* slot = addSlot(new Slot(name, kString));
    size_t len = strlen(value);
    char* str = new char[len + 1];
    memcpy(str, value, len + 1);
    slot->fValue.fString = str;
}

void SkJSONObject::addInt(const char name[], int32_t value) {
    Slot* slot = addSlot(new Slot(name, kInt));
    fTail->fValue.fInt = value;
}

void SkJSONObject::addFloat(const char name[], float value) {
    Slot* slot = addSlot(new Slot(name, kFloat));
    fTail->fValue.fFloat = value;
}

void SkJSONObject::addBool(const char name[], bool value) {
    Slot* slot = addSlot(new Slot(name, kBool));
    fTail->fValue.fBool = value;
}

void SkJSONObject::addObject(const char name[], SkJSONObject* value) {
    Slot* slot = addSlot(new Slot(name, kObject));
    fTail->fValue.fObject = value;
}

///////////////////////////////////////////////////////////////////////////////

const SkJSONObject::Slot* SkJSONObject::findSlot(const char name[]) const {
    for (const Slot* slot = fHead; slot; slot = slot->fNext) {
        if (!strcmp(slot->name(), name)) {
            return slot;
        }
    }
    return NULL;
}

const SkJSONObject::Slot* SkJSONObject::findSlotAndType(const char name[],
                                                        Type t) const {
    const Slot* slot = this->findSlot(name);
    if (slot && (slot->type() != t)) {
        slot = NULL;
    }
    return slot;
}

SkJSONObject::Type SkJSONObject::find(const char name[], intptr_t* value) const {
    const Slot* slot = this->findSlot(name);
    if (slot) {
        if (value) {
            *value = slot->fValue.fIntPtr;
        }
        return slot->type();
    }
    return kInvalid;
}

bool SkJSONObject::findString(const char name[], SkString* value) const {
    const Slot* slot = this->findSlotAndType(name, kString);
    if (slot) {
        if (value) {
            value->set(slot->fValue.fString);
        }
        return true;
    }
    return false;
}

bool SkJSONObject::findInt(const char name[], int32_t* value) const {
    const Slot* slot = this->findSlotAndType(name, kInt);
    if (slot) {
        if (value) {
            *value = slot->fValue.fInt;
        }
        return true;
    }
    return false;
}

bool SkJSONObject::findFloat(const char name[], float* value) const {
    const Slot* slot = this->findSlotAndType(name, kFloat);
    if (slot) {
        if (value) {
            *value = slot->fValue.fFloat;
        }
        return true;
    }
    return false;
}

bool SkJSONObject::findBool(const char name[], bool* value) const {
    const Slot* slot = this->findSlotAndType(name, kBool);
    if (slot) {
        if (value) {
            *value = slot->fValue.fBool;
        }
        return true;
    }
    return false;
}

bool SkJSONObject::findObject(const char name[], SkJSONObject** value) const {
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

void SkJSONObject::dump() const {
    SkDebugf("{\n");
    this->dumpLevel(0);
    SkDebugf("}\n");
}

void SkJSONObject::dumpLevel(int level) const {
    for (Slot* slot = fHead; slot; slot = slot->fNext) {
        Type t = slot->type();
        if (kInvalid == t) {
            continue;
        }

        tabForLevel(level + 1);
        SkDebugf("\"%s\" : ", slot->name());
        switch (slot->type()) {
            case kObject: {
                if (slot->fValue.fObject) {
                    SkDebugf("{\n");
                    slot->fValue.fObject->dumpLevel(level + 1);
                    tabForLevel(level + 1);
                    SkDebugf("}");
                } else {
                    SkDebugf("null");
                }
            } break;
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


