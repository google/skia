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


#ifndef GrPathIter_DEFINED
#define GrPathIter_DEFINED

#include "GrRect.h"

/**
 2D Path iterator. Porting layer creates a subclass of this. It allows Ganesh to
 parse the top-level API's 2D paths. Supports lines, quadratics, and cubic
 pieces and moves (multi-part paths).
 */
class GrPathIter {
public:

    virtual ~GrPathIter() {};

    /**
     * Iterates through the path. Should not be called after
     * kEnd_Command has been returned once. This version retrieves the
     * points for the command.
     * @param points  The points relevant to returned commend. See Command
     *               enum for number of points valid for each command.
     * @return The next command of the path.
     */
    virtual GrPathCmd next(GrPoint points[4]) = 0;

    /**
     * If the host API has knowledge of the convexity of the path
     * it can be communicated by this hint. Gr can analyze the path
     * as it is iterated. So it is not necessary to do additional work to
     * compute convexity status if it isn't already determined.
     *
     * @return a hint about the convexity of the path.
     */
    virtual GrConvexHint convexHint() const = 0;

     /**
      * Iterates through the path. Should not be called after
      * kEnd_Command has been returned once. This version does not retrieve the
      * points for the command.
      * @return The next command of the path.
      */
     virtual GrPathCmd next() = 0;

     /**
      * Returns conservative bounds on the path points. If returns false then
      * no bounds are available.
      */
     virtual bool getConservativeBounds(GrRect* rect) const = 0;

    /**
     Restarts iteration from the beginning.
     */
    virtual void rewind() = 0;

};

#endif
