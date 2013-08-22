/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"
#include "SkString.h"

#ifdef SK_DEBUG
//    #define TRACE_SKJSON_LEAKS
#endif

#ifdef TRACE_SKJSON_LEAKS
    static int gStringCount;
    static int gSlotCount;
    static int gObjectCount;
    static int gArrayCount;
    #define LEAK_CODE(code) code
#else
    #define LEAK_CODE(code)
#endif

///////////////////////////////////////////////////////////////////////////////

static char* alloc_string(size_t len) {
    LEAK_CODE(SkDebugf(" string[%d]\n", gStringCount++);)
    char* str = (char*)sk_malloc_throw(len + 1);
    str[len] = 0;
    return str;
}

static char* dup_string(const char src[]) {
    if (NULL == src) {
        return NULL;
    }
    size_t len = strlen(src);
    char* dst = alloc_string(len);
    memcpy(dst, src, len);
    return dst;
}

static void free_string(char* str) {
    if (str) {
        sk_free(str);
        LEAK_CODE(SkASSERT(gStringCount > 0); SkDebugf("~string[%d]\n", --gStringCount);)
    }
}

///////////////////////////////////////////////////////////////////////////////

struct SkJSON::Object::Slot {
    Slot(const char name[], Type type) {
        LEAK_CODE(SkDebugf(" slot[%d]\n", gSlotCount++);)
        SkASSERT(name);

        fNext = NULL;

        size_t len = strlen(name);
        // extra 1 for str[0] which stores the type
        char* str = alloc_string(1 + len);
        str[0] = (char)type;
        // str[1] skips the type, len+1 includes the terminating 0 byte.
        memcpy(&str[1], name, len + 1);
        fName = str;

        // fValue is uninitialized
    }
    ~Slot();

    Type type() const { return (Type)fName[0]; }
    const char* name() const { return &fName[1]; }

    Slot*   fNext;
    char*   fName;    // fName[0] is the type, &fName[1] is the "name"
    union {
        Object* fObject;
        Array*  fArray;
        char*   fString;
        int32_t fInt;
        float   fFloat;
        bool    fBool;
    } fValue;
};

SkJSON::Object::Slot::~Slot() {
    free_string(fName);
    switch (this->type()) {
        case kObject:
            delete fValue.fObject;
            break;
        case kArray:
            delete fValue.fArray;
            break;
        case kString:
            free_string(fValue.fString);
            break;
        default:
            break;
    }
    LEAK_CODE(SkASSERT(gSlotCount > 0); SkDebugf("~slot[%d]\n", --gSlotCount);)
}

///////////////////////////////////////////////////////////////////////////////

SkJSON::Object::Iter::Iter(const Object& obj) : fSlot(obj.fHead) {}

bool SkJSON::Object::Iter::done() const {
    return NULL == fSlot;
}

void SkJSON::Object::Iter::next() {
    SkASSERT(fSlot);
    fSlot = fSlot->fNext;
}

SkJSON::Type SkJSON::Object::Iter::type() const {
    SkASSERT(fSlot);
    return fSlot->type();
}

const char* SkJSON::Object::Iter::name() const {
    SkASSERT(fSlot);
    return fSlot->name();
}

SkJSON::Object* SkJSON::Object::Iter::objectValue() const {
    SkASSERT(fSlot);
    SkASSERT(kObject == fSlot->type());
    return fSlot->fValue.fObject;
}

SkJSON::Array* SkJSON::Object::Iter::arrayValue() const {
    SkASSERT(fSlot);
    SkASSERT(kArray == fSlot->type());
    return fSlot->fValue.fArray;
}

const char* SkJSON::Object::Iter::stringValue() const {
    SkASSERT(fSlot);
    SkASSERT(kString == fSlot->type());
    return fSlot->fValue.fString;
}

int32_t SkJSON::Object::Iter::intValue() const {
    SkASSERT(fSlot);
    SkASSERT(kInt == fSlot->type());
    return fSlot->fValue.fInt;
}

float SkJSON::Object::Iter::floatValue() const {
    SkASSERT(fSlot);
    SkASSERT(kFloat == fSlot->type());
    return fSlot->fValue.fFloat;
}

bool SkJSON::Object::Iter::boolValue() const {
    SkASSERT(fSlot);
    SkASSERT(kBool == fSlot->type());
    return fSlot->fValue.fBool;
}

///////////////////////////////////////////////////////////////////////////////

SkJSON::Object::Object() : fHead(NULL), fTail(NULL) {
    LEAK_CODE(SkDebugf(" object[%d]\n", gObjectCount++);)
}

