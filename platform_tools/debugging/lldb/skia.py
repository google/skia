# Copyright (c) 2020 Google LLC. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# The following Skia types can be natively visualized in LLDB:
# - SkTArray, SkSTArray
# - SkString
#
# To enable LLDB debugging support, run the following command at the (lldb) prompt:
#
#      command script import (your-skia-local-path)/platform_tools/debugging/lldb/skia.py
#
# This can be automatically enabled at the start of every debugging session by creating a
# ~/.lldbinit file which contains this command.

import lldb

def SkString_SummaryProvider(valobj, dict):
    fRec = valobj.GetChildMemberWithName('fRec')
    fPtr = fRec.GetChildMemberWithName('fPtr')
    fLength = fPtr.GetChildMemberWithName('fLength')
    if fLength.GetValueAsUnsigned(0) <= 0:
        return '""'
    fBeginningOfData = fPtr.GetChildMemberWithName('fBeginningOfData')

    # Fetch string contents into an SBData.
    string = fBeginningOfData.AddressOf().GetPointeeData(0, fLength.GetValueAsUnsigned(0))
    # Zero terminate the SBData. (This actually adds four zero bytes, but that's harmless.)
    string.Append(lldb.SBData.CreateDataFromInt(0))
    # Convert our SBData into a string.
    error = lldb.SBError()
    string = string.GetString(error, 0)
    if error.Fail():
        return '<error: ' + error.GetCString() + '>'
    else:
        return '"' + string + '"'

class SkTArray_SynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj

    def num_children(self):
        try:
            count = self.fCount.GetValueAsSigned(0)
            return count if count >= 0 else 0
        except:
            return 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None

        try:
            offset = index * self.dataSize
            return self.fItemArray.CreateChildAtOffset('[' + str(index) + ']',
                                                       offset, self.dataType)
        except:
            return None

    def update(self):
        try:
            self.fItemArray = self.valobj.GetChildMemberWithName('fItemArray')
            self.fCount = self.valobj.GetChildMemberWithName('fCount')
            self.dataType = self.fItemArray.GetType().GetPointeeType()
            self.dataSize = self.dataType.GetByteSize()
        except:
            pass

    def has_children(self):
        return True


def __lldb_init_module(debugger, dict):
    debugger.HandleCommand(
        'type summary add -F skia.SkString_SummaryProvider "SkString" -w skia')
    debugger.HandleCommand(
        'type synthetic add -l skia.SkTArray_SynthProvider -x "^SkS?TArray<.+>$" -w skia')
    debugger.HandleCommand(
        'type summary add --summary-string "size=${svar%#}" -e -x "^SkS?TArray<.+>$" -w skia')
    debugger.HandleCommand("type category enable skia")
