/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEvent_DEFINED
#define SkEvent_DEFINED

#include "SkDOM.h"
#include "SkMetaData.h"
#include "SkString.h"

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
    /**
     *  Function pointer that takes an event, returns true if it "handled" it.
     */
    typedef bool (*Proc)(const SkEvent& evt);

    SkEvent();
    explicit SkEvent(const SkString& type, SkEventSinkID = 0);
    explicit SkEvent(const char type[], SkEventSinkID = 0);
    SkEvent(const SkEvent& src);
    ~SkEvent();

    /** Copy the event's type into the specified SkString parameter */
    void getType(SkString* str) const;

    /** Returns true if the event's type matches exactly the specified type (case sensitive) */
    bool isType(const SkString& str) const;

    /** Returns true if the event's type matches exactly the specified type (case sensitive) */
    bool isType(const char type[], size_t len = 0) const;

    /**
     *  Set the event's type to the specified string.
     */
    void setType(const SkString&);

    /**
     *  Set the event's type to the specified string.
     */
    void setType(const char type[], size_t len = 0);

    /**
     *  Return the target ID, or 0 if there is none.
     *
     *  When an event is dispatched from the event queue, it is either sent to
     *  the eventsink matching the targetID (if not 0), or the target proc is
     *  called (if not NULL).
     */
    SkEventSinkID getTargetID() const { return fTargetID; }

    /**
     *  Set the target ID for this event. 0 means none. Calling this will
     *  automatically clear the targetProc to null.
     *
     *  When an event is dispatched from the event queue, it is either sent to
     *  the eventsink matching the targetID (if not 0), or the target proc is
     *  called (if not NULL).
     */
    SkEvent* setTargetID(SkEventSinkID targetID) {
        fTargetProc = NULL;
        fTargetID = targetID;
        return this;
    }

    /**
     *  Return the target proc, or NULL if it has none.
     *
     *  When an event is dispatched from the event queue, it is either sent to
     *  the eventsink matching the targetID (if not 0), or the target proc is
     *  called (if not NULL).
     */
    Proc getTargetProc() const { return fTargetProc; }

    /**
     *  Set the target ID for this event. NULL means none. Calling this will
     *  automatically clear the targetID to 0.
     *
     *  When an event is dispatched from the event queue, it is either sent to
     *  the eventsink matching the targetID (if not 0), or the target proc is
     *  called (if not NULL).
     */
    SkEvent* setTargetProc(Proc proc) {
        fTargetID = 0;
        fTargetProc = proc;
        return this;
    }

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
    bool findS32(const char name[], int32_t* value = NULL) const { return fMeta.findS32(name, value); }
    /** Return true if the event contains the named SkScalar field, and return the field
        in value (if value is non-null). If there is no matching named field, return false
        and ignore the value parameter.
    */
    bool findScalar(const char name[], SkScalar* value = NULL) const { return fMeta.findScalar(name, value); }
    /** Return true if the event contains the named SkScalar field, and return the fields
        in value[] (if value is non-null), and return the number of SkScalars in count (if count is non-null).
        If there is no matching named field, return false and ignore the value and count parameters.
    */
    const SkScalar* findScalars(const char name[], int* count, SkScalar values[] = NULL) const { return fMeta.findScalars(name, count, values); }
    /** Return the value of the named string field, or if no matching named field exists, return null.
    */
    const char* findString(const char name[]) const { return fMeta.findString(name); }
    /** Return true if the event contains the named pointer field, and return the field
        in value (if value is non-null). If there is no matching named field, return false
        and ignore the value parameter.
    */
    bool findPtr(const char name[], void** value) const { return fMeta.findPtr(name, value); }
    bool findBool(const char name[], bool* value) const { return fMeta.findBool(name, value); }
    const void* findData(const char name[], size_t* byteCount = NULL) const {
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
    SkScalar* setScalars(const char name[], int count, const SkScalar values[] = NULL) { return fMeta.setScalars(name, count, values); }
    /** Add/replace the named string field to the event. In XML use the subelement <data name=... string=... */
    void setString(const char name[], const SkString& value) { fMeta.setString(name, value.c_str()); }
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

    /** Call this to initialize the event from the specified XML node */
    void inflate(const SkDOM&, const SkDOM::Node*);

    SkDEBUGCODE(void dump(const char title[] = NULL);)

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Post to the event queue using the event's targetID or target-proc.
     *
     *  The event must be dynamically allocated, as ownership is transferred to
     *  the event queue. It cannot be allocated on the stack or in a global.
     */
    void post() {
        return this->postDelay(0);
    }

    /**
     *  Post to the event queue using the event's targetID or target-proc and
     *  the specifed millisecond delay.
     *
     *  The event must be dynamically allocated, as ownership is transferred to
     *  the event queue. It cannot be allocated on the stack or in a global.
     */
    void postDelay(SkMSec delay);

    /**
     *  Post to the event queue using the event's targetID or target-proc.
     *  The event will be delivered no sooner than the specified millisecond
     *  time, as measured by GetMSecsSinceStartup().
     *
     *  The event must be dynamically allocated, as ownership is transferred to
     *  the event queue. It cannot be allocated on the stack or in a global.
     */
    void postTime(SkMSec time);

    /**
     *  Returns ~zero the first time it's called, then returns the number of
     *  milliseconds since the first call. Behavior is undefined if the program
     *  runs more than ~25 days.
     */
    static SkMSec GetMSecsSinceStartup();

    ///////////////////////////////////////////////
    /** Porting layer must call these functions **/
    ///////////////////////////////////////////////

    /** Global initialization function for the SkEvent system. Should be called exactly
        once before any other event method is called, and should be called after the
        call to SkGraphics::Init().
    */
    static void Init();
    /** Global cleanup function for the SkEvent system. Should be called exactly once after
        all event methods have been called.
    */
    static void Term();

    /** Call this to process one event from the queue. If it returns true, there are more events
        to process.
    */
    static bool ProcessEvent();
    /** Call this whenever the requested timer has expired (requested by a call to SetQueueTimer).
        It will post any delayed events whose time as "expired" onto the event queue.
        It may also call SignalQueueTimer() and SignalNonEmptyQueue().
    */
    static void ServiceQueueTimer();

    /** Return the number of queued events. note that this value may be obsolete
        upon return, since another thread may have called ProcessEvent() or
        Post() after the count was made.
     */
    static int CountEventsOnQueue();

    ////////////////////////////////////////////////////
    /** Porting layer must implement these functions **/
    ////////////////////////////////////////////////////

    /** Called whenever an SkEvent is posted to an empty queue, so that the OS
        can be told to later call Dequeue().
    */
    static void SignalNonEmptyQueue();
    /** Called whenever the delay until the next delayed event changes. If zero is
        passed, then there are no more queued delay events.
    */
    static void SignalQueueTimer(SkMSec delay);

#if defined(SK_BUILD_FOR_WIN)
    static bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

private:
    SkMetaData      fMeta;
    mutable char*   fType;  // may be characters with low bit set to know that it is not a pointer
    uint32_t        f32;

    // 'there can be only one' (non-zero) between target-id and target-proc
    SkEventSinkID   fTargetID;
    Proc            fTargetProc;

    // these are for our implementation of the event queue
    SkMSec          fTime;
    SkEvent*        fNextEvent; // either in the delay or normal event queue

    void initialize(const char* type, size_t typeLen, SkEventSinkID);

    static bool Enqueue(SkEvent* evt);
    static SkMSec EnqueueTime(SkEvent* evt, SkMSec time);
    static SkEvent* Dequeue();
    static bool     QHasEvents();
};

#endif
