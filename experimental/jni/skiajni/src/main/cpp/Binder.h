/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BINDERJ_H
#define BINDERJ_H

#include <jni.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// class that connects an instance of Java object with an instance of C++ object
// the C++ object is referenced with a std::shared_ptr and the Java object is referenced with weak
// Java reference
template <class CPPCLASS> class Binder {
private:
    jfieldID _fieldId;  // Id of the member variable that holds the C++ "this" pointer bound to the
                        // Java object
    // the java variable must be named "_this" and the type has to be "long". It contains a raw
    // pointer to "JavaStruct" defined below
    jfieldID _genericClassFieldId;  // Id of "_genericClass" member variable that holds a string
                                    // with the type parameter of the java generic
    jclass _javaClass;              // cached copy of the class object
    // TODO: make destructor to release _javaClass
    jmethodID _javaConstructor;         // java object constructor
    bool _isGeneric;                    // true if the java class is generic
    std::string _javaTypeClass;         // the string representation of the Java generic argument
    std::vector<jfieldID> _properties;  // cache for list of properties

    std::recursive_mutex _mutex;  // serialize all function calls

    // a C++ raw pointer to JavaStruct is embedded in the java objects in "_this" member variable
    struct JavaStruct {
        std::shared_ptr<CPPCLASS> _cppObject;
        jobject _javaWeakPointer;

        JavaStruct(JNIEnv* env, const std::shared_ptr<CPPCLASS>& cppObject, jobject javaObject)
                : _cppObject(cppObject) {
            _javaWeakPointer = env->NewWeakGlobalRef(javaObject);
        }
        void release(JNIEnv* env) {
            if (NULL != _javaWeakPointer) {
                env->DeleteWeakGlobalRef(_javaWeakPointer);
                _javaWeakPointer = NULL;
            }
            _cppObject.reset();
        }
    };
    std::map<void*, JavaStruct*> _cpp_to_java;  // the key is a C++ object raw pointer.

    bool _allowMultipleJavaClasses;  // true if we allow one C++ class "CPPCLASS" to bound to
                                     // multiple Java classes.
    // it can be used to bind anonymous java objects that implement a listener interface, but
    // otherwise avoid it

    bool getClassAndField(JNIEnv* env, jobject javaObject, jclass& javaClass, jfieldID& fieldId) {
        javaClass = env->GetObjectClass(javaObject);
        if (0 == javaClass) {
            return false;
        } else {
            // Look for the instance field in java class
            fieldId = env->GetFieldID(javaClass, "_this", "J");
            if (0 == fieldId) {
                return false;
            }
        }
        return true;
    }

public:
    Binder(bool allowMultipleJavaClasses = false)
            : _fieldId(0)
            , _genericClassFieldId(0)
            , _javaClass(NULL)
            , _javaConstructor(NULL)
            , _isGeneric(false)
            , _allowMultipleJavaClasses(allowMultipleJavaClasses) {}

    ~Binder() {
        // if (NULL != _javaClass) {
        //  env->DeleteGlobalRef( (jobject) _javaClass );
        //  _javaClass = NULL;
        //}
    }

    // binds an instance of C++ object (pass in cppObject parameter) with an instance of
    // Java object (passed in javaObject parameter)
    void bind(JNIEnv* env, const std::shared_ptr<CPPCLASS>& cppObject, jobject javaObject) {
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        jfieldID fieldId;
        if (_allowMultipleJavaClasses) {
            jclass javaClass;
            if (!getClassAndField(env, javaObject, javaClass, fieldId)) {
                return;
            }
        } else {
            fieldId = _fieldId;
        }
        JavaStruct* javaStruct = new JavaStruct(env, cppObject, javaObject);
        env->SetLongField(javaObject, fieldId, (jlong)javaStruct);
        _cpp_to_java[cppObject.get()] = javaStruct;
    }

    // unbinds an instance of C++ and Java objects, that were previously bound with "bind"
    void unbind(JNIEnv* env, jobject javaObject) {
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        jfieldID fieldId;
        if (_allowMultipleJavaClasses) {
            jclass javaClass;
            if (!getClassAndField(env, javaObject, javaClass, fieldId)) {
                return;
            }
        } else {
            fieldId = _fieldId;
        }

        // Read the instance field from the java object
        JavaStruct* javaStruct = (JavaStruct*)env->GetLongField(javaObject, fieldId);
        env->SetLongField(javaObject, fieldId, (jlong)0);
        if (0 != javaStruct) {
            auto it = _cpp_to_java.find((void*)(javaStruct->_cppObject.get()));
            if (_cpp_to_java.end() != it && it->second == javaStruct) {
                _cpp_to_java.erase(it);
            }
            javaStruct->release(env);
            delete javaStruct;
        }
    }

    // finds a C++ object that is bound to a java object "javaObject"
    std::shared_ptr<CPPCLASS> getCppObject(JNIEnv* env, jobject javaObject) {
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        jfieldID fieldId;
        std::shared_ptr<CPPCLASS> res;
        if (!javaObject) {
            return res;
        }
        if (_allowMultipleJavaClasses) {
            jclass javaClass;
            if (!getClassAndField(env, javaObject, javaClass, fieldId)) {
                return res;
            }
        } else {
            fieldId = _fieldId;
        }
        JavaStruct* javaStruct = 0;
        // Read the instance field from the java object
        javaStruct = (JavaStruct*)env->GetLongField(javaObject, fieldId);
        if (0 != javaStruct) {
            res = javaStruct->_cppObject;
        }
        return res;
    }

    // finds a Java object that is bound to a C++ object "cppObject"
    // if bCreate is true, the java object will be automatically created if needed
    // return NULL if java object is not found or cppObject is null
    jobject getJavaObject(JNIEnv* env, const std::shared_ptr<CPPCLASS>& cppObject,
                          bool bCreate = true) {
        if (!cppObject) return NULL;
        jobject javaObject = NULL;
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        auto it = _cpp_to_java.find((void*)cppObject.get());
        if (_cpp_to_java.end() == it) {
            if (!bCreate) {
                return NULL;
            }
            javaObject = NULL;
        } else {
            javaObject = env->NewLocalRef(it->second->_javaWeakPointer);
            // check for dead weak pointer
            if (NULL == javaObject) {
                it->second->release(env);
                _cpp_to_java.erase(
                        it);  // this should not really happen, as the java finalize should erase it
            }
        }
        if (NULL == javaObject && bCreate) {
            if (NULL == _javaConstructor && NULL != _javaClass) {
                _javaConstructor = env->GetMethodID(_javaClass, "<init>", "(Z)V");
                //_javaConstructor = env->GetMethodID(_javaClass, "<init>", "()V");
            }
            if (NULL != _javaConstructor) {
                javaObject = env->NewObject(_javaClass, _javaConstructor, JNI_TRUE);
                // javaObject = env->NewObject(_javaClass, _javaConstructor);
                if (NULL != javaObject) {
                    bind(env, cppObject, javaObject);
                }
                if (_isGeneric) {
                    // set the Java generic field, because reflection does not work
                    // if the generic class was created from JNI
                    jstring genericClass = env->NewStringUTF(_javaTypeClass.c_str());
                    env->SetObjectField(javaObject, _genericClassFieldId, genericClass);
                }
            }
        }
        return javaObject;
    }

    jclass getJavaClass() { return _javaClass; }

    // initialize the binding class. needs the java class object
    // usually this is called from a java static initializer
    // this function does not need to be invoker if _allowMultipleJavaClasses=true
    void init(JNIEnv* env, jclass javaClass, bool resolveThis = true) {
        std::lock_guard<std::recursive_mutex> guard(_mutex);
        _javaClass = (jclass)env->NewGlobalRef((jobject)javaClass);
        if (NULL == _javaClass) {
        } else if (resolveThis) {
            // Look for the instance field in java class
            _fieldId = env->GetFieldID(_javaClass, "_this", "J");
            if (0 == _fieldId) {
                return;  // failed to find the field
            }
        }
    }

    // used for template class instances only
    void init(jclass javaClass, jfieldID fieldId, jfieldID genericClassFieldId,
              const std::string& javaTypeClass) {
        _isGeneric = true;
        _javaClass = javaClass;
        _fieldId = fieldId;
        _genericClassFieldId = genericClassFieldId;
        _javaTypeClass = javaTypeClass;
    }

    std::vector<jfieldID>& properties() { return _properties; }
};

