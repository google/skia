/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPreprocessorSeq_DEFINED
#define SkPreprocessorSeq_DEFINED

#define SK_CONCAT(a, b) SK_CONCAT_I(a, b) // allow for macro expansion
#define SK_CONCAT_I(a, b) a ## b

//A preprocessor pair is of the form '(a, b)'
#define SK_PAIR_FIRST(pair) SK_PAIR_FIRST_I pair
#define SK_PAIR_FIRST_I(a, b) a
#define SK_PAIR_SECOND(pair) SK_PAIR_SECOND_I pair
#define SK_PAIR_SECOND_I(a, b) b

//A preprocessor sequence is of the form (a)(b)(c)SK_SEQ_END
#if 0
//The following is what we logically want to do.
//However, MSVC expands macros at the wrong time, causing an error on use of
//SK_SEQ_IGNORE_SECOND_I because it formally takes two parameters, but we only
//pass "one" parameter in SK_SEQ_IGNORE_SECOND.

#define SK_SEQ_HEAD(seq) SK_SEQ_IGNORE_SECOND(SK_SEQ_FIRST seq)
#define SK_SEQ_IGNORE_SECOND(x) SK_SEQ_IGNORE_SECOND_I(x) // expand x
#define SK_SEQ_IGNORE_SECOND_I(x, _) x
#define SK_SEQ_FIRST(x) x, SK_NIL

#else

//This is less obvious, but works on GCC, clang, and MSVC.
#define SK_SEQ_HEAD(seq) SK_SEQ_HEAD_II((SK_SEQ_FIRST seq))
#define SK_SEQ_HEAD_II(x) SK_SEQ_IGNORE_SECOND_I x
#define SK_SEQ_IGNORE_SECOND_I(x, _) x
#define SK_SEQ_FIRST(x) x, SK_NIL

#endif

#define SK_SEQ_TAIL(seq) SK_SEQ_TAIL_I seq
#define SK_SEQ_TAIL_I(x)

#define SK_SEQ_SIZE(seq) SK_CONCAT(SK_SEQ_SIZE_, SK_SEQ_SIZE_TERMINATOR seq)

#define SK_SEQ_SIZE_TERMINATOR(_) SK_SEQ_SIZE_0
#define SK_SEQ_SIZE_0(_) SK_SEQ_SIZE_1
#define SK_SEQ_SIZE_1(_) SK_SEQ_SIZE_2
#define SK_SEQ_SIZE_2(_) SK_SEQ_SIZE_3
#define SK_SEQ_SIZE_3(_) SK_SEQ_SIZE_4
#define SK_SEQ_SIZE_4(_) SK_SEQ_SIZE_5
#define SK_SEQ_SIZE_5(_) SK_SEQ_SIZE_6
#define SK_SEQ_SIZE_6(_) SK_SEQ_SIZE_7
#define SK_SEQ_SIZE_7(_) SK_SEQ_SIZE_8
#define SK_SEQ_SIZE_8(_) SK_SEQ_SIZE_9
#define SK_SEQ_SIZE_9(_) SK_SEQ_SIZE_10
#define SK_SEQ_SIZE_10(_) SK_SEQ_SIZE_11
#define SK_SEQ_SIZE_11(_) SK_SEQ_SIZE_12
#define SK_SEQ_SIZE_12(_) SK_SEQ_SIZE_13
#define SK_SEQ_SIZE_13(_) SK_SEQ_SIZE_14
#define SK_SEQ_SIZE_14(_) SK_SEQ_SIZE_15
#define SK_SEQ_SIZE_15(_) SK_SEQ_SIZE_16
#define SK_SEQ_SIZE_16(_) SK_SEQ_SIZE_17

#define SK_SEQ_SIZE_SK_SEQ_SIZE_0 0
#define SK_SEQ_SIZE_SK_SEQ_SIZE_1 1
#define SK_SEQ_SIZE_SK_SEQ_SIZE_2 2
#define SK_SEQ_SIZE_SK_SEQ_SIZE_3 3
#define SK_SEQ_SIZE_SK_SEQ_SIZE_4 4
#define SK_SEQ_SIZE_SK_SEQ_SIZE_5 5
#define SK_SEQ_SIZE_SK_SEQ_SIZE_6 6
#define SK_SEQ_SIZE_SK_SEQ_SIZE_7 7
#define SK_SEQ_SIZE_SK_SEQ_SIZE_8 8
#define SK_SEQ_SIZE_SK_SEQ_SIZE_9 9
#define SK_SEQ_SIZE_SK_SEQ_SIZE_10 10
#define SK_SEQ_SIZE_SK_SEQ_SIZE_11 11
#define SK_SEQ_SIZE_SK_SEQ_SIZE_12 12
#define SK_SEQ_SIZE_SK_SEQ_SIZE_13 13
#define SK_SEQ_SIZE_SK_SEQ_SIZE_14 14
#define SK_SEQ_SIZE_SK_SEQ_SIZE_15 15
#define SK_SEQ_SIZE_SK_SEQ_SIZE_16 16
#define SK_SEQ_SIZE_SK_SEQ_SIZE_17 17

#define SK_SEQ_FOREACH(op, data, seq) SK_SEQ_FOREACH_L(op, op, data, seq)
#define SK_SEQ_FOREACH_L(op, lop, data, seq) SK_CONCAT(SK_SEQ_FOREACH_, SK_SEQ_SIZE(seq)) (op, lop, data, SK_SEQ_HEAD(seq), SK_SEQ_TAIL(seq))

#define SK_SEQ_FOREACH_0(op,lop,d,x,t)
#define SK_SEQ_FOREACH_1(op,lop,d,x,t) lop(d,x)
#define SK_SEQ_FOREACH_2(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_1(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_3(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_2(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_4(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_3(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_5(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_4(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_6(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_5(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_7(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_6(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_8(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_7(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_9(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_8(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_10(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_9(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_11(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_10(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_12(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_11(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_13(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_12(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_14(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_13(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_15(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_14(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_16(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_15(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_17(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_16(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))

#define SK_SEQ_END (SK_NIL)

#endif
