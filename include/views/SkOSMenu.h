
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
    enum Type {
        kAction_Type,
        kList_Type,
        kSlider_Type,
        kSwitch_Type,
        kTriState_Type,
        kTextField_Type,
        kCustom_Type
    };

    enum TriState {
        kMixedState = -1,
        kOffState = 0,
        kOnState = 1
    };

    class Item {
    public:
        /**
         * Auto increments a global to generate an unique ID for each new item
         * Note: Thread safe
         */
        Item(const char label[], SkOSMenu::Type type, const char slotName[],
             SkEvent* evt);
        ~Item() { delete fEvent; }

        SkEvent*    getEvent() const { return fEvent; }
        int         getID() const { return fID; }
        const char* getLabel() const { return fLabel.c_str(); }
        const char* getSlotName() const { return fSlotName.c_str(); }
        Type        getType() const { return fType; }
        void        setKeyEquivalent(SkUnichar key) { fKey = key; }
        SkUnichar   getKeyEquivalent() const { return fKey; }

        /**
         * Helper functions for predefined types
         */
        void setBool(bool value) const;             //For Switch
        void setScalar(SkScalar value) const;       //For Slider
        void setInt(int value) const;               //For List
        void setTriState(TriState value) const;     //For Tristate
        void setString(const char value[]) const;   //For TextField

        /**
         * Post event associated with the menu item to target, any changes to
         * the associated event must be made prior to calling this method
         */
        void postEvent() const { (new SkEvent(*(fEvent)))->post(); }

    private:
        int             fID;
        SkEvent*        fEvent;
        SkString        fLabel;
        SkString        fSlotName;
        Type            fType;
        SkUnichar       fKey;
    };

    void        reset();
    const char* getTitle() const { return fTitle.c_str(); }
    void        setTitle (const char title[]) { fTitle.set(title); }
    int         getCount() const { return fItems.count(); }
    const Item* getItemByID(int itemID) const;
    void        getItems(const Item* items[]) const;

    /**
     * Assign key to the menu item with itemID, will do nothing if there's no
     * item with the id given
     */
    void        assignKeyEquivalentToItem(int itemID, SkUnichar key);
    /**
     * Call this in a SkView's onHandleChar to trigger any menu items with the
     * given key equivalent. If such an item is found, the method will return
     * true and its corresponding event will be triggered (default behavior
     * defined for switches(toggling), tristates(cycle), and lists(cycle),
     * for anything else, the event attached is posted without state changes)
     * If no menu item can be matched with the key, false will be returned
     */
    bool        handleKeyEquivalent(SkUnichar key);

    /**
     * The following functions append new items to the menu and returns their
     * associated unique id, which can be used to by the client to refer to
     * the menu item created and change its state. slotName specifies the string
     * identifier of any state/value to be returned in the item's SkEvent object
     * NOTE: evt must be dynamically allocated
     */
    int appendItem(const char label[], Type type, const char slotName[],
                   SkEvent* evt);

    /**
     * Create predefined items with the given parameters. To be used with the
     * other helper functions below to retrive/update state information.
     * Note: the helper functions below assume that slotName is UNIQUE for all
     * menu items of the same type since it's used to identify the event
     */
    int appendAction(const char label[], SkEventSinkID target);
    int appendList(const char label[], const char slotName[],
                   SkEventSinkID target, int defaultIndex, const char* ...);
    int appendSlider(const char label[], const char slotName[],
                     SkEventSinkID target, SkScalar min, SkScalar max,
                     SkScalar defaultValue);
    int appendSwitch(const char label[], const char slotName[],
                     SkEventSinkID target, bool defaultState = false);
    int appendTriState(const char label[], const char slotName[],
                       SkEventSinkID target, TriState defaultState = kOffState);
    int appendTextField(const char label[], const char slotName[],
                        SkEventSinkID target, const char placeholder[] = "");


    /**
     * Helper functions to retrieve information other than the stored value for
     * some predefined types
     */
    static bool FindListItemCount(const SkEvent& evt, int* count);
    /**
     * Ensure that the items array can store n SkStrings where n is the count
     * extracted using FindListItemCount
     */
    static bool FindListItems(const SkEvent& evt, SkString items[]);
    static bool FindSliderMin(const SkEvent& evt, SkScalar* min);
    static bool FindSliderMax(const SkEvent& evt, SkScalar* max);

    /**
     * Returns true if an action with the given label is found, false otherwise
     */
    static bool FindAction(const SkEvent& evt, const char label[]);
    /**
     * The following helper functions will return true if evt is generated from
     * a predefined item type and retrieve the corresponding state information.
     * They will return false and leave value unchanged if there's a type
     * mismatch or slotName is incorrect
     */
    static bool FindListIndex(const SkEvent& evt, const char slotName[], int* value);
    static bool FindSliderValue(const SkEvent& evt, const char slotName[], SkScalar* value);
    static bool FindSwitchState(const SkEvent& evt, const char slotName[], bool* value);
    static bool FindTriState(const SkEvent& evt, const char slotName[], TriState* value);
    static bool FindText(const SkEvent& evt, const char slotName[], SkString* value);

private:
    SkString fTitle;
    SkTDArray<Item*> fItems;

    // illegal
    SkOSMenu(const SkOSMenu&);
    SkOSMenu& operator=(const SkOSMenu&);
};

#endif
