
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayApply.h"
#include "SkAnimateActive.h"
#include "SkAnimateMaker.h"
#include "SkAnimateSet.h"
#include "SkAnimatorScript.h"
#include "SkDisplayType.h"
#include "SkDrawGroup.h"
#include "SkParse.h"
#include "SkScript.h"
#include "SkSystemEventTypes.h"
#ifdef SK_DEBUG
#include "SkTime.h"
#endif
#include <ctype.h>

enum SkApply_Properties {
    SK_PROPERTY(animator),
    SK_PROPERTY(step),
    SK_PROPERTY(steps),
    SK_PROPERTY(time)
};

#if SK_USE_CONDENSED_INFO == 0

// if no attibutes, enclosed displayable is both scope & target
// only if both scope & target are specified, or if target and enclosed displayable, are scope and target different
const SkMemberInfo SkApply::fInfo[] = {
    SK_MEMBER_PROPERTY(animator, Animate),
    SK_MEMBER(begin, MSec),
    SK_MEMBER(dontDraw, Boolean),
    SK_MEMBER(dynamicScope, String),
    SK_MEMBER(interval, MSec),  // recommended redraw interval
    SK_MEMBER(mode, ApplyMode),
#if 0
    SK_MEMBER(pickup, Boolean),
#endif
    SK_MEMBER(restore, Boolean),
    SK_MEMBER(scope, Drawable), // thing that scopes animation (unnamed enclosed displayable goes here)
    SK_MEMBER_PROPERTY(step, Int),
    SK_MEMBER_PROPERTY(steps, Int),
    SK_MEMBER_PROPERTY(time, MSec),
    SK_MEMBER(transition, ApplyTransition)
};

#endif

DEFINE_GET_MEMBER(SkApply);

SkApply::SkApply() : begin(0), dontDraw(false), interval((SkMSec) -1), mode((Mode) -1), /*pickup(false), */
    restore(false), scope(NULL), steps(-1), transition((Transition) -1), fActive(NULL), /*fCurrentScope(NULL),*/
    fLastTime(0), fAppended(false), fContainsScope(false), fDeleteScope(false), fEmbedded(false),
    fEnabled(false), fEnabling(false) {
}

SkApply::~SkApply() {
    for (SkDrawable** curPtr = fScopes.begin(); curPtr < fScopes.end(); curPtr++)
        delete *curPtr;
    if (fDeleteScope)
        delete scope;
    // !!! caller must call maker.removeActive(fActive)
    delete fActive;
}

void SkApply::activate(SkAnimateMaker& maker) {
    if (fActive != NULL) {
        if (fActive->fDrawIndex == 0 && fActive->fDrawMax == 0)
            return; // if only one use, nothing more to do
        if (restore == false)
            return; // all share same state, regardless of instance number
        bool save = fActive->initializeSave();
        fActive->fixInterpolator(save);
    } else {
        fActive = new SkActive(*this, maker);
        fActive->init();
        maker.appendActive(fActive);
        if (restore) {
            fActive->initializeSave();
            int animators = fAnimators.count();
            for (int index = 0; index < animators; index++)
                fActive->saveInterpolatorValues(index);
        }
    }
}

void SkApply::append(SkApply* apply) {
    if (fActive == NULL)
        return;
    int oldCount = fActive->fAnimators.count();
    fActive->append(apply);
    if (restore) {
        fActive->appendSave(oldCount);
        int newCount = fActive->fAnimators.count();
        for (int index = oldCount; index < newCount; index++)
            fActive->saveInterpolatorValues(index);
    }
}

