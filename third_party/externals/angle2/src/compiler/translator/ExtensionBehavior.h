//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_
#define COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_

#include <map>
#include <string>

typedef enum
{
    EBhRequire,
    EBhEnable,
    EBhWarn,
    EBhDisable,
    EBhUndefined
} TBehavior;

inline const char* getBehaviorString(TBehavior b)
{
    switch(b)
    {
      case EBhRequire: return "require";
      case EBhEnable: return "enable";
      case EBhWarn: return "warn";
      case EBhDisable: return "disable";
      default: return NULL;
    }
}

// Mapping between extension name and behavior.
typedef std::map<std::string, TBehavior> TExtensionBehavior;

inline bool IsExtensionEnabled(const TExtensionBehavior &extBehavior, const char *extension)
{
    auto iter = extBehavior.find(extension);
    return iter != extBehavior.end() && (iter->second == EBhEnable || iter->second == EBhRequire);
}

#endif // COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_
