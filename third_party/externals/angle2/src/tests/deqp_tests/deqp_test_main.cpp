//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"
#include "deqp_tests.h"

#include <EGL/eglext.h>

#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    typedef std::pair<std::string, EGLNativeDisplayType> NameDisplayTypePair;
    typedef std::map<std::string, EGLNativeDisplayType> DisplayTypeMap;
    DisplayTypeMap allDisplays;
    allDisplays["d3d11"] = EGL_D3D11_ONLY_DISPLAY_ANGLE;

    // Iterate through the command line arguments and check if they are config names
    std::vector<NameDisplayTypePair> requestedDisplays;
    for (size_t i = 1; i < static_cast<size_t>(argc); i++)
    {
        DisplayTypeMap::const_iterator iter = allDisplays.find(argv[i]);
        if (iter != allDisplays.end())
        {
            requestedDisplays.push_back(*iter);
        }
    }

    // If no configs were requested, run them all
    if (requestedDisplays.empty())
    {
        for (DisplayTypeMap::const_iterator i = allDisplays.begin(); i != allDisplays.end(); i++)
        {
            requestedDisplays.push_back(*i);
        }
    }

    // Run each requested config
    int rt = 0;
    for (size_t i = 0; i < requestedDisplays.size(); i++)
    {
        DEQPConfig config;
        config.displayType = requestedDisplays[i].second;
        config.width = 256;
        config.height = 256;
        config.hidden = false;

        SetCurrentConfig(config);

        std::cout << "Running test configuration \"" << requestedDisplays[i].first << "\".\n";

        rt |= RUN_ALL_TESTS();
    }

    return rt;
}