void SkApply::applyValues(int animatorIndex, SkOperand* values, int count,
     SkDisplayTypes valuesType, SkMSec time)
{
    SkAnimateBase* animator = fActive->fAnimators[animatorIndex];
    const SkMemberInfo * info = animator->fFieldInfo;
    SkASSERT(animator);
    SkASSERT(info != NULL);
    SkDisplayTypes type = (SkDisplayTypes) info->fType;
    SkDisplayable* target = getTarget(animator);
    if (animator->hasExecute() || type == SkType_MemberFunction || type == SkType_MemberProperty) {
        SkDisplayable* executor = animator->hasExecute() ? animator : target;
        if (type != SkType_MemberProperty) {
            SkTDArray<SkScriptValue> typedValues;
            for (int index = 0; index < count; index++) {
                SkScriptValue temp;
                temp.fType = valuesType;
                temp.fOperand = values[index];
                *typedValues.append() = temp;
            }
            executor->executeFunction(target, info->functionIndex(), typedValues, info->getType(), NULL);
        } else {
            SkScriptValue scriptValue;
            scriptValue.fOperand = values[0];
            scriptValue.fType = info->getType();
            target->setProperty(info->propertyIndex(), scriptValue);
        }
    } else {
        SkTypedArray converted;
        if (type == SkType_ARGB) {
            if (count == 4) {
                // !!! assert that it is SkType_Float ?
                animator->packARGB(&values->fScalar, count, &converted);
                values = converted.begin();
                count = converted.count();
            } else {
                SkASSERT(count == 1);
            }
        }
//      SkASSERT(type == SkType_ARGB || type == SkType_String ||info->isSettable());
        if (type == SkType_String || type == SkType_DynamicString)
            info->setString(target, values->fString);
        else if (type == SkType_Drawable || type == SkType_Displayable)
            target->setReference(info, values->fDisplayable);
        else
            info->setValue(target, values, count);
    }
}

bool SkApply::contains(SkDisplayable* child) {
    for (SkDrawable** curPtr = fScopes.begin(); curPtr < fScopes.end(); curPtr++) {
        if (*curPtr == child || (*curPtr)->contains(child))
            return true;
    }
    return fDeleteScope && scope == child;
}

SkDisplayable* SkApply::deepCopy(SkAnimateMaker* maker) {
    SkDrawable* saveScope = scope;
    scope = NULL;
    SkApply* result = (SkApply*) INHERITED::deepCopy(maker);
    result->scope = scope = saveScope;
    SkAnimateBase** end = fAnimators.end();
    for (SkAnimateBase** animPtr = fAnimators.begin(); animPtr < end; animPtr++) {
        SkAnimateBase* anim = (SkAnimateBase*) (*animPtr)->deepCopy(maker);
        *result->fAnimators.append() = anim;
        maker->helperAdd(anim);
    }
    return result;
}

void SkApply::disable() {
    //!!! this is the right thing to do, but has bad side effects because of other problems
    // currently, if an apply is in a g and scopes a statement in another g, it ends up as members
    // of both containers. The disabling here incorrectly disables both instances
    // maybe the fEnabled flag needs to be moved to the fActive data so that both
    // instances are not affected.
//  fEnabled = false;
}

bool SkApply::draw(SkAnimateMaker& maker) {
    if (scope ==NULL)
        return false;
    if (scope->isApply() || scope->isDrawable() == false)
        return false;
    if (fEnabled == false)
        enable(maker);
    SkASSERT(scope);
    activate(maker);
    if (mode == kMode_immediate)
        return fActive->draw();
    bool result = interpolate(maker, maker.getInTime());
    if (dontDraw == false) {
//      if (scope->isDrawable())
            result |= scope->draw(maker);
    }
    if (restore) {
        for (int index = 0; index < fActive->fAnimators.count(); index++)
            endSave(index);
        fActive->advance();
    }
    return result;
}

#ifdef SK_DUMP_ENABLED
void SkApply::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    if (dynamicScope.isEmpty() == false)
        SkDebugf("dynamicScope=\"%s\" ", dynamicScope.c_str());
    if (dontDraw)
        SkDebugf("dontDraw=\"true\" ");
    if (begin != 0) //perhaps we want this no matter what?
        SkDebugf("begin=\"%g\" ", (float) begin/1000.0f);   //is this correct?
    if (interval != (SkMSec) -1)
        SkDebugf("interval=\"%g\" ", (float) interval/1000.0f);
    if (steps != -1)
        SkDebugf("steps=\"%d\" ", steps);
    if (restore)
        SkDebugf("restore=\"true\" ");
    if (transition == kTransition_reverse)
        SkDebugf("transition=\"reverse\" ");
    if (mode == kMode_immediate) {
        SkDebugf("mode=\"immediate\" ");
    }
    else if (mode == kMode_create) {
        SkDebugf("mode=\"create\" ");
    }
    bool closedYet = false;
    SkDisplayList::fIndent += 4;
    int save = SkDisplayList::fDumpIndex;
    if (scope) {
        if (closedYet == false) {
            SkDebugf(">\n");
            closedYet = true;
        }
        scope->dump(maker);
    }
    int index;