class JNIHelper {
public:
    // convert jbyteArray into unsigned char*. new memory allocated with malloc.
    static unsigned char* as_unsigned_char_array(JNIEnv* env, jbyteArray array, int& len) {
        len = env->GetArrayLength(array);
        if (!len) return nullptr;
        unsigned char* buf = (unsigned char*)malloc(len);
        env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
        return buf;
    }

    // Convert jbyteArray into unsigned char*. memory may or may be copied, deallocation done by
    // shared_ptr.
    static std::shared_ptr<unsigned char> as_unsigned_char_array2(JNIEnv* env, jbyteArray array,
                                                                  int& len) {
        len = env->GetArrayLength(array);
        if (!len) return std::shared_ptr<unsigned char>();
        std::shared_ptr<unsigned char> result(
                (unsigned char*)env->GetByteArrayElements(array, NULL), [=](unsigned char* p) {
                    env->ReleaseByteArrayElements(array, (jbyte*)p, JNI_ABORT);
                });
        return result;
    }

    // Convert jintArray into unsigned int*. memory may or may be copied, deallocation done by
    // shared_ptr.
    static std::shared_ptr<unsigned int> as_unsigned_int_array2(JNIEnv* env, jintArray array,
                                                                int& len) {
        len = env->GetArrayLength(array);
        if (!len) return std::shared_ptr<unsigned int>();
        std::shared_ptr<unsigned int> result(
                (unsigned int*)env->GetIntArrayElements(array, NULL),
                [=](unsigned int* p) { env->ReleaseIntArrayElements(array, (jint*)p, JNI_ABORT); });
        return result;
    }

    // Convert jfloatArray into unsigned float*. memory may or may be copied, deallocation done by
    // shared_ptr.
    static std::shared_ptr<float> as_float_array2(JNIEnv* env, jfloatArray array, int& len) {
        len = env->GetArrayLength(array);
        if (!len) return std::shared_ptr<float>();
        std::shared_ptr<float> result(
                (float*)env->GetFloatArrayElements(array, NULL),
                [=](float* p) { env->ReleaseFloatArrayElements(array, (jfloat*)p, JNI_ABORT); });
        return result;
    }
};

#endif  // BINDERJ_H
