/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "gm.h"

int main() {
    std::vector<std::string> gms;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        std::unique_ptr<skiagm::GM> gm(r->factory()(nullptr));
        gms.push_back(std::string(gm->getName()));
    }
    std::sort(gms.begin(), gms.end());
    for (const std::string& gm : gms) {
        std::cout << gm << '\n';
    }
}
