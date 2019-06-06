/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TESTBED_GENERATOR_H234432423
#define TESTBED_GENERATOR_H234432423

#include <functional>
#include <string>
#include <vector>

namespace generator {

enum VariableType {
    eVoid,
    eInt,
    eBool,
    eFloat,
    eByteArray,
    eIntArray,
    eFloatArray,
    eEnum,
    eInterfaceP  // returns a pointer to interface
};

enum Direction { eInput, eOutputNoDealloc, eOutputDelete, eOutputCustomDealloc, eOutputByValue };

enum FuncType { eNormal, eCtor, eStatic };

struct Interface;
struct Function;
struct Enum;

struct Variable {
    std::string _name;
    VariableType _type;
    Direction _dir;
    int _iface = -1;  // id of interface or enum in case of eInterfaceP or eEnum
    int _deallocIface = -1;
    int _deallocFunc = -1;
    int _getArraySizeFunc =
            -1;  // id of a function in the same interface to resolve array size of eByteArray
    std::string _countVarName;  // when _type is eByteArray, this optionally holds the name of
                                // another variable that has the array length
    bool _exposed =
            true;  // some variables may be not be exposed (used only to to store array count)
    int _fixedArraySize = -1;  // this is set with the length for fixed sized arrays (eByteArray,
                               // eIntArray, eFloatArray)

    // fields left empty and initialized latter by API::resolve()
    Interface* _ifaceP = nullptr;
    Interface* _deallocIfaceP = nullptr;
    Function* _deallocFuncP = nullptr;
    Enum* _enumP = nullptr;
    Function* _getArraySizeFuncP = nullptr;
};

struct Function {
    int _id;
    std::string _name;
    FuncType _type;
    std::vector<Variable> _params;
    bool _exposed = true;  // some function may be not be exposed (used only for ref counting)
};

struct Interface {
    int _id;
    std::string _name;
    std::vector<Function> _funcs;
    std::vector<Variable> _props;
};

struct EnumValue {
    std::string _name;
    // int _value; //-> value of the enum if not default
};

struct Enum {
    int _id;
    std::string _name;
    std::vector<EnumValue> _values;
};

struct API {
    std::vector<Interface> _interfaces;
    std::vector<Enum> _enums;

    void resolve();
};

struct NameConverter {
    std::function<std::string(std::string)> _ifaceNameConverter;
    std::function<std::string(std::string, std::string)> _funcNameConverter;
};

struct JavaConverter {
    std::string _packageName;
    NameConverter _convC;
    NameConverter _convJava;
    API _api;
    std::string _headerFiles;
};

}  // namespace generator

#endif  // TESTBED_GENERATOR_H234432423
