/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordPattern_DEFINED
#define SkRecordPattern_DEFINED

#include "SkRecord.h"
#include "SkTLogic.h"

namespace SkRecords {

// First, some matchers.  These match a single command in the SkRecord,
// and may hang onto some data from it.  If so, you can get the data by calling .get().

// Matches a command of type T, and stores that command.
template <typename T>
class Is {
public:
    Is() : fPtr(nullptr) {}

    typedef T type;
    type* get() { return fPtr; }

    bool operator()(T* ptr) {
        fPtr = ptr;
        return true;
    }

    template <typename U>
    bool operator()(U*) {
        fPtr = nullptr;
        return false;
    }

private:
    type* fPtr;
};

// Matches any command that draws, and stores its paint.
class IsDraw {
public:
    IsDraw() : fPaint(nullptr) {}

    typedef SkPaint type;
    type* get() { return fPaint; }

    template <typename T>
    SK_WHEN(T::kTags & kDraw_Tag, bool) operator()(T* draw) {
        fPaint = AsPtr(draw->paint);
        return true;
    }

    bool operator()(DrawDrawable*) {
        static_assert(DrawDrawable::kTags & kDraw_Tag, "");
        fPaint = nullptr;
        return true;
    }

    template <typename T>
    SK_WHEN(!(T::kTags & kDraw_Tag), bool) operator()(T* draw) {
        fPaint = nullptr;
        return false;
    }

private:
    // Abstracts away whether the paint is always part of the command or optional.
    template <typename T> static T* AsPtr(SkRecords::Optional<T>& x) { return x; }
    template <typename T> static T* AsPtr(T& x) { return &x; }

    type* fPaint;
};

// Matches if Matcher doesn't.  Stores nothing.
template <typename Matcher>
struct Not {
    template <typename T>
    bool operator()(T* ptr) { return !Matcher()(ptr); }
};

// Matches if any of First or Rest... does.  Stores nothing.
template <typename First, typename... Rest>
struct Or {
    template <typename T>
    bool operator()(T* ptr) { return First()(ptr) || Or<Rest...>()(ptr); }
};
template <typename First>
struct Or<First> {
    template <typename T>
    bool operator()(T* ptr) { return First()(ptr); }
};


// Greedy is a special matcher that greedily matches Matcher 0 or more times.  Stores nothing.
template <typename Matcher>
struct Greedy {
    template <typename T>
    bool operator()(T* ptr) { return Matcher()(ptr); }
};

// Pattern matches each of its matchers in order.
//
// This is the main entry point to pattern matching, and so provides a couple of extra API bits:
//  - search scans through the record to look for matches;
//  - first, second, third, ... return the data stored by their respective matchers in the pattern.

template <typename... Matchers> class Pattern;

template <> class Pattern<> {
public:
    // Bottoms out recursion.  Just return whatever i the front decided on.
    int match(SkRecord*, int i) { return i; }
};

template <typename First, typename... Rest>
class Pattern<First, Rest...> {
public:
    // If this pattern matches the SkRecord starting from i,
    // return the index just past the end of the pattern, otherwise return 0.
    SK_ALWAYS_INLINE int match(SkRecord* record, int i) {
        i = this->matchFirst(&fFirst, record, i);
        return i > 0 ? fRest.match(record, i) : 0;
    }

    // Starting from *end, walk through the SkRecord to find the first span matching this pattern.
    // If there is no such span, return false.  If there is, return true and set [*begin, *end).
    SK_ALWAYS_INLINE bool search(SkRecord* record, int* begin, int* end) {
        for (*begin = *end; *begin < record->count(); ++(*begin)) {
            *end = this->match(record, *begin);
            if (*end != 0) {
                return true;
            }
        }
        return false;
    }

    // TODO: some sort of smart get<i>()
    template <typename T> T* first()  { return fFirst.get();   }
    template <typename T> T* second() { return fRest.template first<T>();  }
    template <typename T> T* third()  { return fRest.template second<T>(); }
    template <typename T> T* fourth() { return fRest.template third<T>();  }

private:
    // If first isn't a Greedy, try to match at i once.
    template <typename T>
    int matchFirst(T* first, SkRecord* record, int i) {
        if (i < record->count()) {
            if (record->mutate(i, *first)) {
                return i+1;
            }
        }
        return 0;
    }

    // If first is a Greedy, walk i until it doesn't match.
    template <typename T>
    int matchFirst(Greedy<T>* first, SkRecord* record, int i) {
        while (i < record->count()) {
            if (!record->mutate(i, *first)) {
                return i;
            }
            i++;
        }
        return 0;
    }

    First            fFirst;
    Pattern<Rest...> fRest;
};

}  // namespace SkRecords

#endif//SkRecordPattern_DEFINED
