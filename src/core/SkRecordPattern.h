#ifndef SkRecordPattern_DEFINED
#define SkRecordPattern_DEFINED

#include "SkTLogic.h"

namespace SkRecords {

// First, some matchers.  These match a single command in the SkRecord,
// and may hang onto some data from it.  If so, you can get the data by calling .get().

// Matches a command of type T, and stores that command.
template <typename T>
class Is {
public:
    Is() : fPtr(NULL) {}

    typedef T type;
    type* get() { return fPtr; }

    bool operator()(T* ptr) {
        fPtr = ptr;
        return true;
    }

    template <typename U>
    bool operator()(U*) {
        fPtr = NULL;
        return false;
    }

private:
    type* fPtr;
};

// Matches any command that draws, and stores its paint.
class IsDraw {
    SK_CREATE_MEMBER_DETECTOR(paint);
public:
    IsDraw() : fPaint(NULL) {}

    typedef SkPaint type;
    type* get() { return fPaint; }

    template <typename T>
    SK_WHEN(HasMember_paint<T>, bool) operator()(T* draw) {
        fPaint = AsPtr(draw->paint);
        return true;
    }

    template <typename T>
    SK_WHEN(!HasMember_paint<T>, bool) operator()(T*) {
        fPaint = NULL;
        return false;
    }

    // SaveLayer has an SkPaint named paint, but it's not a draw.
    bool operator()(SaveLayer*) {
        fPaint = NULL;
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

// Matches if either of A or B does.  Stores nothing.
template <typename A, typename B>
struct Or {
    template <typename T>
    bool operator()(T* ptr) { return A()(ptr) || B()(ptr); }
};

// Matches if any of A, B or C does.  Stores nothing.
template <typename A, typename B, typename C>
struct Or3 : Or<A, Or<B, C> > {};

// Matches if any of A, B, C or D does.  Stores nothing.
template <typename A, typename B, typename C, typename D>
struct Or4 : Or<A, Or<B, Or<C, D> > > {};

// Star is a special matcher that greedily matches Matcher 0 or more times.  Stores nothing.
template <typename Matcher>
struct Star {
    template <typename T>
    bool operator()(T* ptr) { return Matcher()(ptr); }
};

// Cons builds a list of Matchers.
// It first matches Matcher (something from above), then Pattern (another Cons or Nil).
//
// This is the main entry point to pattern matching, and so provides a couple of extra API bits:
//  - search scans through the record to look for matches;
//  - first, second, and third return the data stored by their respective matchers in the pattern.
//
// These Cons build lists analogously to Lisp's "cons".  See Pattern# for the "list" equivalent.
template <typename Matcher, typename Pattern>
class Cons {
public:
    // If this pattern matches the SkRecord starting at i,
    // return the index just past the end of the pattern, otherwise return 0.
    SK_ALWAYS_INLINE unsigned match(SkRecord* record, unsigned i) {
        i = this->matchHead(&fHead, record, i);
        return i == 0 ? 0 : fTail.match(record, i);
    }

    // Starting from *end, walk through the SkRecord to find the first span matching this pattern.
    // If there is no such span, return false.  If there is, return true and set [*begin, *end).
    SK_ALWAYS_INLINE bool search(SkRecord* record, unsigned* begin, unsigned* end) {
        for (*begin = *end; *begin < record->count(); ++(*begin)) {
            *end = this->match(record, *begin);
            if (*end != 0) {
                return true;
            }
        }
        return false;
    }

    // Once either match or search has succeeded, access the stored data of the first, second,
    // or third matcher in this pattern.  Add as needed for longer patterns.
    // T is checked statically at compile time; no casting is involved.  It's just an API wart.
    template <typename T> T* first()  { return fHead.get(); }
    template <typename T> T* second() { return fTail.fHead.get(); }
    template <typename T> T* third()  { return fTail.fTail.fHead.get(); }

private:
    // If head isn't a Star, try to match at i once.
    template <typename T>
    unsigned matchHead(T*, SkRecord* record, unsigned i) {
        if (i < record->count()) {
            if (record->mutate<bool>(i, fHead)) {
                return i+1;
            }
        }
        return 0;
    }

    // If head is a Star, walk i until it doesn't match.
    template <typename T>
    unsigned matchHead(Star<T>*, SkRecord* record, unsigned i) {
        while (i < record->count()) {
            if (!record->mutate<bool>(i, fHead)) {
                return i;
            }
            i++;
        }
        return 0;
    }

    Matcher fHead;
    Pattern fTail;

    // All Cons are friends with each other.  This lets first, second, and third work.
    template <typename, typename> friend class Cons;
};

// Nil is the end of every pattern Cons chain.
struct Nil {
    // Bottoms out recursion down the fTail chain.  Just return whatever i the front decided on.
    unsigned match(SkRecord*, unsigned i) { return i; }
};

// These Pattern# types are syntax sugar over Cons and Nil, just to help eliminate some of the
// template noise.  Use these if you can.  Feel free to add more for longer patterns.
// All types A, B, C, ... are Matchers.
template <typename A>
struct Pattern1 : Cons<A, Nil> {};

template <typename A, typename B>
struct Pattern2 : Cons<A, Pattern1<B> > {};

template <typename A, typename B, typename C>
struct Pattern3 : Cons<A, Pattern2<B, C> > {};

}  // namespace SkRecords

#endif//SkRecordPattern_DEFINED
