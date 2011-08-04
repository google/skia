
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOSMenu_DEFINED
#define SkOSMenu_DEFINED

#include "SkEvent.h"
#include "SkTDArray.h"

class SkOSMenu {
public:
    explicit SkOSMenu(const char title[] = "");
    ~SkOSMenu();
    
    void reset();
    /**
     * Each of these (except action) has an associated value, which is stored in 
     * the event payload for the item.
     * Each type has a specific type for its value...
     *     Action : none
     *     List : int (selected index)
     *     Segmented : int (selected index)
     *     Slider : float
     *     Switch : bool
     *     TextField : string
     *     TriState : TriState
     *     Custom : custom object/value
     */
    enum TriState {
        kMixedState = -1,
        kOffState = 0,
        kOnState = 1
    };
    
    enum Type {
        kAction_Type,
        kList_Type,
        kSlider_Type,
        kSwitch_Type,
        kTriState_Type,
        kTextField_Type,
        kCustom_Type
    };
    
    class Item {
    public:
        //Auto increments a global to generate an unique ID for each new item
        //Thread safe
        Item(const char label[], SkOSMenu::Type type, const char slotName[], 
             SkEvent* evt, SkEventSinkID target);
        ~Item() { delete fEvent; }
        
        SkEvent* getEvent() const { return fEvent; }
        int getID() { return fID; }
        const char* getLabel() const { return fLabel.c_str(); }
        const char* getSlotName() const { return fSlotName.c_str(); }
        Type getType() const { return fType; }
        
        //Post event associated with the menu item to target, any changes to the
        //associated event must be made prior to calling this method.
        void postEvent() const {
            (new SkEvent(*(fEvent)))->setTargetID(fTarget)->post();
        }
        
        //Helper functions for predefined types
        void postEventWithBool(bool value) const; //For Switch
        void postEventWithScalar(SkScalar value) const; //For Slider
        void postEventWithInt(int value) const; //For List, TriState
        void postEventWithString(const char value[]) const; //For TextField

        
    private:
        int             fID;
        SkEvent*        fEvent;
        SkString        fLabel;
        SkString        fSlotName;
        SkEventSinkID   fTarget;
        Type            fType;
    };
    
    //The following functions append new items to the menu and returns their 
    //associated unique id, which can be used to by the client to refer to 
    //the menu item created and change its state. slotName specifies the string
    //identifier of any state/value to be returned in the item's SkEvent object
    //NOTE: evt must be dynamically allocated
    int appendItem(const char label[], Type type, const char slotName[], 
                   SkEvent* evt, SkEventSinkID target); 
    
    //Predefined items and helper functions:
    //Identifiers
    static const char* EventType;
    static const char* Delimiter;
    static const char* List_Items_Str;
    static const char* Slider_Min_Scalar;
    static const char* Slider_Max_Scalar;
    
    //Create predefined items with the given parameters. To be used with the 
    int appendAction(const char label[], SkEventSinkID target);
    int appendList(const char label[], const char slotName[], 
                   SkEventSinkID target, int defaultIndex, const char[] ...);
    int appendSlider(const char label[], const char slotName[], 
                     SkEventSinkID target, SkScalar min, SkScalar max, 
                     SkScalar defaultValue);
    int appendSwitch(const char label[], const char slotName[], 
                     SkEventSinkID target, bool defaultState = false);
    int appendTriState(const char label[], const char slotName[],
                       SkEventSinkID target, SkOSMenu::TriState defaultState = kOffState);
    int appendTextField(const char label[], const char slotName[],
                        SkEventSinkID target, const char placeholder[] = "");
    
    //Returns true if the event is of type SkOSMenu::EventType and retrieves 
    //value stored in the evt that corresponds to the slotName. Otherwise, 
    //returns false and leaves value unchanged
    static bool FindAction(const SkEvent* evt, const char label[]);
    static bool FindListIndex(const SkEvent* evt, const char slotName[], int* selected);
    static bool FindSliderValue(const SkEvent* evt, const char slotName[], SkScalar* value);
    static bool FindSwitchState(const SkEvent* evt, const char slotName[], bool* value);
    static bool FindTriState(const SkEvent* evt, const char slotName[], TriState* state);
    static bool FindText(const SkEvent* evt, const char slotName[], SkString* value);
    
    const char* getTitle() const { return fTitle.c_str(); }
    void setTitle (const char title[]) { fTitle.set(title); }
    // called by SkOSWindow when it receives an OS menu event
    int         countItems() const;
    const Item* getItem(int index) const;

private:
    SkString fTitle;
    SkTDArray<Item*> fItems;
    
    // illegal
    SkOSMenu(const SkOSMenu&);
    SkOSMenu& operator=(const SkOSMenu&);
};

#endif