//  if (fActive) {
        for (index = 0; index < fAnimators.count(); index++) {
            if (closedYet == false) {
                SkDebugf(">\n");
                closedYet = true;
            }
            SkAnimateBase* animator = fAnimators[index];
            animator->dump(maker);
//      }
    }
    SkDisplayList::fIndent -= 4;
    SkDisplayList::fDumpIndex = save;
    if (closedYet)
        dumpEnd(maker);
    else
        SkDebugf("/>\n");
}
#endif

bool SkApply::enable(SkAnimateMaker& maker) {
    fEnabled = true;
    bool initialized = fActive != NULL;
    if (dynamicScope.size() > 0)
        enableDynamic(maker);
    if (maker.fError.hasError())
        return false;
    int animators = fAnimators.count();
    int index;
    for (index = 0; index < animators; index++) {
        SkAnimateBase* animator = fAnimators[index];
        animator->fStart = maker.fEnableTime;
        animator->fResetPending = animator->fReset;
    }
    if (scope && scope->isApply())
        ((SkApply*) scope)->setEmbedded();
/*  if (mode == kMode_once) {
        if (scope) {
            activate(maker);
            interpolate(maker, maker.fEnableTime);
            inactivate(maker);
        }
        return true;
    }*/
    if ((mode == kMode_immediate || mode == kMode_create) && scope == NULL)
        return false;   // !!! error?
    bool enableMe = scope && (scope->hasEnable() || scope->isApply() || scope->isDrawable() == false);
    if ((mode == kMode_immediate && enableMe) || mode == kMode_create)
        activate(maker);    // for non-drawables like post, prime them here
    if (mode == kMode_immediate && enableMe)
        fActive->enable();
    if (mode == kMode_create && scope != NULL) {
        enableCreate(maker);
        return true;
    }
    if (mode == kMode_immediate) {
        return scope->isApply() || scope->isDrawable() == false;
    }
    refresh(maker);
    SkDisplayList& displayList = maker.fDisplayList;
    SkDrawable* drawable;
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
    SkString debugOut;
    SkMSec time = maker.getAppTime();
    debugOut.appendS32(time - maker.fDebugTimeBase);
    debugOut.append(" apply enable id=");
    debugOut.append(_id);
    debugOut.append("; start=");
    debugOut.appendS32(maker.fEnableTime - maker.fDebugTimeBase);
    SkDebugf("%s\n", debugOut.c_str());
#endif
    if (scope == NULL || scope->isApply() || scope->getType() == SkType_Movie || scope->isDrawable() == false) {
        activate(maker);    // for non-drawables like post, prime them here
        if (initialized) {
            append(this);
        }
        fEnabling = true;
        interpolate(maker, maker.fEnableTime);
        fEnabling = false;
        if (scope != NULL && dontDraw == false)
            scope->enable(maker);
        return true;
    } else if (initialized && restore == false)
        append(this);
#if 0
    bool wasActive = inactivate(maker); // start fresh
    if (wasActive) {
        activate(maker);
        interpolate(maker, maker.fEnableTime);
        return true;
    }
#endif
//  start here;
    // now that one apply might embed another, only the parent apply should replace the scope
    // or get appended to the display list
    // similarly, an apply added by an add immediate has already been located in the display list
    // and should not get moved or added again here
    if (fEmbedded) {
        return false;   // already added to display list by embedder
    }
    drawable = (SkDrawable*) scope;
    SkTDDrawableArray* parentList;
    SkTDDrawableArray* grandList;
    SkGroup* parentGroup;
    SkGroup* thisGroup;
    int old = displayList.findGroup(drawable, &parentList, &parentGroup, &thisGroup, &grandList);
    if (old < 0)
        goto append;
    else if (fContainsScope) {
        if ((*parentList)[old] != this || restore) {
append:
            if (parentGroup)
                parentGroup->markCopySize(old);
            if (parentList->count() < 10000) {
                fAppended = true;
                *parentList->append() = this;
            } else
                maker.setErrorCode(SkDisplayXMLParserError::kDisplayTreeTooDeep);
            old = -1;
        } else
            reset();
    } else {
        SkASSERT(old < parentList->count());
        if ((*parentList)[old]->isApply()) {
            SkApply* apply = (SkApply*) (*parentList)[old];
            if (apply != this && apply->fActive == NULL)
                apply->activate(maker);
            apply->append(this);
            parentGroup = NULL;
        } else {
            if (parentGroup)
                parentGroup->markCopySize(old);
            SkDrawable** newApplyLocation = &(*parentList)[old];
            SkGroup* pGroup;
            int oldApply = displayList.findGroup(this, &parentList, &pGroup, &thisGroup, &grandList);
            if (oldApply >= 0) {
                (*parentList)[oldApply] = (SkDrawable*) SkDisplayType::CreateInstance(&maker, SkType_Apply);
                parentGroup = NULL;
                fDeleteScope = true;
            }
            *newApplyLocation = this;
        }
    }
    if (parentGroup) {
        parentGroup->markCopySet(old);
        fDeleteScope = dynamicScope.size() == 0;
    }
    return true;
}

