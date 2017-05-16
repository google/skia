//
// Copyright (c) 2012-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/DirectiveHandler.h"

#include <sstream>

#include "common/debug.h"
#include "compiler/translator/Diagnostics.h"

static TBehavior getBehavior(const std::string& str)
{
    const char kRequire[] = "require";
    const char kEnable[] = "enable";
    const char kDisable[] = "disable";
    const char kWarn[] = "warn";

    if (str == kRequire) return EBhRequire;
    else if (str == kEnable) return EBhEnable;
    else if (str == kDisable) return EBhDisable;
    else if (str == kWarn) return EBhWarn;
    return EBhUndefined;
}

TDirectiveHandler::TDirectiveHandler(TExtensionBehavior& extBehavior,
                                     TDiagnostics& diagnostics,
                                     int& shaderVersion,
                                     bool debugShaderPrecisionSupported)
    : mExtensionBehavior(extBehavior),
      mDiagnostics(diagnostics),
      mShaderVersion(shaderVersion),
      mDebugShaderPrecisionSupported(debugShaderPrecisionSupported)
{
}

TDirectiveHandler::~TDirectiveHandler()
{
}

void TDirectiveHandler::handleError(const pp::SourceLocation& loc,
                                    const std::string& msg)
{
    mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc, msg, "", "");
}

void TDirectiveHandler::handlePragma(const pp::SourceLocation& loc,
                                     const std::string& name,
                                     const std::string& value,
                                     bool stdgl)
{
    if (stdgl)
    {
        const char kInvariant[] = "invariant";
        const char kAll[] = "all";

        if (name == kInvariant && value == kAll)
            mPragma.stdgl.invariantAll = true;
        // The STDGL pragma is used to reserve pragmas for use by future
        // revisions of GLSL.  Do not generate an error on unexpected
        // name and value.
        return;
    }
    else
    {
        const char kOptimize[] = "optimize";
        const char kDebug[] = "debug";
        const char kDebugShaderPrecision[] = "webgl_debug_shader_precision";
        const char kOn[] = "on";
        const char kOff[] = "off";

        bool invalidValue = false;
        if (name == kOptimize)
        {
            if (value == kOn) mPragma.optimize = true;
            else if (value == kOff) mPragma.optimize = false;
            else invalidValue = true;
        }
        else if (name == kDebug)
        {
            if (value == kOn) mPragma.debug = true;
            else if (value == kOff) mPragma.debug = false;
            else invalidValue = true;
        }
        else if (name == kDebugShaderPrecision && mDebugShaderPrecisionSupported)
        {
            if (value == kOn) mPragma.debugShaderPrecision = true;
            else if (value == kOff) mPragma.debugShaderPrecision = false;
            else invalidValue = true;
        }
        else
        {
            mDiagnostics.report(pp::Diagnostics::PP_UNRECOGNIZED_PRAGMA, loc, name);
            return;
        }

        if (invalidValue)
        {
            mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc,
                                   "invalid pragma value", value,
                                   "'on' or 'off' expected");
        }
    }
}

void TDirectiveHandler::handleExtension(const pp::SourceLocation& loc,
                                        const std::string& name,
                                        const std::string& behavior)
{
    const char kExtAll[] = "all";

    TBehavior behaviorVal = getBehavior(behavior);
    if (behaviorVal == EBhUndefined)
    {
        mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc,
                               "behavior", name, "invalid");
        return;
    }

    if (name == kExtAll)
    {
        if (behaviorVal == EBhRequire)
        {
            mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc,
                                   "extension", name,
                                   "cannot have 'require' behavior");
        }
        else if (behaviorVal == EBhEnable)
        {
            mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc,
                                   "extension", name,
                                   "cannot have 'enable' behavior");
        }
        else
        {
            for (TExtensionBehavior::iterator iter = mExtensionBehavior.begin();
                 iter != mExtensionBehavior.end(); ++iter)
                iter->second = behaviorVal;
        }
        return;
    }

    TExtensionBehavior::iterator iter = mExtensionBehavior.find(name);
    if (iter != mExtensionBehavior.end())
    {
        iter->second = behaviorVal;
        return;
    }

    pp::Diagnostics::Severity severity = pp::Diagnostics::PP_ERROR;
    switch (behaviorVal) {
      case EBhRequire:
        severity = pp::Diagnostics::PP_ERROR;
        break;
      case EBhEnable:
      case EBhWarn:
      case EBhDisable:
        severity = pp::Diagnostics::PP_WARNING;
        break;
      default:
        UNREACHABLE();
        break;
    }
    mDiagnostics.writeInfo(severity, loc,
                           "extension", name, "is not supported");
}

void TDirectiveHandler::handleVersion(const pp::SourceLocation& loc,
                                      int version)
{
    if (version == 100 ||
        version == 300)
    {
        mShaderVersion = version;
    }
    else
    {
        std::stringstream stream;
        stream << version;
        std::string str = stream.str();
        mDiagnostics.writeInfo(pp::Diagnostics::PP_ERROR, loc,
                               "version number", str, "not supported");
    }
}
