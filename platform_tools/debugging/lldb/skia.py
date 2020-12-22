# Copyright (c) 2020 Google LLC. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# The following Skia types can be natively visualized in LLDB:
# - SkAutoTArray, SkAutoSTArray
# - SkString
# - SkTArray, SkSTArray
# - sk_sp
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
    # The fPtr inside fRec is automatically consumed by sk_sp_SynthProvider.
    fLength = fRec.GetChildMemberWithName('fLength')
    if fLength.GetValueAsUnsigned(0) <= 0:
        return '""'
    fBeginningOfData = fRec.GetChildMemberWithName('fBeginningOfData')

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
            count = max(count, 0)
            count = min(count, 10000)
            return count
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


class SkAutoTArray_SynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj

    def num_children(self):
        try:
            count = self.fCount.GetValueAsSigned(0)
            count = max(count, 0)
            count = min(count, 10000)
            return count
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
            return self.fValue.CreateChildAtOffset('[' + str(index) + ']',
                                                   offset, self.dataType)
        except:
            return None

    def update(self):
        try:
            self.fCount = self.valobj.GetChildMemberWithName('fCount')
            fArray = self.valobj.GetChildMemberWithName('fArray')
            # These lookups rely on implementation details of unique_ptr and __compressed_pair.
            ptr = fArray.GetChildMemberWithName('__ptr_')
            self.fValue = ptr.GetChildMemberWithName('__value_')
            self.dataType = self.fValue.GetType().GetPointeeType()
            self.dataSize = self.dataType.GetByteSize()
        except:
            pass

    def has_children(self):
        return True


class SkSpan_SynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj

    def num_children(self):
        try:
            count = self.fSize.GetValueAsSigned(0)
            count = max(count, 0)
            count = min(count, 10000)
            return count
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
            return self.fPtr.CreateChildAtOffset('[' + str(index) + ']',
                                                 offset, self.dataType)
        except:
            return None

    def update(self):
        try:
            self.fPtr = self.valobj.GetChildMemberWithName('fPtr')
            self.fSize = self.valobj.GetChildMemberWithName('fSize')
            self.dataType = self.fPtr.GetType().GetPointeeType()
            self.dataSize = self.dataType.GetByteSize()
        except:
            pass

    def has_children(self):
        return True



class sk_sp_SynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj

    def num_children(self):
        return self.fPtr.GetNumChildren()

    def get_child_at_index(self, index):
        try:
            return self.fPtr.GetChildAtIndex(index)
        except:
            return None

    def get_child_index(self, name):
        return self.fPtr.GetIndexOfChildWithName(name)

    def update(self):
        try:
            self.fPtr = self.valobj.GetChildMemberWithName('fPtr')
        except:
            pass


def __lldb_init_module(debugger, dict):
    debugger.HandleCommand(
        'type summary add -F skia.SkString_SummaryProvider "SkString" -w skia')
    debugger.HandleCommand(
        'type synthetic add -l skia.sk_sp_SynthProvider -x "^sk_sp<.+>$" -w skia')
    debugger.HandleCommand(
        'type summary add --summary-string "fPtr = ${var.fPtr}" -x "^sk_sp<.+>$" -w skia')
    debugger.HandleCommand(
        'type synthetic add -l skia.SkTArray_SynthProvider -x "^SkS?TArray<.+>$" -w skia')
    debugger.HandleCommand(
        'type synthetic add -l skia.SkSpan_SynthProvider -x "^SkSpan<.+>$" -w skia')
    debugger.HandleCommand(
        'type summary add --summary-string "size=${svar%#}" -e -x "^SkS?TArray<.+>$" -w skia')
    debugger.HandleCommand(
        'type synthetic add -l skia.SkAutoTArray_SynthProvider -x "^SkAutoS?TArray<.+>$" -w skia')
    debugger.HandleCommand(
        'type summary add --summary-string "size=${svar%#}" -e -x "^SkAutoS?TArray<.+>$" -w skia')
    debugger.HandleCommand("type category enable skia")
