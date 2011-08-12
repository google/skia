/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <stdarg.h>
#include "SkOSMenu.h"
#include "SkThread.h"

static int gOSMenuCmd = 7000;

SkOSMenu::SkOSMenu(const char title[]) {
	fTitle.set(title);
}

SkOSMenu::~SkOSMenu() {
    this->reset();
}

void SkOSMenu::reset() {
    fItems.deleteAll();
    fTitle.reset();
}

const SkOSMenu::Item* SkOSMenu::getItemByID(int itemID) const {
    for (int i = 0; i < fItems.count(); ++i) {
        if (itemID == fItems[i]->getID())
            return fItems[i];
    }
    return NULL;
}

void SkOSMenu::getItems(const SkOSMenu::Item* items[]) const {
    if (NULL != items) {
        for (int i = 0; i < fItems.count(); ++i) {
            items[i] = fItems[i];
        }
    }
}

void SkOSMenu::assignKeyEquivalentToItem(int itemID, SkUnichar key) {
    for (int i = 0; i < fItems.count(); ++i) {
        if (itemID == fItems[i]->getID())
            fItems[i]->setKeyEquivalent(key);
    }
}

bool SkOSMenu::handleKeyEquivalent(SkUnichar key) {    
    int value = 0, size = 0;
    bool state;
    SkOSMenu::TriState tristate;
    for (int i = 0; i < fItems.count(); ++i) {
        Item* item = fItems[i];
        if (item->getKeyEquivalent()== key) {
            SkString list;
            switch (item->getType()) {
                case kList_Type:
                    SkOSMenu::FindListItemCount(*item->getEvent(), &size);
                    SkOSMenu::FindListIndex(*item->getEvent(), item->getSlotName(), &value);
                    value = (value + 1) % size;
                    item->setInt(value);
                    break;
                case kSwitch_Type:
                    SkOSMenu::FindSwitchState(*item->getEvent(), item->getSlotName(), &state);
                    item->setBool(!state);
                    break;
                case kTriState_Type:
                    SkOSMenu::FindTriState(*item->getEvent(), item->getSlotName(), &tristate);
                    if (kOnState == tristate)
                        tristate = kMixedState;
                    else
                        tristate = (SkOSMenu::TriState)((int)tristate + 1);
                    item->setTriState(tristate);
                    break;
                case kAction_Type:
                case kCustom_Type:
                case kSlider_Type:
                case kTextField_Type:
                default:
                    break;
            }
            item->postEvent();
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

SkOSMenu::Item::Item(const char label[], SkOSMenu::Type type, 
                     const char slotName[], SkEvent* evt) {
    fLabel.set(label);
    fSlotName.set(slotName);
    fType = type;
    fEvent = evt;
    fKey = 0;
    fID = sk_atomic_inc(&gOSMenuCmd);
}

void SkOSMenu::Item::setBool(bool value) const {
    SkASSERT(SkOSMenu::kSwitch_Type == fType);
    fEvent->setBool(fSlotName.c_str(), value);
}

void SkOSMenu::Item::setScalar(SkScalar value) const {
    SkASSERT(SkOSMenu::kSlider_Type == fType);
    fEvent->setScalar(fSlotName.c_str(), value);
}

void SkOSMenu::Item::setInt(int value) const {
    SkASSERT(SkOSMenu::kList_Type == fType);
    fEvent->setS32(fSlotName.c_str(), value);
}

void SkOSMenu::Item::setTriState(TriState value) const {
    SkASSERT(SkOSMenu::kTriState_Type == fType);
    fEvent->setS32(fSlotName.c_str(), value);
}

void SkOSMenu::Item::setString(const char value[]) const {
    SkASSERT(SkOSMenu::kTextField_Type == fType);
    fEvent->setString(fSlotName.c_str(), value);
}

////////////////////////////////////////////////////////////////////////////////

static const char* gMenuEventType = "SkOSMenuEventType";
static const char* gSlider_Min_Scalar = "SkOSMenuSlider_Min";
static const char* gSlider_Max_Scalar = "SkOSMenuSlider_Max";
static const char* gDelimiter = "|";
static const char* gList_Items_Str = "SkOSMenuList_Items";
static const char* gList_ItemCount_S32 = "SkOSMenuList_ItemCount";

int SkOSMenu::appendItem(const char label[], Type type, const char slotName[], 
                         SkEvent* evt) {
    SkOSMenu::Item* item = new Item(label, type, slotName, evt);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendAction(const char label[], SkEventSinkID target) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    //Store label in event so it can be used to identify the action later
    evt->setString(label, label);
    return appendItem(label, SkOSMenu::kAction_Type, "", evt);
}

int SkOSMenu::appendList(const char label[], const char slotName[], 
                         SkEventSinkID target, int index, const char option[], ...) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    va_list args;
    if (option) {
        SkString str(option);
        va_start(args, option);
        int count = 1;
        for (const char* arg = va_arg(args, const char*); arg != NULL; arg = va_arg(args, const char*)) {
            str += gDelimiter;
            str += arg;
            ++count;
        }
        va_end(args);
        evt->setString(gList_Items_Str, str);
        evt->setS32(gList_ItemCount_S32, count);
        evt->setS32(slotName, index);
    }
    return appendItem(label, SkOSMenu::kList_Type, slotName, evt);
}

int SkOSMenu::appendSlider(const char label[], const char slotName[], 
                           SkEventSinkID target, SkScalar min, SkScalar max, 
                           SkScalar defaultValue) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    evt->setScalar(gSlider_Min_Scalar, min);
    evt->setScalar(gSlider_Max_Scalar, max);
    evt->setScalar(slotName, defaultValue);
    return appendItem(label, SkOSMenu::kSlider_Type, slotName, evt);
}

int SkOSMenu::appendSwitch(const char label[], const char slotName[], 
                           SkEventSinkID target, bool defaultState) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    evt->setBool(slotName, defaultState);
    return appendItem(label, SkOSMenu::kSwitch_Type, slotName, evt);
}

