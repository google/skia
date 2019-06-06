/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "generator.h"
#include "jnigenerator.h"
#include "skia_api_idl.h"

using namespace std;

namespace generator {

// calculate Variable fields: _ifaceP,_deallocIfaceP and deallocFuncP
void API::resolve() {
    map<int, Interface*> ifaces;
    for (auto& iface : _interfaces) {
        ifaces[iface._id] = &iface;
    }

    map<int, Enum*> enums;
    for (auto& jenum : _enums) {
        enums[jenum._id] = &jenum;
    }

    for (auto& iface : _interfaces) {
        for (auto& prop : iface._props) {
            if (prop._type == eEnum) {
                prop._enumP = enums[prop._iface];
            }
        }

        for (auto& func : iface._funcs) {
            for (auto& param : func._params) {
                switch (param._type) {
                    case eInterfaceP:
                        param._ifaceP = ifaces[param._iface];
                        break;
                    case eEnum:
                        param._enumP = enums[param._iface];
                        break;
                    case eByteArray: {
                        Function* getSizeFunc = nullptr;
                        for (auto& func2 : iface._funcs) {
                            if (func2._id == param._getArraySizeFunc) {
                                getSizeFunc = &func2;
                                break;
                            }
                        }
                        param._getArraySizeFuncP = getSizeFunc;
                    } break;
                    default:
                        break;
                }
                switch (param._dir) {
                    case eOutputCustomDealloc: {
                        param._deallocIfaceP = ifaces[param._deallocIface];
                        map<int, Function*> funcs;
                        for (auto& func : param._deallocIfaceP->_funcs) {
                            funcs[func._id] = &func;
                        }
                        param._deallocFuncP = funcs[param._deallocFunc];
                    } break;
                    default:
                        break;
                }
            }
        }
    }
}

}  // namespace generator
