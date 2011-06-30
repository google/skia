/*
    Copyright 2010 Google Inc.

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

/* If your system embeds skia and has complex event logging, define these
   to map to your system's equivalents. A trivial example is given in
   src/utils/SkDebugTrace.h.

   All arguments are const char*. Skia typically passes the name of
   the object and function (and sometimes region of interest within
   the function) separated by double colons for 'event'.

   SK_TRACE_EVENT1 and SK_TRACE_EVENT2 take one or two arbitrary
   name-value pairs that you also want to log. SkStringPrintf() is useful
   for formatting these values.

   For example:
    SK_TRACE_EVENT0("GrContext::createAndLockTexture");
    SK_TRACE_EVENT1("GrDefaultPathRenderer::onDrawPath::renderPasses",
                    "verts", SkStringPrintf("%i", vert - base).c_str());
*/
#define SK_TRACE_EVENT0(event)
#define SK_TRACE_EVENT1(event, name1, value1)
#define SK_TRACE_EVENT2(event, name1, value1, name2, value2)

#endif