SkJSON::Object::Object(const Object& other) : fHead(NULL), fTail(NULL) {
    LEAK_CODE(SkDebugf(" object[%d]\n", gObjectCount++);)

    Iter iter(other);
    while (!iter.done()) {
        switch (iter.type()) {
            case kObject:
                this->addObject(iter.name(), new Object(*iter.objectValue()));
                break;
            case kArray:
                this->addArray(iter.name(), new Array(*iter.arrayValue()));
                break;
            case kString:
                this->addString(iter.name(), dup_string(iter.stringValue()));
                break;
            case kInt:
                this->addInt(iter.name(), iter.intValue());
                break;
            case kFloat:
                this->addFloat(iter.name(), iter.floatValue());
                break;
            case kBool:
                this->addBool(iter.name(), iter.boolValue());
                break;
        }
        iter.next();
    }
}

SkJSON::Object::~Object() {
    Slot* slot = fHead;
    while (slot) {
        Slot* next = slot->fNext;
        delete slot;
        slot = next;
    }
    LEAK_CODE(SkASSERT(gObjectCount > 0); SkDebugf("~object[%d]\n", --gObjectCount);)
}

int SkJSON::Object::count() const {
    int n = 0;
    for (const Slot* slot = fHead; slot; slot = slot->fNext) {
        n += 1;
    }
    return n;
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
    return slot;
}

void SkJSON::Object::addObject(const char name[], SkJSON::Object* value) {
    this->addSlot(new Slot(name, kObject))->fValue.fObject = value;
}

void SkJSON::Object::addArray(const char name[], SkJSON::Array* value) {
    this->addSlot(new Slot(name, kArray))->fValue.fArray = value;
}

void SkJSON::Object::addString(const char name[], const char value[]) {
    this->addSlot(new Slot(name, kString))->fValue.fString = dup_string(value);
}

void SkJSON::Object::addInt(const char name[], int32_t value) {
    this->addSlot(new Slot(name, kInt))->fValue.fInt = value;
}

void SkJSON::Object::addFloat(const char name[], float value) {
    this->addSlot(new Slot(name, kFloat))->fValue.fFloat = value;
}

void SkJSON::Object::addBool(const char name[], bool value) {
    this->addSlot(new Slot(name, kBool))->fValue.fBool = value;
}

///////////////////////////////////////////////////////////////////////////////

const SkJSON::Object::Slot* SkJSON::Object::findSlot(const char name[],
                                                     Type t) const {
    for (const Slot* slot = fHead; slot; slot = slot->fNext) {
        if (t == slot->type() && !strcmp(slot->name(), name)) {
            return slot;
        }
    }
    return NULL;
}

bool SkJSON::Object::find(const char name[], Type t) const {
    return this->findSlot(name, t) != NULL;
}