int SkOSMenu::appendTriState(const char label[], const char slotName[],
                             SkEventSinkID target, SkOSMenu::TriState defaultState) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    evt->setS32(slotName, defaultState);
    return appendItem(label, SkOSMenu::kTriState_Type, slotName, evt);
}

int SkOSMenu::appendTextField(const char label[], const char slotName[], 
                              SkEventSinkID target, const char placeholder[]) {
    SkEvent* evt = new SkEvent(gMenuEventType, target);
    evt->setString(slotName, placeholder);
    return appendItem(label, SkOSMenu::kTextField_Type, slotName, evt);
}

bool SkOSMenu::FindListItemCount(const SkEvent& evt, int* count) {
    return evt.isType(gMenuEventType) && evt.findS32(gList_ItemCount_S32, count);
}

bool SkOSMenu::FindListItems(const SkEvent& evt, SkString items[]) {
    if (evt.isType(gMenuEventType) && NULL != items) {
        const char* text = evt.findString(gList_Items_Str);
        if (text != NULL) {
            SkString temp(text);
            char* token = strtok((char*)temp.c_str(), gDelimiter);
            int index = 0;
            while (token != NULL) {
                items[index].set(token, strlen(token));
                token = strtok (NULL, gDelimiter);
                ++index;
            }
        }
        return true;
    }
    return false;
}

bool SkOSMenu::FindSliderMin(const SkEvent& evt, SkScalar* min) {
    return evt.isType(gMenuEventType) && evt.findScalar(gSlider_Min_Scalar, min);
}

bool SkOSMenu::FindSliderMax(const SkEvent& evt, SkScalar* max) {
    return evt.isType(gMenuEventType) && evt.findScalar(gSlider_Max_Scalar, max);
}

bool SkOSMenu::FindAction(const SkEvent& evt, const char label[]) {
    return evt.isType(gMenuEventType) && evt.findString(label);
}

bool SkOSMenu::FindListIndex(const SkEvent& evt, const char slotName[], int* value) {
    return evt.isType(gMenuEventType) && evt.findS32(slotName, value); 
}

bool SkOSMenu::FindSliderValue(const SkEvent& evt, const char slotName[], SkScalar* value) {
    return evt.isType(gMenuEventType) && evt.findScalar(slotName, value);
}

bool SkOSMenu::FindSwitchState(const SkEvent& evt, const char slotName[], bool* value) {
    return evt.isType(gMenuEventType) && evt.findBool(slotName, value);
}

bool SkOSMenu::FindTriState(const SkEvent& evt, const char slotName[], SkOSMenu::TriState* value) {
    return evt.isType(gMenuEventType) && evt.findS32(slotName, (int*)value);
}

bool SkOSMenu::FindText(const SkEvent& evt, const char slotName[], SkString* value) {
    if (evt.isType(gMenuEventType)) {
        const char* text = evt.findString(slotName);
        if (!text || !*text)
            return false;
        else {
            value->set(text);
            return true;
        }
    }
    return false;
}