void SkApply::enableCreate(SkAnimateMaker& maker) {
    SkString newID;
    for (int step = 0; step <= steps; step++) {
        fLastTime = step * SK_MSec1;
        bool success = maker.computeID(scope, this, &newID);
        if (success == false)
            return;
        if (maker.find(newID.c_str(), NULL))
            continue;
        SkApply* copy = (SkApply*) deepCopy(&maker); // work on copy of animator state
        if (mode == kMode_create)
            copy->mode = (Mode) -1;
        SkDrawable* copyScope = copy->scope = (SkDrawable*) scope->deepCopy(&maker);
        *fScopes.append() = copyScope;
        if (copyScope->resolveIDs(maker, scope, this)) {
            step = steps; // quit
            goto next; // resolveIDs failed
        }
        if (newID.size() > 0)
            maker.setID(copyScope, newID);
        if (copy->resolveIDs(maker, this, this)) { // fix up all fields, including target
            step = steps; // quit
            goto next; // resolveIDs failed
        }
        copy->activate(maker);
        copy->interpolate(maker, step * SK_MSec1);
        maker.removeActive(copy->fActive);
    next:
        delete copy;
    }
}

void SkApply::enableDynamic(SkAnimateMaker& maker) {
    SkASSERT(mode != kMode_create); // create + dynamic are not currently compatible
    SkDisplayable* newScope;
    bool success = SkAnimatorScript::EvaluateDisplayable(maker, this, dynamicScope.c_str(),
        &newScope);
    if (success && scope != newScope) {
        SkTDDrawableArray* pList, * gList;
        SkGroup* pGroup = NULL, * found = NULL;
        int old = maker.fDisplayList.findGroup(scope, &pList, &pGroup, &found, &gList);
        if (pList && old >= 0 && (*pList)[old]->isApply() && (*pList)[old] != this) {
            if (fAppended == false) {
                if (found != NULL) {
                    SkDisplayable* oldChild = (*pList)[old];
                    if (oldChild->isApply() && found->copySet(old)) {
                        found->markCopyClear(old);
    //                  delete oldChild;
                    }
                }
                (*pList)[old] = scope;
            } else
                pList->remove(old);
        }
        scope = (SkDrawable*) newScope;
        onEndElement(maker);
    }
    maker.removeActive(fActive);
    delete fActive;
    fActive = NULL;
}

