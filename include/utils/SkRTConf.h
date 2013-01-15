/*
 * Copyright 2013 Google, Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkRTConf_DEFINED
#define SkRTConf_DEFINED

#include "SkString.h"
#include "SkStream.h"

#include "SkTDict.h"
#include "SkTArray.h"

/** \class SkRTConfBase
    Non-templated base class for the runtime configs
*/

class SkRTConfBase {
public:
    SkRTConfBase(const char *name) : fName(name) {}
    virtual ~SkRTConfBase() {}
    virtual const char *getName() const { return fName.c_str(); }
    virtual bool isDefault() const = 0;
    virtual void print(SkWStream *o) const = 0;
    virtual bool equals(const SkRTConfBase *conf) const = 0;
protected:
    SkString fName;
};

/** \class SkRTConf
    A class to provide runtime configurability.
*/
template<typename T> class SkRTConf: public SkRTConfBase {
public:
    SkRTConf(const char *name, const T &defaultValue, const char *description);
    operator const T&() const { return fValue; }
    void print(SkWStream *o) const;
    bool equals(const SkRTConfBase *conf) const;
    bool isDefault() const { return fDefault == fValue; }
    void set(const T& value) { fValue = value; }
protected:
    void doPrint(char *s) const;

    T        fValue;
    T        fDefault;
    SkString fDescription;
};

#ifdef SK_DEVELOPER
#define SK_CONF_DECLARE(confType, varName, confName, defaultValue, description) static SkRTConf<confType> varName(confName, defaultValue, description)
#define SK_CONF_SET(confname, value) skRTConfRegistry().set(confname, value)
#else
#define SK_CONF_DECLARE(confType, varName, confName, defaultValue, description) static const confType varName = defaultValue
#define SK_CONF_SET(confname, value) (void) confname, (void) value
#endif

/** \class SkRTConfRegistry
    A class that maintains a systemwide registry of all runtime configuration
    parameters.  Mainly used for printing them out and handling multiply-defined
    knobs.
*/

class SkRTConfRegistry {
public:
    SkRTConfRegistry();
    void printAll(const char *fname = NULL) const;
    void printNonDefault(const char *fname = NULL) const;
    const char *configFileLocation() const;
    void possiblyDumpFile() const;
    void validate() const;
    template <typename T> void set(const char *confname, T value);
private:
    template<typename T> friend class SkRTConf;

    void registerConf(SkRTConfBase *conf);
    template <typename T> bool parse(const char *name, T* value);

    SkTDArray<SkString *> fConfigFileKeys, fConfigFileValues;
    typedef SkTDict< SkTDArray<SkRTConfBase *> * > ConfMap;
    ConfMap fConfs;
};

// our singleton registry

SkRTConfRegistry &skRTConfRegistry();

template<typename T>
SkRTConf<T>::SkRTConf(const char *name, const T &defaultValue, const char *description)
    : SkRTConfBase(name)
    , fValue(defaultValue)
    , fDefault(defaultValue)
    , fDescription(description) {

    T value;
    if (skRTConfRegistry().parse(fName.c_str(), &value)) {
        fValue = value;
    }
    skRTConfRegistry().registerConf(this);
}

template<typename T>
void SkRTConf<T>::print(SkWStream *o) const {
    char outline[200]; // should be ok because we specify a max. width for everything here.

    sprintf(outline, "%-30.30s", getName());
    doPrint(&(outline[30]));
    sprintf(&(outline[60]), " %.128s", fDescription.c_str());
    if (' ' == outline[strlen(outline)-1]) {
        for (int i = strlen(outline)-1 ; ' ' == outline[i] ; i--) {
            outline[i] = '\0';
        }
    }
    o->writeText(outline);
}

template<typename T>
void SkRTConf<T>::doPrint(char *s) const {
    sprintf(s, "%-30.30s", "How do I print myself??");
}

template<> inline void SkRTConf<bool>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%s # [%s]", fValue ? "true" : "false", fDefault ? "true" : "false");
    sprintf(s, "%-30.30s", tmp);
}

template<> inline void SkRTConf<int>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%d # [%d]", fValue, fDefault);
    sprintf(s, "%-30.30s", tmp);
}

template<> inline void SkRTConf<unsigned int>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%u # [%u]", fValue, fDefault);
    sprintf(s, "%-30.30s", tmp);
}

template<> inline void SkRTConf<float>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%6.6f # [%6.6f]", fValue, fDefault);
    sprintf(s, "%-30.30s", tmp);
}

template<> inline void SkRTConf<double>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%6.6f # [%6.6f]", fValue, fDefault);
    sprintf(s, "%-30.30s", tmp);
}

template<> inline void SkRTConf<const char *>::doPrint(char *s) const {
    char tmp[30];
    sprintf(tmp, "%s # [%s]", fValue, fDefault);
    sprintf(s, "%-30.30s", tmp);
}

template<typename T>
bool SkRTConf<T>::equals(const SkRTConfBase *conf) const {
    // static_cast here is okay because there's only one kind of child class.
    const SkRTConf<T> *child_pointer = static_cast<const SkRTConf<T> *>(conf);
    return child_pointer &&
           fName == child_pointer->fName &&
           fDescription == child_pointer->fDescription &&
           fValue == child_pointer->fValue &&
           fDefault == child_pointer->fDefault;
}

#endif
