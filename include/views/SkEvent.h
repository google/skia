/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEvent_DEFINED
#define SkEvent_DEFINED

#include "SkMetaData.h"

/** Unique 32bit id used to identify an instance of SkEventSink. When events are
    posted, they are posted to a specific sinkID. When it is time to dispatch the
    event, the sinkID is used to find the specific SkEventSink object. If it is found,
    its doEvent() method is called with the event.
*/
typedef uint32_t SkEventSinkID;

/**
 *  \class SkEvent
 *
 *  When an event is dispatched from the event queue, it is either sent to
 *  the eventsink matching the target ID (if not 0), or the target proc is
 *  called (if not NULL).
 */
class SkEvent {
public:
    SkEvent();
    explicit SkEvent(const char type[]);
    SkEvent(const SkEvent& src);
    ~SkEvent();

    /** Returns true if the event's type matches exactly the specified type (case sensitive) */
    bool isType(const char type[]) const;

    /**
     *  Set the event's type to the specified string.
     */
    void setType(const char type[]);

    /**
     *  Return the event's unnamed 32bit field. Default value is 0
     */
    uint32_t getFast32() const { return f32; }

    /**
     *  Set the event's unnamed 32bit field.
     */
    void setFast32(uint32_t x) { f32 = x; }

    /** Return true if the event contains the named 32bit field, and return the field
        in value (if value is non-null). If there is no matching named field, return false
        and ignore the value parameter.
    */
    bool findS32(const char name[], int32_t* value = nullptr) const {
        return fMeta.findS32(name, value);
    }
    /** Return true if the event contains the named SkScalar field, and return the field
        in value (if value is non-null). If there is no matching named field, return false
        and ignore the value parameter.
    */
    bool findScalar(const char name[], SkScalar* value = nullptr) const {
        return fMeta.findScalar(name, value);
    }
    /** Return true if the event contains the named SkScalar field, and return the fields
        in value[] (if value is non-null), and return the number of SkScalars in count (if count is non-null).
        If there is no matching named field, return false and ignore the value and count parameters.
    */
    const SkScalar* findScalars(const char name[], int* count, SkScalar values[] = nullptr) const {
        return fMeta.findScalars(name, count, values);
    }
    /** Return the value of the named string field, or if no matching named field exists, return null.
    */
    const char* findString(const char name[]) const { return fMeta.findString(name); }
    /** Return true if the event contains the named pointer field, and return the field
        in value (if value is non-null). If there is no matching named field, return false
        and ignore the value parameter.
    */
    bool findPtr(const char name[], void** value) const { return fMeta.findPtr(name, value); }
    bool findBool(const char name[], bool* value) const { return fMeta.findBool(name, value); }
    const void* findData(const char name[], size_t* byteCount = nullptr) const {
        return fMeta.findData(name, byteCount);
    }

    /** Returns true if ethe event contains the named 32bit field, and if it equals the specified value */
    bool hasS32(const char name[], int32_t value) const { return fMeta.hasS32(name, value); }
    /** Returns true if ethe event contains the named SkScalar field, and if it equals the specified value */
    bool hasScalar(const char name[], SkScalar value) const { return fMeta.hasScalar(name, value); }
    /** Returns true if ethe event contains the named string field, and if it equals (using strcmp) the specified value */
    bool hasString(const char name[], const char value[]) const { return fMeta.hasString(name, value); }
    /** Returns true if ethe event contains the named pointer field, and if it equals the specified value */
    bool hasPtr(const char name[], void* value) const { return fMeta.hasPtr(name, value); }
    bool hasBool(const char name[], bool value) const { return fMeta.hasBool(name, value); }
    bool hasData(const char name[], const void* data, size_t byteCount) const {
        return fMeta.hasData(name, data, byteCount);
    }

    /** Add/replace the named 32bit field to the event. In XML use the subelement <data name=... s32=... /> */
    void setS32(const char name[], int32_t value) { fMeta.setS32(name, value); }
    /** Add/replace the named SkScalar field to the event. In XML use the subelement <data name=... scalar=... /> */
    void setScalar(const char name[], SkScalar value) { fMeta.setScalar(name, value); }
    /** Add/replace the named SkScalar[] field to the event. */
    SkScalar* setScalars(const char name[], int count, const SkScalar values[] = nullptr) {
        return fMeta.setScalars(name, count, values);
    }
    /** Add/replace the named string field to the event. In XML use the subelement <data name=... string=... */
    void setString(const char name[], const char value[]) { fMeta.setString(name, value); }
    /** Add/replace the named pointer field to the event. There is no XML equivalent for this call */
    void setPtr(const char name[], void* value) { fMeta.setPtr(name, value); }
    void setBool(const char name[], bool value) { fMeta.setBool(name, value); }
    void setData(const char name[], const void* data, size_t byteCount) {
        fMeta.setData(name, data, byteCount);
    }

    /** Return the underlying metadata object */
    SkMetaData& getMetaData() { return fMeta; }
    /** Return the underlying metadata object */
    const SkMetaData& getMetaData() const { return fMeta; }

    ///////////////////////////////////////////////////////////////////////////

private:
    SkMetaData      fMeta;
    char*           fType;
    uint32_t        f32;

    void initialize(const char* type);
};

#endif
