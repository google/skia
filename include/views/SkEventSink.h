
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEventSink_DEFINED
#define SkEventSink_DEFINED

#include "SkRefCnt.h"
#include "SkEvent.h"

/** \class SkEventSink

    SkEventSink is the base class for all objects that receive SkEvents.
*/
class SkEventSink : public SkRefCnt {
public:


             SkEventSink();
    virtual ~SkEventSink();

    /**
     *  Returns this eventsink's unique ID. Use this to post SkEvents to
     *  this eventsink.
     */
    SkEventSinkID getSinkID() const { return fID; }

    /**
     *  Call this to pass an event to this object for processing. Returns true if the
     *  event was handled.
     */
    bool doEvent(const SkEvent&);

    /** Returns true if the sink (or one of its subclasses) understands the event as a query.
        If so, the sink may modify the event to communicate its "answer".
    */
    bool doQuery(SkEvent* query);

    /**
     *  Returns the matching eventsink, or null if not found
     */
    static SkEventSink* FindSink(SkEventSinkID);

protected:
    /** Override this to handle events in your subclass. Be sure to call the inherited version
        for events that you don't handle.
    */
    virtual bool onEvent(const SkEvent&);
    virtual bool onQuery(SkEvent*);

private:
    SkEventSinkID   fID;

    // for our private link-list
    SkEventSink*    fNextSink;

    typedef SkRefCnt INHERITED;
};

#endif
