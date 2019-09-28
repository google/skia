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

#include "gm/gm.h"

int main() {
    std::vector<std::string> gms;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(factory());
        gms.push_back(std::string(gm->getName()));
    }
    std::sort(gms.begin(), gms.end());
    for (const std::string& gm : gms) {
        std::cout << gm << '\n';
    }
}
