
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBML_Verbs_DEFINED
#define SkBML_Verbs_DEFINED

enum Verbs {
    kStartElem_Value_Verb,
    kStartElem_Index_Verb,
    kEndElem_Verb,
    kAttr_Value_Value_Verb,
    kAttr_Value_Index_Verb,
    kAttr_Index_Value_Verb,
    kAttr_Index_Index_Verb,

    kVerbCount
};

#endif // SkBML_Verbs_DEFINED
