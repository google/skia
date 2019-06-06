/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "jnigenerator.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

namespace generator {

// generate a string for the parameter type
string genJavaTypeName(const JavaConverter& conv, const Variable& param,
                       map<int, Interface*>& usedIfaces, map<int, Enum*>& usedEnums) {
    switch (param._type) {
        case eInterfaceP: {
            auto paramIface = param._ifaceP;
            string javaClassName = conv._convJava._ifaceNameConverter(paramIface->_name);
            usedIfaces[paramIface->_id] = paramIface;
            return javaClassName;
        } break;
        case eVoid:
            return "void";
        case eInt:
            return "int";
        case eBool:
            return "boolean";
        case eFloat:
            return "float";
        case eByteArray:
            return "byte[]";
        case eIntArray:
            return "int[]";
        case eFloatArray:
            return "float[]";
            break;
        case eEnum: {
            auto jenum = param._enumP;
            string javaClassName = conv._convJava._ifaceNameConverter(jenum->_name);
            usedEnums[jenum->_id] = jenum;
            return javaClassName;
        } break;
            // TODO
    }

    return "";
}

void genEnumJavaClass(const JavaConverter& conv, const Enum& jenum, const char* path) {
    string javaClassName = conv._convJava._ifaceNameConverter(jenum._name);
    string filename = javaClassName + ".java";
    ofstream file(string(path) + "/" + filename);
    if (!file.is_open()) return;

    file << "// This is an automatically generated file. Please do not edit.\n\n";
    file << "package " + conv._packageName + ";\n\n";
    file << "public enum " + javaClassName + " {\n";

    // insert public native functions here
    for (int i = 0; i < jenum._values.size(); i++) {
        bool lastElement = i == jenum._values.size() - 1;
        auto& enumvalue = jenum._values[i];
        file << "    " + enumvalue._name;
        if (lastElement) {
            file << ";\n\n";
        } else {
            file << ",\n";
        }
    }

    // insert boilerplate
    file << "    static {\n";
    file << "        staticInit();\n";
    file << "    }\n\n";
    file << "    static void referenced() {}\n\n";
    file << "    private static native void staticInit();\n\n";
    file << "}\n";

    file.close();
}

void genJavaClass(const JavaConverter& conv, const Interface& iface, const char* path) {
    string javaClassName = conv._convJava._ifaceNameConverter(iface._name);
    string filename = javaClassName + ".java";
    ofstream file(string(path) + "/" + filename);
    if (!file.is_open()) return;
    file << "// This is an automatically generated file. Please do not edit.\n\n";

    map<int, Interface*> usedIfaces;
    map<int, Enum*> usedEnums;

    string pubfunctions;
    pubfunctions.reserve(10000);

    // insert public native functions here
    for (auto& func : iface._funcs) {
        if (!func._exposed) continue;
        string javaFuncName = conv._convJava._funcNameConverter(iface._name, func._name);
        bool isCtor = func._type == eCtor;
        string javaReturnType =
                isCtor ? "void" : genJavaTypeName(conv, func._params[0], usedIfaces, usedEnums);
        if (isCtor) {
            javaFuncName = "nativeInit";
            pubfunctions += "    private ";
        } else {
            pubfunctions += "    public ";
        }
        if (func._type == eStatic) pubfunctions += "static ";
        pubfunctions += "native " + javaReturnType + " " + javaFuncName + "(";
        string funcparams;
        string ctorparamsinvoke;
        for (int i = 1; i < func._params.size(); i++) {
            auto& param = func._params[i];
            if (!param._exposed) continue;
            if (i != 1) {
                funcparams += ", ";
                ctorparamsinvoke += ", ";
            }
            funcparams += genJavaTypeName(conv, param, usedIfaces, usedEnums);
            if (!param._name.empty()) {
                funcparams += " " + param._name;
                ctorparamsinvoke += " " + param._name;
            } else {
                funcparams += string(" p") + to_string(i);
                ctorparamsinvoke += string(" p") + to_string(i);
            }
        }
        pubfunctions += funcparams;
        pubfunctions += ");\n\n";

        if (isCtor) {
            pubfunctions += "    public " + javaClassName + "(" + funcparams + ") {\n";
            pubfunctions += "        nativeInit(" + ctorparamsinvoke + ");\n";
            pubfunctions += "    }\n\n";
        }
    }

    // insert ctor if there are properties defined
    if (!iface._props.empty()) {
        pubfunctions += "    public " + javaClassName + "(";
        for (int i = 0; i < iface._props.size(); i++) {
            auto& prop = iface._props[i];
            string javaReturnType = genJavaTypeName(conv, prop, usedIfaces, usedEnums);
            if (i != 0) pubfunctions += ", ";
            pubfunctions += javaReturnType + " " + prop._name;
        }
        pubfunctions += ") {\n";
        pubfunctions += "        this();\n";
        for (auto& prop : iface._props) {
            string javaReturnType = genJavaTypeName(conv, prop, usedIfaces, usedEnums);
            pubfunctions += "        this." + prop._name + " = " + prop._name + ";\n";
        }
        pubfunctions += "    }\n\n";
    }

    // insert properties
    for (auto& prop : iface._props) {
        string javaReturnType = genJavaTypeName(conv, prop, usedIfaces, usedEnums);
        pubfunctions += "    public " + javaReturnType + " " + prop._name + ";\n\n";
    }

    file << "package " + conv._packageName + ";\n\n";
    for (auto& iface : usedIfaces) {
        string javaClassName = conv._convJava._ifaceNameConverter(iface.second->_name);
        file << "import " + conv._packageName + "." + javaClassName + ";\n";
    }
    for (auto& jenum : usedEnums) {
        string javaClassName = conv._convJava._ifaceNameConverter(jenum.second->_name);
        file << "import " + conv._packageName + "." + javaClassName + ";\n";
    }

    file << "\npublic class " + javaClassName + " {\n";

    file << pubfunctions;

    // insert boilerplate
    file << "    @Override\n";
    file << "    protected void finalize() throws Throwable {\n";
    file << "        try {\n";
    file << "          nativeFinalize();\n";
    file << "        } finally {\n";
    file << "          super.finalize();\n";
    file << "        }\n";
    file << "    }\n\n";
    file << "    static {\n";
    for (auto& iface : usedIfaces) {
        string javaClassName = conv._convJava._ifaceNameConverter(iface.second->_name);
        file << "        " + javaClassName + ".referenced();\n";
    }
    for (auto& jenum : usedEnums) {
        string javaClassName = conv._convJava._ifaceNameConverter(jenum.second->_name);
        file << "        " + javaClassName + ".referenced();\n";
    }
    file << "        staticInit();\n";
    file << "    }\n\n";
    file << "    static void referenced() {}\n\n";
    file << "    private " + javaClassName + "(boolean invokeFromNative) {}\n\n";
    file << "    private static native void staticInit();\n\n";
    file << "    private native void nativeFinalize();\n\n";
    file << "    private long _this;\n";
    file << "}\n";

    file.close();
}

// generate a string for C parameter type
string genJNICTypeName(const JavaConverter& conv, const Variable& param) {
    switch (param._type) {
        case eVoid:
            return "void";
        case eInterfaceP:
            return "jobject";
        case eInt:
            return "jint";
        case eBool:
            return "jboolean";
        case eFloat:
            return "jfloat";
        case eByteArray:
            return "jbyteArray";
        case eIntArray:
            return "jintArray";
        case eFloatArray:
            return "jfloatArray";
        case eEnum:
            return "jobject";
            // TODO
        default:
            break;
    }

    return "";
}

// generate a string for JNI signature  type
string genJNISignTypeName(const JavaConverter& conv, const Variable& param) {
    switch (param._type) {
        case eVoid:
            return "V";
        case eInt:
            return "I";
        case eBool:
            return "Z";
        case eFloat:
            return "F";
        case eFloatArray:
            return "[F";
        case eIntArray:
            return "[I";
        case eEnum: {
            string jniSignature = conv._packageName + "." +
                                  conv._convJava._ifaceNameConverter(param._enumP->_name);
            replace(jniSignature.begin(), jniSignature.end(), '.', '/');
            return string("L") + jniSignature + ";";
        }
            // TODO
        default:
            break;
    }
    return "";
}

// generate a string for JNI get method
string genJNIGetMethodName(const Variable& param) {
    switch (param._type) {
        case eVoid:
            return "V";
        case eInt:
            return "GetIntField";
        case eBool:
            return "GetBooleanField";
        case eFloat:
            return "GetFloatField";
        case eFloatArray:
            return "GetObjectField";
        case eEnum:
            return "GetObjectField";
            // TODO
        default:
            break;
    }
    return "";
}

void genJNI(const JavaConverter& conv, const char* path) {
    string filename = conv._packageName + "_jni.cpp";
    ofstream file(string(path) + "/" + filename);
    if (!file.is_open()) return;
    file << "// This is an automatically generated file. Please do not edit.\n\n";

    file << "#include <jni.h>\n";
    file << "#include \"Binder.h\"\n";
    file << "#include \"EnumHelper.h\"\n";
    file << "#include <memory>\n\n";

    file << conv._headerFiles;
    file << "\n";

    // generate Binders
    for (auto& iface : conv._api._interfaces) {
        string CClassName = conv._convC._ifaceNameConverter(iface._name);
        file << "static Binder<" + CClassName + "> s" + iface._name + "Binder;\n";
    }
    // generate enum helpers
    for (auto& jenum : conv._api._enums) {
        string CClassName = conv._convC._ifaceNameConverter(jenum._name);
        file << "static EnumHelper<" + CClassName + "> s" + jenum._name + "Enum;\n";
    }
    file << "\n";

    string JNIFunctionPrefix = "Java_" + conv._packageName + "_";
    std::replace(JNIFunctionPrefix.begin(), JNIFunctionPrefix.end(), '.', '_');

    // generate enum static inits
    for (auto& jenum : conv._api._enums) {
        string javaClassName = conv._convJava._ifaceNameConverter(jenum._name);
        file << "extern \"C\" JNIEXPORT void JNICALL " + JNIFunctionPrefix + javaClassName +
                        "_staticInit(JNIEnv* env, jclass clazz) {\n";
        file << "    s" + jenum._name + "Enum.init(env, clazz);\n";
        file << "}\n\n";
    }

    for (auto& iface : conv._api._interfaces) {
        string CClassName = conv._convC._ifaceNameConverter(iface._name);
        string javaClassName = conv._convJava._ifaceNameConverter(iface._name);

        file << "extern \"C\" JNIEXPORT void JNICALL " + JNIFunctionPrefix + javaClassName +
                        "_staticInit(JNIEnv* env, jclass clazz) {\n";
        file << "    s" + iface._name + "Binder.init(env, clazz);\n";
        file << "}\n\n";

        file << "extern \"C\" JNIEXPORT void JNICALL " + JNIFunctionPrefix + javaClassName +
                        "_nativeFinalize(JNIEnv* env, jobject clazz) {\n";
        file << "    s" + iface._name + "Binder.unbind(env, clazz);\n";
        file << "}\n\n";

        for (auto& func : iface._funcs) {
            if (!func._exposed) continue;
            bool isCtor = func._type == eCtor;
            string javaFuncName =
                    isCtor ? "nativeInit"
                           : conv._convJava._funcNameConverter(iface._name, func._name);
            string CFuncName = conv._convC._funcNameConverter(iface._name, func._name);
            if (isCtor && func._name.empty()) {
                CFuncName = "new " + conv._convC._ifaceNameConverter(iface._name);
            }
            string JNIReturnType = isCtor ? "void" : genJNICTypeName(conv, func._params[0]);
            file << "extern \"C\" JNIEXPORT " + JNIReturnType + " JNICALL " + JNIFunctionPrefix +
                            javaClassName + "_" + javaFuncName + "(JNIEnv* env, ";
            if (func._type == eStatic)
                file << "jclass clazz";
            else
                file << "jobject thiz";

            string inparams_binders;
            string inparams_invoke;
            string returnCTypeName;
            if (func._params[0]._type == eInterfaceP) {
                returnCTypeName = conv._convC._ifaceNameConverter(func._params[0]._ifaceP->_name);
            }

            if (func._params[0]._type == eEnum) {
                // returnCTypeName =
                // conv._convC._ifaceNameConverter(func._params[0]._ifaceP->_name);
                // TODO
            }

            if (func._type == eNormal) {
                inparams_invoke = "cppThiz.get()";
            }

            for (int i = 1; i < func._params.size(); i++) {
                auto& param = func._params[i];
                if (param._exposed) {
                    file << ", " + genJNICTypeName(conv, param) + " ";
                }
                string param_name;
                if (param._name.empty()) {
                    param_name = "p" + to_string(i);
                } else {
                    param_name = param._name;
                }
                if (param._exposed) {
                    file << param_name;
                }

                if (!inparams_invoke.empty()) inparams_invoke += ", ";
                switch (param._type) {
                    case eInterfaceP: {
                        string paramCTypeName =
                                conv._convC._ifaceNameConverter(param._ifaceP->_name);
                        inparams_binders += "    std::shared_ptr<" + paramCTypeName + "> bp" +
                                            to_string(i) + " = s" + param._ifaceP->_name +
                                            "Binder.getCppObject(env, " + param_name + ");\n";
                        // copy the property values from java to C++ structs
                        if (!param._ifaceP->_props.empty()) {
                            inparams_binders += "    if (bp" + to_string(i) + ".get()) {\n";
                            string props_name = string("props") + to_string(i);
                            inparams_binders += "        auto& " + props_name + " = s" +
                                                param._ifaceP->_name + "Binder.properties();\n";
                            inparams_binders += "        if (" + props_name + ".empty()) {\n";
                            for (int j = 0; j < param._ifaceP->_props.size(); j++) {
                                auto& prop = param._ifaceP->_props[j];
                                inparams_binders +=
                                        "            " + props_name +
                                        ".push_back(env->GetFieldID(s" + param._ifaceP->_name +
                                        "Binder.getJavaClass(), \"" + prop._name + "\", \"" +
                                        genJNISignTypeName(conv, prop) + "\" ));\n";
                            }
                            inparams_binders += "        }\n";
                            for (int j = 0; j < param._ifaceP->_props.size(); j++) {
                                auto& prop = param._ifaceP->_props[j];
                                if (prop._type == eFloatArray) {
                                    inparams_binders += "        {\n";
                                    inparams_binders +=
                                            "            int proplen" + to_string(j) + ";\n";
                                    inparams_binders += "            auto proparr" + to_string(j) +
                                                        " = JNIHelper::as_float_array2(env, "
                                                        "(jfloatArray)env->GetObjectField(" +
                                                        param_name + ", " + props_name + "[" +
                                                        to_string(j) + "]), proplen" +
                                                        to_string(j) + ");\n";
                                    inparams_binders += "            memcpy(bp" + to_string(i) +
                                                        "->" + prop._name + ", proparr" +
                                                        to_string(j) + ".get(), proplen" +
                                                        to_string(j) + "*sizeof(float));\n";
                                    inparams_binders += "        }\n";
                                } else if (prop._type == eEnum) {
                                    inparams_binders += "        bp" + to_string(i) + "->" +
                                                        prop._name + " = s" + prop._enumP->_name +
                                                        "Enum.getCppValue(env, env->" +
                                                        genJNIGetMethodName(prop) + "(" +
                                                        param_name + ", " + props_name + "[" +
                                                        to_string(j) + "]));\n";
                                } else {
                                    inparams_binders += "        bp" + to_string(i) + "->" +
                                                        prop._name + " =  env->" +
                                                        genJNIGetMethodName(prop) + "(" +
                                                        param_name + ", " + props_name + "[" +
                                                        to_string(j) + "]);\n";
                                }
                            }
                            inparams_binders += "    }\n";
                        }
                        inparams_invoke += "bp" + to_string(i) + ".get()";
                    } break;
                    case eEnum: {
                        string paramCTypeName =
                                conv._convC._ifaceNameConverter(param._enumP->_name);
                        inparams_binders += "    " + paramCTypeName + " bp" + to_string(i) +
                                            " = s" + param._enumP->_name +
                                            "Enum.getCppValue(env, " + param_name + ");\n";
                        inparams_invoke += "bp" + to_string(i);
                    } break;
                    case eInt:
                        inparams_invoke += "(int)" + param_name;
                        break;
                    case eBool:
                        inparams_invoke += "JNI_TRUE == " + param_name;
                        break;
                    case eFloat:
                        inparams_invoke += "(float)" + param_name;
                        break;
                    case eByteArray: {
                        // TODO: this works only if array param precedes length param in parameters
                        // list. This happens to be true for Skia API, but it is easy to fix if
                        // needed.
                        string lenVariable = param._countVarName;
                        if (lenVariable.empty()) {
                            lenVariable = "binarlen" + to_string(i);
                        }
                        inparams_binders += string("    int " + lenVariable + ";\n    auto binar") +
                                            to_string(i) +
                                            " = JNIHelper::as_unsigned_char_array2(env, " +
                                            param_name + ", " + lenVariable + " );\n";
                        inparams_invoke += "binar" + to_string(i) + ".get()";
                    } break;
                    case eIntArray: {
                        string lenVariable = param._countVarName;
                        if (lenVariable.empty()) {
                            lenVariable = "binarlen" + to_string(i);
                        }
                        inparams_binders += string("    int " + lenVariable + ";\n    auto binar") +
                                            to_string(i) +
                                            " = JNIHelper::as_unsigned_int_array2(env, " +
                                            param_name + ", " + lenVariable + " );\n";
                        inparams_invoke += "binar" + to_string(i) + ".get()";
                    } break;
                    case eFloatArray: {
                        string lenVariable = param._countVarName;
                        if (lenVariable.empty()) {
                            lenVariable = "binarlen" + to_string(i);
                        }
                        inparams_binders += string("    int " + lenVariable + ";\n    auto binar") +
                                            to_string(i) + " = JNIHelper::as_float_array2(env, " +
                                            param_name + ", " + lenVariable + " );\n";
                        inparams_invoke += "binar" + to_string(i) + ".get()";
                    } break;
                    default:
                        break;
                }
            }
            file << ") {\n";

            if (func._type == eNormal) {
                file << "    auto cppThiz = s" + iface._name + "Binder.getCppObject(env, thiz);\n";
            }
            file << inparams_binders;

            bool hasReturnValue = func._params[0]._type != eVoid;
            file << "    ";
            if (hasReturnValue) {
                if (func._params[0]._dir == eOutputByValue) {
                    file << returnCTypeName + "* rawResult = new " + returnCTypeName + "();\n";
                    file << "    *rawResult = ";
                } else {
                    file << "auto rawResult = ";
                }
            }
            file << CFuncName + "(";
            file << inparams_invoke;
            file << ");\n";
            if (hasReturnValue || isCtor) {
                switch (func._params[0]._type) {
                    case eInterfaceP: {
                        file << "    std::shared_ptr<" + returnCTypeName + "> result(rawResult";
                        switch (func._params[0]._dir) {
                            case eOutputNoDealloc:
                                file << ", [](" + returnCTypeName + "* p) { }";
                                break;
                            case eOutputCustomDealloc: {
                                string DeallocCFuncName = conv._convC._funcNameConverter(
                                        func._params[0]._deallocIfaceP->_name,
                                        func._params[0]._deallocFuncP->_name);
                                file << ", [](" + returnCTypeName + "* p) { " + DeallocCFuncName +
                                                "(p); }";
                            } break;
                            case eOutputDelete:
                                break;
                            case eOutputByValue:
                                break;
                            default:
                                break;
                        }

                        file << ");\n";

                        if (isCtor) {
                            file << "    s" + iface._name + "Binder.bind(env, result, thiz);\n";
                        } else {
                            file << "    return s" + func._params[0]._ifaceP->_name +
                                            "Binder.getJavaObject(env, result);\n";
                        }
                    } break;
                    case eEnum:
                        file << "    return s" + func._params[0]._enumP->_name +
                                        "Enum.getJavaValue(env, rawResult);\n";
                        break;
                    case eBool:
                        file << "    return rawResult ? JNI_TRUE : JNI_FALSE;\n";
                        break;
                    case eByteArray: {
                        auto getSizeFunc = func._params[0]._getArraySizeFuncP;
                        string CFuncName2 =
                                conv._convC._funcNameConverter(iface._name, getSizeFunc->_name);
                        file << "    auto dataSize = " + CFuncName2 + "(" + inparams_invoke +
                                        ");\n";
                        file << "    jbyteArray result = env->NewByteArray(dataSize);\n";
                        file << "    env->SetByteArrayRegion(result, 0, dataSize, (const "
                                "jbyte*)rawResult);\n";
                        file << "    return result;\n";
                    } break;
                    case eFloatArray:
                        // TODO
                        break;
                    case eIntArray:
                        // TODO
                        break;
                    default:
                        file << "    return rawResult;\n";  // TODO: maybe cast
                        break;
                }
            }

            file << "}\n\n";
        }
    }

    file.close();
}

void genJavaAPI(JavaConverter& conv, const char* path) {
    conv._api.resolve();
    for (auto& jenum : conv._api._enums) {
        genEnumJavaClass(conv, jenum, path);
    }
    for (auto& iface : conv._api._interfaces) {
        genJavaClass(conv, iface, path);
    }
    genJNI(conv, path);
}

}  // namespace generator