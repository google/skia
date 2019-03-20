/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleCode_DEFINED
#define SampleCode_DEFINED

#include "Registry.h"
#include "SkColor.h"
#include "SkMacros.h"
#include "SkMetaData.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkString.h"

class AnimTimer;
class SkCanvas;
class Sample;

using SampleFactory = Sample* (*)();
using SampleRegistry = sk_tools::Registry<SampleFactory>;

#define DEF_SAMPLE(code) \
    static Sample*          SK_MACRO_APPEND_LINE(F_)() { code } \
    static SampleRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

///////////////////////////////////////////////////////////////////////////////

class Sample : public SkRefCnt {
public:
    Sample()
        : fBGColor(SK_ColorWHITE)
        , fWidth(0), fHeight(0)
        , fHaveCalledOnceBeforeDraw(false)
    {}

    SkScalar    width() const { return fWidth; }
    SkScalar    height() const { return fHeight; }
    void        setSize(SkScalar width, SkScalar height);
    void        setSize(const SkPoint& size) { this->setSize(size.fX, size.fY); }
    void        setWidth(SkScalar width) { this->setSize(width, fHeight); }
    void        setHeight(SkScalar height) { this->setSize(fWidth, height); }

    /** Call this to have the view draw into the specified canvas. */
    virtual void draw(SkCanvas* canvas);

    //  Click handling
    class Click {
    public:
        Click(Sample* target);
        virtual ~Click();

        enum State {
            kDown_State,
            kMoved_State,
            kUp_State
        };
        enum ModifierKeys {
            kShift_ModifierKey    = 1 << 0,
            kControl_ModifierKey  = 1 << 1,
            kOption_ModifierKey   = 1 << 2,   // same as ALT
            kCommand_ModifierKey  = 1 << 3,
        };
        SkPoint     fOrig, fPrev, fCurr;
        SkIPoint    fIOrig, fIPrev, fICurr;
        State       fState;
        unsigned    fModifierKeys;

        SkMetaData  fMeta;
    private:
        sk_sp<Sample> fTarget;

        friend class Sample;
    };
    Click* findClickHandler(SkScalar x, SkScalar y, unsigned modifierKeys);
    static void DoClickDown(Click*, int x, int y, unsigned modi);
    static void DoClickMoved(Click*, int x, int y, unsigned modi);
    static void DoClickUp(Click*, int x, int y, unsigned modi);

    void setBGColor(SkColor color) { fBGColor = color; }
    bool animate(const AnimTimer& timer) { return this->onAnimate(timer); }

    class Event {
    public:
        Event();
        explicit Event(const char type[]);
        Event(const Event& src);
        ~Event();

        /** Returns true if the event's type matches exactly the specified type (case sensitive) */
        bool isType(const char type[]) const;

        /** Return the event's unnamed 32bit field. Default value is 0 */
        uint32_t getFast32() const { return f32; }

        /** Set the event's unnamed 32bit field. */
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
            in value[] (if value is non-null), and return the number of SkScalars in count
            (if count is non-null). If there is no matching named field, return false
            and ignore the value and count parameters.
        */
        const SkScalar* findScalars(const char name[], int* count, SkScalar values[]=nullptr) const{
            return fMeta.findScalars(name, count, values);
        }
        /** Return the value of the named string field, or nullptr. */
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

        /** Add/replace the named 32bit field to the event. */
        void setS32(const char name[], int32_t value) { fMeta.setS32(name, value); }
        /** Add/replace the named SkScalar field to the event. */
        void setScalar(const char name[], SkScalar value) { fMeta.setScalar(name, value); }
        /** Add/replace the named SkScalar[] field to the event. */
        SkScalar* setScalars(const char name[], int count, const SkScalar values[] = nullptr) {
            return fMeta.setScalars(name, count, values);
        }
        /** Add/replace the named string field to the event. */
        void setString(const char name[], const char value[]) { fMeta.setString(name, value); }
        /** Add/replace the named pointer field to the event. */
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
        SkString        fType;
        uint32_t        f32;
    };

    /** Pass an event to this object for processing. Returns true if the event was handled. */
    bool doEvent(const Event&);

    /** Returns true if the sink (or one of its subclasses) understands the event as a query.
        If so, the sink may modify the event to communicate its "answer".
    */
    bool doQuery(Event* query);

    static const char* kCharEvtName;
    static const char* kTitleEvtName;
    static bool CharQ(const Event&, SkUnichar* outUni);
    static bool TitleQ(const Event&);
    static void TitleR(Event*, const char title[]);
    static bool RequestTitle(Sample* view, SkString* title);

protected:
    /** Override to handle events in your subclass.
     *  Overriders must call the super class for unhandled events.
     */
    virtual bool onEvent(const Event&);
    virtual bool onQuery(Event*);

    /** Override to be notified of size changes. Overriders must call the super class. */
    virtual void onSizeChange();

    /** Override this if you might handle the click */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi);

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*);

    virtual void onDrawBackground(SkCanvas*);
    virtual void onDrawContent(SkCanvas*) = 0;
    virtual bool onAnimate(const AnimTimer&) { return false; }
    virtual void onOnceBeforeDraw() {}

private:
    SkColor fBGColor;
    SkScalar fWidth, fHeight;
    bool fHaveCalledOnceBeforeDraw;
};

#endif