void SkApply::endSave(int index) {
    SkAnimateBase* animate = fActive->fAnimators[index];
    const SkMemberInfo* info = animate->fFieldInfo;
    SkDisplayTypes type = (SkDisplayTypes) info->fType;
    if (type == SkType_MemberFunction)
        return;
    SkDisplayable* target = getTarget(animate);
    size_t size = info->getSize(target);
    int count = (int) (size / sizeof(SkScalar));
    int activeIndex = fActive->fDrawIndex + index;
    SkOperand* last = new SkOperand[count];
    SkAutoTDelete<SkOperand> autoLast(last);
    if (type != SkType_MemberProperty) {
        info->getValue(target, last, count);
        SkOperand* saveOperand = fActive->fSaveRestore[activeIndex];
        if (saveOperand)
            info->setValue(target, fActive->fSaveRestore[activeIndex], count);
    } else {
        SkScriptValue scriptValue;
        SkDEBUGCODE(bool success = ) target->getProperty(info->propertyIndex(), &scriptValue);
        SkASSERT(success == true);
        last[0] = scriptValue.fOperand;
        scriptValue.fOperand = fActive->fSaveRestore[activeIndex][0];
        target->setProperty(info->propertyIndex(), scriptValue);
    }
    SkOperand* save = fActive->fSaveRestore[activeIndex];
    if (save)
        memcpy(save, last, count * sizeof(SkOperand));
}

bool SkApply::getProperty(int index, SkScriptValue* value) const {
    switch (index) {
        case SK_PROPERTY(step):
            value->fType = SkType_Int;
            value->fOperand.fS32 = fLastTime / SK_MSec1;
            break;
        case SK_PROPERTY(steps):
            value->fType = SkType_Int;
            value->fOperand.fS32 = steps;
            break;
        case SK_PROPERTY(time):
            value->fType = SkType_MSec;
            value->fOperand.fS32 = fLastTime;
            break;
        default:
    //      SkASSERT(0);
            return false;
    }
    return true;
}

void SkApply::getStep(SkScriptValue* value) {
    getProperty(SK_PROPERTY(step), value);
}

SkDrawable* SkApply::getTarget(SkAnimateBase* animate) {
    if (animate->fTargetIsScope == false || mode != kMode_create)
        return animate->fTarget;
    return scope;
}

bool SkApply::hasDelayedAnimator() const {
    SkAnimateBase* const* animEnd = fAnimators.end();
    for (SkAnimateBase* const* animPtr = fAnimators.begin(); animPtr < animEnd; animPtr++) {
        SkAnimateBase* const animator = *animPtr;
        if (animator->fDelayed)
            return true;
    }
    return false;
}

bool SkApply::hasEnable() const {
    return true;
}

bool SkApply::inactivate(SkAnimateMaker& maker) {
    if (fActive == NULL)
        return false;
    maker.removeActive(fActive);
    delete fActive;
    fActive = NULL;
    return true;
}

#ifdef SK_DEBUG
SkMSec lastTime = (SkMSec) -1;
#endif

