
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderer.h"


class GrAndroidPathRenderer : public GrPathRenderer {
public:
    GrAndroidPathRenderer();

    virtual bool canDrawPath(const SkPath& path,
                             const SkStrokeRec& stroke,
                             const GrDrawTarget* target,
                             bool antiAlias) const override;

protected:
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
                            bool antiAlias) override;

private:
    typedef GrPathRenderer INHERITED;
};
