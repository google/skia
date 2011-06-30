/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */
#ifndef SkUserTrace_DEFINED
#define SkUserTrace_DEFINED

/* Sample implementation of SkUserTrace that routes all of the
   trace macros to debug output stream.
   To use this, redefine SK_USER_TRACE_INCLUDE_FILE in
   include/config/SkUserConfig.h to point to this file
*/
#define SK_TRACE_EVENT0(event) \
  SkDebugf("Trace: %s\n", event)
#define SK_TRACE_EVENT1(event, name1, value1) \
  SkDebugf("Trace: %s (%s=%s)\n", event, name1, value1)
#define SK_TRACE_EVENT2(event, name1, value1, name2, value2) \
  SkDebugf("Trace: %s (%s=%s, %s=%s)\n", event, name1, value1, name2, value2)

#endif