bool SkApply::interpolate(SkAnimateMaker& maker, SkMSec rawTime) {
    if (fActive == NULL)
        return false;
    bool result = false;
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
    SkMSec time = maker.getAppTime();
    if (lastTime == (SkMSec) -1)
        lastTime = rawTime - 1;
    if (fActive != NULL &&
        strcmp(id, "a3") == 0 && rawTime > lastTime) {
        lastTime += 1000;
        SkString debugOut;
        debugOut.appendS32(time - maker.fDebugTimeBase);
        debugOut.append(" apply id=");
        debugOut.append(_id);
        debugOut.append("; ");
        debugOut.append(fActive->fAnimators[0]->_id);
        debugOut.append("=");
        debugOut.appendS32(rawTime - fActive->fState[0].fStartTime);
        debugOut.append(")");
        SkDebugf("%s\n", debugOut.c_str());
    }
#endif
    fActive->start();
    if (restore)
        fActive->initializeSave();
    int animators = fActive->fAnimators.count();
    for (int inner = 0; inner < animators; inner++) {
        SkAnimateBase* animate = fActive->fAnimators[inner];
        if (animate->fChanged) {
            animate->fChanged = false;
            animate->fStart = rawTime;
    //      SkTypedArray values;
    //      int count = animate->fValues.count();
    //      values.setCount(count);
    //      memcpy(values.begin(), animate->fValues.begin(), sizeof(SkOperand) * count);
            animate->onEndElement(maker);
    //      if (memcmp(values.begin(), animate->fValues.begin(), sizeof(SkOperand) * count) != 0) {
                fActive->append(this);
                fActive->start();
    //      }
        }
        SkMSec time = fActive->getTime(rawTime, inner);
        SkActive::SkState& state = fActive->fState[inner];
        if (SkMSec_LT(rawTime, state.fStartTime)) {
            if (fEnabling) {
                animate->fDelayed = true;
                maker.delayEnable(this, state.fStartTime);
            }
            continue;
        } else
            animate->fDelayed = false;
        SkMSec innerTime = fLastTime = state.getRelativeTime(time);
        if (restore)
            fActive->restoreInterpolatorValues(inner);
        if (animate->fReset) {
            if (transition != SkApply::kTransition_reverse) {
                if (SkMSec_LT(state.fBegin + state.fDuration, innerTime)) {
                    if (animate->fResetPending) {
                        innerTime = 0;
                        animate->fResetPending = false;
                    } else
                        continue;
                }
            } else if (innerTime == 0) {
                    if (animate->fResetPending) {
                        innerTime = state.fBegin + state.fDuration;
                        animate->fResetPending = false;
                    } else
                        continue;
            }
        }
        int count = animate->components();
        SkAutoSTMalloc<16, SkOperand> values(count);
        SkInterpolatorBase::Result interpResult = fActive->fInterpolators[inner]->timeToValues(
            innerTime, values.get());
        result |= (interpResult != SkInterpolatorBase::kFreezeEnd_Result);
        if (((transition != SkApply::kTransition_reverse && interpResult == SkInterpolatorBase::kFreezeEnd_Result) ||
                (transition == SkApply::kTransition_reverse && fLastTime == 0)) && state.fUnpostedEndEvent) {
//          SkDEBUGF(("interpolate: post on end\n"));
            state.fUnpostedEndEvent = false;
            maker.postOnEnd(animate, state.fBegin + state.fDuration);
            maker.fAdjustedStart = 0;    // !!! left over from synchronizing animation days, undoubtably out of date (and broken)
        }
        if (animate->formula.size() > 0) {
            if (fLastTime > animate->dur)
                fLastTime = animate->dur;
            SkTypedArray formulaValues;
            formulaValues.setCount(count);
            SkDEBUGCODE(bool success = ) animate->fFieldInfo->setValue(maker, &formulaValues, 0, 0, NULL,
                animate->getValuesType(), animate->formula);
            SkASSERT(success);
            if (restore)
                save(inner); // save existing value
            applyValues(inner, formulaValues.begin(), count, animate->getValuesType(), innerTime);
        } else {
            if (restore)
                save(inner); // save existing value
            applyValues(inner, values.get(), count, animate->getValuesType(), innerTime);
        }
    }
    return result;
}

void SkApply::initialize() {
    if (scope == NULL)
        return;
    if (scope->isApply() || scope->isDrawable() == false)
        return;
    scope->initialize();
}

void SkApply::onEndElement(SkAnimateMaker& maker)
{
    SkDrawable* scopePtr = scope;
    while (scopePtr && scopePtr->isApply()) {
        SkApply* scopedApply = (SkApply*) scopePtr;
        if (scopedApply->scope == this) {
            maker.setErrorCode(SkDisplayXMLParserError::kApplyScopesItself);
            return;
        }
        scopePtr = scopedApply->scope;
    }
    if (mode == kMode_create)
        return;
    if (scope != NULL && steps >= 0 && scope->isApply() == false && scope->isDrawable())
        scope->setSteps(steps);
    for (SkAnimateBase** animPtr = fAnimators.begin(); animPtr < fAnimators.end(); animPtr++) {
        SkAnimateBase* anim = *animPtr;
        //for reusing apply statements with dynamic scope
        if (anim->fTarget == NULL || anim->fTargetIsScope) {
            anim->fTargetIsScope = true;
            if (scope)
                anim->fTarget = scope;
            else
                anim->setTarget(maker);
            anim->onEndElement(maker);  // allows animate->fFieldInfo to be set
        }
        if (scope != NULL && steps >= 0 && anim->fTarget != scope && anim->fTarget->isDrawable())
            anim->fTarget->setSteps(steps);
    }
}

const SkMemberInfo* SkApply::preferredChild(SkDisplayTypes type) {
    SkASSERT(SkDisplayType::IsAnimate(type) == false);
    fContainsScope = true;
    return getMember("scope"); // !!! cwap! need to refer to member through enum like kScope instead
}