bool SkJSON::Object::findObject(const char name[], SkJSON::Object** value) const {
    const Slot* slot = this->findSlot(name, kObject);
    if (slot) {
        if (value) {
            *value = slot->fValue.fObject;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findArray(const char name[], SkJSON::Array** value) const {
    const Slot* slot = this->findSlot(name, kArray);
    if (slot) {
        if (value) {
            *value = slot->fValue.fArray;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findString(const char name[], SkString* value) const {
    const Slot* slot = this->findSlot(name, kString);
    if (slot) {
        if (value) {
            value->set(slot->fValue.fString);
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findInt(const char name[], int32_t* value) const {
    const Slot* slot = this->findSlot(name, kInt);
    if (slot) {
        if (value) {
            *value = slot->fValue.fInt;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findFloat(const char name[], float* value) const {
    const Slot* slot = this->findSlot(name, kFloat);
    if (slot) {
        if (value) {
            *value = slot->fValue.fFloat;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::findBool(const char name[], bool* value) const {
    const Slot* slot = this->findSlot(name, kBool);
    if (slot) {
        if (value) {
            *value = slot->fValue.fBool;
        }
        return true;
    }
    return false;
}

bool SkJSON::Object::remove(const char name[], Type t) {
    SkDEBUGCODE(int count = this->count();)
    Slot* prev = NULL;
    Slot* slot = fHead;
    while (slot) {
        Slot* next = slot->fNext;
        if (t == slot->type() && !strcmp(slot->name(), name)) {
            if (prev) {
                SkASSERT(fHead != slot);
                prev->fNext = next;
            } else {
                SkASSERT(fHead == slot);
                fHead = next;
            }
            if (fTail == slot) {
                fTail = prev;
            }
            delete slot;
            SkASSERT(count - 1 == this->count());
            return true;
        }
        prev = slot;
        slot = next;
    }
    SkASSERT(count == this->count());
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void tabForLevel(int level) {
    for (int i = 0; i < level; ++i) {
        SkDebugf("    ");
    }
}

void SkJSON::Object::toDebugf() const {
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
                SkDEBUGFAIL("how did I get here");
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
        case kObject: {
            SkDebugf("\n");
            for (int i = 0; i <= last; ++i) {
                Object* obj = fArray.fObjects[i];
                tabForLevel(level + 1);
                if (obj) {
                    SkDebugf("{\n");
                    obj->dumpLevel(level + 1);
                    tabForLevel(level + 1);
                    SkDebugf(i < last ? "}," : "}");
                } else {
                    SkDebugf(i < last ? "null," : "null");
                }
                SkDebugf("\n");
            }
        } break;
        case kArray: {
            SkDebugf("\n");
            for (int i = 0; i <= last; ++i) {
                Array* array = fArray.fArrays[i];
                tabForLevel(level + 1);
                if (array) {
                    SkDebugf("[");
                    array->dumpLevel(level + 1);
                    tabForLevel(level + 1);
                    SkDebugf(i < last ? "]," : "]");
                } else {
                    SkDebugf(i < last ? "null," : "null");
                }
                SkDebugf("\n");
            }
        } break;
        case kString: {
            for (int i = 0; i < last; ++i) {
                const char* str = fArray.fStrings[i];
                SkDebugf(str ? " \"%s\"," : " null,", str);
            }
            const char* str = fArray.fStrings[last];
            SkDebugf(str ? " \"%s\" " : " null ", str);
        } break;
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
            SkDEBUGFAIL("unsupported array type");
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

typedef void* (*DupProc)(const void*);

static void* dup_object(const void* src) {
    return SkNEW_ARGS(SkJSON::Object, (*(SkJSON::Object*)src));
}

static void* dup_array(const void* src) {
    return SkNEW_ARGS(SkJSON::Array, (*(SkJSON::Array*)src));
}

static const DupProc gDupProcs[] = {
    dup_object,             // Object
    dup_array,              // Array
    (DupProc)dup_string,    // String
    NULL,                   // int
    NULL,                   // float
    NULL,                   // bool
};

void SkJSON::Array::init(Type type, int count, const void* src) {
    LEAK_CODE(SkDebugf(" array[%d]\n", gArrayCount++);)

    SkASSERT((unsigned)type < SK_ARRAY_COUNT(gBytesPerType));

    if (count < 0) {
        count = 0;
    }
    size_t size = count * gBytesPerType[type];

    fCount = count;
    fType = type;
    fArray.fVoids = sk_malloc_throw(size);
    if (src) {
        DupProc proc = gDupProcs[fType];
        if (!proc) {
            memcpy(fArray.fVoids, src, size);
        } else {
            void** srcPtr = (void**)src;
            void** dstPtr = (void**)fArray.fVoids;
            for (int i = 0; i < fCount; ++i) {
                dstPtr[i] = proc(srcPtr[i]);
            }
        }
    } else {
        sk_bzero(fArray.fVoids, size);
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

SkJSON::Array::Array(const Array& other) {
    this->init(other.type(), other.count(), other.fArray.fVoids);
}

typedef void (*FreeProc)(void*);

static void free_object(void* obj) {
    delete (SkJSON::Object*)obj;
}

static void free_array(void* array) {
    delete (SkJSON::Array*)array;
}

static const FreeProc gFreeProcs[] = {
    free_object,            // Object
    free_array,             // Array
    (FreeProc)free_string,  // String
    NULL,                   // int
    NULL,                   // float
    NULL,                   // bool
};

SkJSON::Array::~Array() {
    FreeProc proc = gFreeProcs[fType];
    if (proc) {
        void** ptr = (void**)fArray.fVoids;
        for (int i = 0; i < fCount; ++i) {
            proc(ptr[i]);
        }
    }
    sk_free(fArray.fVoids);

    LEAK_CODE(SkASSERT(gArrayCount > 0); SkDebugf("~array[%d]\n", --gArrayCount);)
}

void SkJSON::Array::setObject(int index, Object* object) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    Object*& prev = fArray.fObjects[index];
    if (prev != object) {
        delete prev;
        prev = object;
    }
}

void SkJSON::Array::setArray(int index, Array* array) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    Array*& prev = fArray.fArrays[index];
    if (prev != array) {
        delete prev;
        prev = array;
    }
}

void SkJSON::Array::setString(int index, const char str[]) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    char*& prev = fArray.fStrings[index];
    if (prev != str) {
        free_string(prev);
        prev = dup_string(str);
    }
}
