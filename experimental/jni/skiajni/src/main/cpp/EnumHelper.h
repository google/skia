/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ENUMHELPER_H_4324324
#define ENUMHELPER_H_4324324

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

template <class CPPCLASS> class EnumHelper {
    jclass _javaClass;                     // cached copy of the class object
    jmethodID _javaOrdinal;                // java ordinal function
    std::vector<jobject> _javaEnumValues;  // an array with all possible java Enum objects

public:
    void init(JNIEnv* env, jclass javaClass) {
        _javaClass = (jclass)env->NewGlobalRef((jobject)javaClass);
        if (NULL == _javaClass) {
            // Log::error( THIS_FUNCTION, "failed to initialize" );
        } else {
            _javaOrdinal = env->GetMethodID(_javaClass, "ordinal", "()I");

            // Now get the class object's class descriptor
            jclass cls = env->GetObjectClass(javaClass);

            // Find the getName() method on the class object
            jmethodID mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");

            // Call the getName() to get a jstring object back
            jstring strObj = (jstring)env->CallObjectMethod(javaClass, mid);

            // Now get the c string from the java jstring object
            const char* str = env->GetStringUTFChars(strObj, NULL);
            std::string jniSignature = "()[L" + std::string(str) + ";";
            // Release the memory pinned char array
            env->ReleaseStringUTFChars(strObj, str);

            // Get the Enum.values() static method
            std::replace(jniSignature.begin(), jniSignature.end(), '.', '/');
            jmethodID javaGetValues =
                    env->GetStaticMethodID(_javaClass, "values", jniSignature.c_str());

            jobjectArray data =
                    (jobjectArray)env->CallStaticObjectMethod(_javaClass, javaGetValues);
            jsize length = env->GetArrayLength(data);
            for (int i = 0; i < length; i++) {
                jobject element = (jobject)env->GetObjectArrayElement(data, i);
                _javaEnumValues.push_back(env->NewGlobalRef(element));
            }
        }
    }

    CPPCLASS getCppValue(JNIEnv* env, jobject javaObject) {
        return (CPPCLASS)env->CallIntMethod(javaObject, _javaOrdinal);
    }

    jobject getJavaValue(JNIEnv* env, CPPCLASS cppObject) {
        int index = (int)cppObject;
        if (index < _javaEnumValues.size()) {
            return _javaEnumValues[index];
        }
        return NULL;
    }
};

#endif  // ENUMHELPER_H_4324324