void SkApply::refresh(SkAnimateMaker& maker) {
    for (SkAnimateBase** animPtr = fAnimators.begin(); animPtr < fAnimators.end(); animPtr++) {
        SkAnimateBase* animate = *animPtr;
        animate->onEndElement(maker);
    }
    if (fActive)
        fActive->resetInterpolators();
}

void SkApply::reset() {
    if (fActive)
        fActive->resetState();
}

bool SkApply::resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* apply) { //   replace to/formula strings in animators of the form xxx.step with the step value, if xxx.step is in scope
    if (resolveField(maker, apply, &dynamicScope) == false)
        return true;    // failed
    SkAnimateBase** endPtr = fAnimators.end();
    SkAnimateBase** origPtr = ((SkApply*) original)->fAnimators.begin();
    for (SkAnimateBase** animPtr = fAnimators.begin(); animPtr < endPtr; ) {
        SkAnimateBase* animator = *animPtr++;
        maker.resolveID(animator, *origPtr++);
        if (resolveField(maker, this, &animator->target) == false)
            return true;
        if (resolveField(maker, this, &animator->from) == false)
            return true;
        if (resolveField(maker, this, &animator->to) == false)
            return true;
        if (resolveField(maker, this, &animator->formula) == false)
            return true;
    }
//  setEmbedded();
    onEndElement(maker);
    return false; // succeeded
}

bool SkApply::resolveField(SkAnimateMaker& maker, SkDisplayable* parent, SkString* str) {
    const char* script = str->c_str();
    if (str->startsWith("#string:") == false)
        return true;
    script += sizeof("#string:") - 1;
    return SkAnimatorScript::EvaluateString(maker, this, parent, script, str);
}

void SkApply::save(int index) {
    SkAnimateBase* animate = fActive->fAnimators[index];
    const SkMemberInfo * info = animate->fFieldInfo;
    SkDisplayable* target = getTarget(animate);
//  if (animate->hasExecute())
//      info = animate->getResolvedInfo();
    SkDisplayTypes type = (SkDisplayTypes) info->fType;
    if (type == SkType_MemberFunction)
        return; // nothing to save
    size_t size = info->getSize(target);
    int count = (int) (size / sizeof(SkScalar));
    bool useLast = true;
// !!! this all may be unneeded, at least in the dynamic case ??
    int activeIndex = fActive->fDrawIndex + index;
    SkTDOperandArray last;
    if (fActive->fSaveRestore[activeIndex] == NULL) {
        fActive->fSaveRestore[activeIndex] = new SkOperand[count];
        useLast = false;
    } else {
        last.setCount(count);
        memcpy(last.begin(), fActive->fSaveRestore[activeIndex], count * sizeof(SkOperand));
    }
    if (type != SkType_MemberProperty) {
        info->getValue(target, fActive->fSaveRestore[activeIndex], count);
        if (useLast)
            info->setValue(target, last.begin(), count);
    } else {
        SkScriptValue scriptValue;
        SkDEBUGCODE(bool success = ) target->getProperty(info->propertyIndex(), &scriptValue);
        SkASSERT(success == true);
        SkASSERT(scriptValue.fType == SkType_Float);
        fActive->fSaveRestore[activeIndex][0] = scriptValue.fOperand;
        if (useLast) {
            SkScriptValue scriptValue;
            scriptValue.fType = type;
            scriptValue.fOperand = last[0];
            target->setProperty(info->propertyIndex(), scriptValue);
        }
    }
// !!!  end of unneeded
}

bool SkApply::setProperty(int index, SkScriptValue& scriptValue) {
    switch (index) {
        case SK_PROPERTY(animator): {
            SkAnimateBase* animate = (SkAnimateBase*) scriptValue.fOperand.fDisplayable;
            SkASSERT(animate->isAnimate());
            *fAnimators.append() = animate;
            return true;
        }
        case SK_PROPERTY(steps):
            steps = scriptValue.fOperand.fS32;
            if (fActive)
                fActive->setSteps(steps);
            return true;
    }
    return false;
}

void SkApply::setSteps(int _steps) {
    steps = _steps;
}

#ifdef SK_DEBUG
void SkApply::validate() {
    if (fActive)
        fActive->validate();
}
#endif
