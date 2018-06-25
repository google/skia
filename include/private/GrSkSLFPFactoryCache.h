/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"

#include <vector>

class GrSkSLFPFactory;

class GrSkSLFPFactoryCache : public SkNVRefCnt<GrSkSLFPFactoryCache> {
public:
    sk_sp<GrSkSLFPFactory> get(int index);

    void set(int index, sk_sp<GrSkSLFPFactory> factory);

    ~GrSkSLFPFactoryCache();

private:
    std::vector<GrSkSLFPFactory*> fFactories;
};
