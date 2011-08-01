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

int SkOSMenu::countItems() const {
	return fItems.count();
}

const SkOSMenu::Item* SkOSMenu::getItem(int index) const{
	return fItems[index];
}

////////////////////////////////////////////////////////////////////////////////

SkOSMenu::Item::Item(const char label[], SkOSMenu::Type type, 
                     const char slotName[], SkEvent* evt, SkEventSinkID target) {
    fLabel.set(label);
    fSlotName.set(slotName);
    fType = type;
    fTarget = target;
    fEvent = evt;
    fID = sk_atomic_inc(&gOSMenuCmd);
}

void SkOSMenu::Item::postEventWithBool(bool value) const {
    SkASSERT(SkOSMenu::kSwitch_Type == fType);
    fEvent->setBool(fSlotName.c_str(), value);
    this->postEvent();
}

void SkOSMenu::Item::postEventWithScalar(SkScalar value) const {
    SkASSERT(SkOSMenu::kSlider_Type == fType);
    fEvent->setScalar(fSlotName.c_str(), value);
    this->postEvent();
}

void SkOSMenu::Item::postEventWithInt(int value) const {
    SkASSERT(SkOSMenu::kList_Type == fType || SkOSMenu::kTriState_Type == fType);
    fEvent->setS32(fSlotName.c_str(), value);
    this->postEvent();
}

void SkOSMenu::Item::postEventWithString(const char value[]) const {
    SkASSERT(SkOSMenu::kTextField_Type == fType);
    fEvent->setString(fSlotName.c_str(), value);
    this->postEvent();
}

////////////////////////////////////////////////////////////////////////////////

const char* SkOSMenu::EventType = "SkOSMenuEventType";
const char* SkOSMenu::Delimiter = "|";
const char* SkOSMenu::Slider_Min_Scalar = "SkOSMenuSlider_Min";
const char* SkOSMenu::Slider_Max_Scalar = "SkOSMenuSlider_Max";
const char* SkOSMenu::List_Items_Str = "SkOSMenuList_Items";

int SkOSMenu::appendItem(const char label[], Type type, const char slotName[], 
                         SkEvent* evt, SkEventSinkID target) {
    SkOSMenu::Item* item = new Item(label, type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendAction(const char label[], SkEventSinkID target) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kAction_Type, "", evt, target);
    //Store label in event so it can be used to identify the action later
    evt->setString(label, "");
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendList(const char label[], const char slotName[], 
                         SkEventSinkID target, int index, const char option[], ...) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    va_list args;
    if (option) {
        SkString str(option);
        va_start(args, option);
        for (const char* arg = va_arg(args, const char*); arg != NULL; arg = va_arg(args, const char*)) {
            str += SkOSMenu::Delimiter;
            str += arg;
        }
        va_end(args);
        evt->setString(SkOSMenu::List_Items_Str, str);
        evt->setS32(slotName, index);
    }
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kList_Type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendSlider(const char label[], const char slotName[], 
                           SkEventSinkID target, SkScalar min, SkScalar max, 
                           SkScalar defaultValue) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    evt->setScalar(SkOSMenu::Slider_Min_Scalar, min);
    evt->setScalar(SkOSMenu::Slider_Max_Scalar, max);
    evt->setScalar(slotName, defaultValue);
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kSlider_Type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendSwitch(const char label[], const char slotName[], 
                           SkEventSinkID target, bool defaultState) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    evt->setBool(slotName, defaultState);
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kSwitch_Type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendTriState(const char label[], const char slotName[],
                             SkEventSinkID target, SkOSMenu::TriState defaultState) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    evt->setS32(slotName, defaultState);
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kTriState_Type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}

int SkOSMenu::appendTextField(const char label[], const char slotName[], 
                              SkEventSinkID target, const char placeholder[]) {
    SkEvent* evt = new SkEvent(SkOSMenu::EventType);
    evt->setString(slotName, placeholder);
    SkOSMenu::Item* item = new Item(label, SkOSMenu::kTextField_Type, slotName, evt, target);
    fItems.append(1, &item);
    return item->getID();
}


bool SkOSMenu::FindAction(const SkEvent* evt, const char label[]) {
    return evt->isType(SkOSMenu::EventType) && evt->findString(label);
}

bool SkOSMenu::FindListIndex(const SkEvent* evt, const char slotName[], int* selected) {
    return evt->isType(SkOSMenu::EventType) && evt->findS32(slotName, selected); 
}

bool SkOSMenu::FindSliderValue(const SkEvent* evt, const char slotName[], SkScalar* value) {
    return evt->isType(SkOSMenu::EventType) && evt->findScalar(slotName, value);
}

bool SkOSMenu::FindSwitchState(const SkEvent* evt, const char slotName[], bool* value) {
    return evt->isType(SkOSMenu::EventType) && evt->findBool(slotName, value);
}

bool SkOSMenu::FindTriState(const SkEvent* evt, const char slotName[], SkOSMenu::TriState* state) {
    return evt->isType(SkOSMenu::EventType) && evt->findS32(slotName, (int*)state);
}

bool SkOSMenu::FindText(const SkEvent* evt, const char slotName[], SkString* value) {
    if (evt->isType(SkOSMenu::EventType)) {
        const char* text = evt->findString(slotName);
        if (!text || !*text)
            return false;
        else {
            value->set(text);
            return true;
        }
    }
    return false;
}
