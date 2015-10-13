//
// Copyright (c) 2012-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Diagnostics.h"

#include "common/debug.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/preprocessor/SourceLocation.h"

TDiagnostics::TDiagnostics(TInfoSink& infoSink) :
    mInfoSink(infoSink),
    mNumErrors(0),
    mNumWarnings(0)
{
}

TDiagnostics::~TDiagnostics()
{
}

void TDiagnostics::writeInfo(Severity severity,
                             const pp::SourceLocation& loc,
                             const std::string& reason,
                             const std::string& token,
                             const std::string& extra)
{
    TPrefixType prefix = EPrefixNone;
    switch (severity)
    {
      case PP_ERROR:
        ++mNumErrors;
        prefix = EPrefixError;
        break;
      case PP_WARNING:
        ++mNumWarnings;
        prefix = EPrefixWarning;
        break;
      default:
        UNREACHABLE();
        break;
    }

    TInfoSinkBase& sink = mInfoSink.info;
    /* VC++ format: file(linenum) : error #: 'token' : extrainfo */
    sink.prefix(prefix);
    sink.location(loc.file, loc.line);
    sink << "'" << token <<  "' : " << reason << " " << extra << "\n";
}

void TDiagnostics::print(ID id,
                         const pp::SourceLocation& loc,
                         const std::string& text)
{
    writeInfo(severity(id), loc, message(id), text, "");
}
