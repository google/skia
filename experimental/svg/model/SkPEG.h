/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPEG_DEFINED
#define SkPEG_DEFINED

#include "SkTArray.h"
#include "SkTLazy.h"

namespace skpeg {

/**
 *  The result of an expression match attempt.
 *
 *  If the match was successful, |fNext| points to the next unconsumed character in the
 *  input string, and |fValue| holds an (arbitrarily nested) match result value.
 *
 *  Otherwise, |fNext| is nullptr and |fValue| is uninitialized.
 */
template <typename V>
struct MatchResult {
    MatchResult(std::nullptr_t) : fNext(nullptr) {}
    MatchResult(const char* next, const V& v) : fNext(next), fValue(&v) {}

    operator bool() const {
        SkASSERT(fValue.isValid() == SkToBool(fNext));
        return SkToBool(fNext);
    }

    const V& operator* () const { return *fValue.get(); }
    const V* operator->() const { return  fValue.get(); }

    const char* fNext;
    SkTLazy<V>  fValue;
};

/**
 * Optional operator (e?).  Always succeeds.
 *
 * If e also matches, then the result of e::Match() is stored in |fValue|.
 * Otherwise, |fValue| is uninitialized.
 *
 */
template <typename E>
struct Opt {
    struct V {
        V(const typename E::V* v) : fValue(v) {}

        SkTLazy<typename E::V> fValue;
    };
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        const auto m = E::Match(in);
        return m ? MatchT(m.fNext, V(m.fValue.get()))
                 : MatchT(in, nullptr);
    }
};

/**
 * Helper for selecting the value type of the n-th expression type in the list.
 */
template <size_t, typename... Es> struct SelectV;

template <typename E, typename... Es>
struct SelectV<0, E, Es...> {
    using V = typename E::V;
};

template <size_t idx, typename E, typename... Es>
struct SelectV<idx, E, Es...> {
    using V = typename SelectV<idx - 1, Es...>::V;
};

/**
 * Sequence operator (e0 e1...).
 *
 * Succeeds when all expressions match, in sequence.  The subexpression match
 * results can be accessed via get<INDEX>() -- where get<0> returns the value
 * of the first expression, and so on.
 *
 */
template <typename... E> struct Seq;

template <>
struct Seq<> {
    struct V {};
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        return MatchT(in, V());
    }
};

template <typename E, typename... Es>
struct Seq<E, Es...> {
    class V {
    public:
        V(const typename E::V& head, const typename Seq<Es...>::V& tail)
            : fHeadV(head), fTailV(tail) {}

        template <size_t idx, typename std::enable_if<idx == 0, bool>::type = 0>
        const typename E::V& get() const {
            return fHeadV;
        }

        template <size_t idx, typename std::enable_if<idx != 0, bool>::type = 0>
        const typename SelectV<idx, E, Es...>::V& get() const {
            return fTailV.template get<idx - 1>();
        }

    private:
        typename E::V          fHeadV;
        typename Seq<Es...>::V fTailV;
    };
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        const auto headMatch = E::Match(in);
        if (!headMatch) {
            return nullptr;
        }

        const auto tailMatch = Seq<Es...>::Match(headMatch.fNext);
        return tailMatch ? MatchT(tailMatch.fNext, V(*headMatch, *tailMatch))
                         : nullptr;
    }
};

/**
 * Ordered choice operator (e1|e2).
 *
 * Succeeds when either e1 or e2 match (e1 is tried first, then e2).
 *
 * The (optional) match results are stored in |v1|, |v2|.
 *
 */
template <typename E1, typename E2>
struct Choice {
    struct V {
        V (const typename E1::V* v1, const typename E2::V* v2) : v1(v1), v2(v2)
        {
            SkASSERT(!v1 || !v2);
        }

        SkTLazy<typename E1::V> v1;
        SkTLazy<typename E2::V> v2;
    };
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        if (const auto m1 = E1::Match(in)) {
            return MatchT(m1.fNext, V(m1.fValue.get(), nullptr));
        }
        if (const auto m2 = E2::Match(in)) {
            return MatchT(m2.fNext, V(nullptr, m2.fValue.get()));
        }
        return nullptr;
    }
};

/**
 * Zero-or-more operator (e*).  Always succeeds.
 *
 * Matches e greedily, and stores the match results in |fValues|.
 *
 */
template <typename E>
struct Any {
    struct V {
        V(SkTArray<typename E::V>&& vs) : fValues(vs) {}

        SkTArray<typename E::V> fValues;
    };
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        SkTArray<typename E::V> values;
        while (const auto m = E::Match(in)) {
            in = m.fNext;
            values.push_back(*m);
        }
        return MatchT(in, std::move(values));
    }
};

/**
 * One-or-more operator (e+).
 *
 * Same as zero-or-more, except it fails if e doesn't match at least once.
 *
 */
template <typename E>
using Some = Seq<E, Any<E>>;

/**
 * End-of-string atom.  Matches \0.
 */
struct EOS {
    struct V {};
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        return (*in != '\0') ? nullptr : MatchT(in, V());
    }
};


/**
 * Literal atom.  Matches a list of char literals.
 */
template <char... Cs> struct LIT;

template <>
struct LIT<> {
    struct V {};
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        return MatchT(in, V());
    }
};

template <char C, char... Cs>
struct LIT<C, Cs...> {
    struct V {};
    using MatchT = MatchResult<V>;

    static MatchT Match(const char* in) {
        if (*in != C) {
            return nullptr;
        }
        const auto m = LIT<Cs...>::Match(in + 1);
        return m ? MatchT(m.fNext, V()) : nullptr;
    }
};

} // skpeg ns

#endif // SkPEG_DEFINED
