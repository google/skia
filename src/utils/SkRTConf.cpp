/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTConf.h"
#include "SkOSFile.h"

SkRTConfRegistry::SkRTConfRegistry(): fConfs(100) {

    SkFILE *fp = sk_fopen(configFileLocation(), kRead_SkFILE_Flag);

    if (!fp) {
        return;
    }

    char line[1024];

    while (!sk_feof(fp)) {

        if (!sk_fgets(line, sizeof(line), fp)) {
            break;
        }

        char *commentptr = strchr(line, '#');
        if (commentptr == line) {
            continue;
        }
        if (NULL != commentptr) {
            *commentptr = '\0';
        }

        char sep[] = " \t\r\n";

        char *keyptr = strtok(line, sep);
        if (!keyptr) {
            continue;
        }

        char *valptr = strtok(NULL, sep);
        if (!valptr) {
            continue;
        }

        SkString* key = new SkString(keyptr);
        SkString* val = new SkString(valptr);

        fConfigFileKeys.append(1, &key);
        fConfigFileValues.append(1, &val);
    }
    sk_fclose(fp);
}

const char *SkRTConfRegistry::configFileLocation() const {
    return "skia.conf"; // for now -- should probably do something fancier like home directories or whatever.
}

// dump all known runtime config options to the file with their default values.
// to trigger this, make a config file of zero size.
void SkRTConfRegistry::possiblyDumpFile() const {
    const char *path = configFileLocation();
    SkFILE *fp = sk_fopen(path, kRead_SkFILE_Flag);
    if (!fp) {
        return;
    }
    size_t configFileSize = sk_fgetsize(fp);
    if (configFileSize == 0) {
        printAll(path);
    }
    sk_fclose(fp);
}

// Run through every provided configuration option and print a warning if the user hasn't
// declared a correponding configuration object somewhere.
void SkRTConfRegistry::validate() const {
    for (int i = 0 ; i < fConfigFileKeys.count() ; i++) {
        if (!fConfs.find(fConfigFileKeys[i]->c_str())) {
            SkDebugf("WARNING: You have config value %s in your configuration file, but I've never heard of that.\n", fConfigFileKeys[i]->c_str());
        }
    }
}

void SkRTConfRegistry::printAll(const char *fname) const {
    SkWStream *o;

    if (NULL != fname) {
        o = new SkFILEWStream(fname);
    } else {
        o = new SkDebugWStream();
    }

    ConfMap::Iter iter(fConfs);
    SkTDArray<SkRTConfBase *> *confArray;

    while (iter.next(&confArray)) {
        if (confArray->getAt(0)->isDefault()) {
            o->writeText("# ");
        }
        confArray->getAt(0)->print(o);
        o->newline();
    }

    delete o;
}

void SkRTConfRegistry::printNonDefault(const char *fname) const {
    SkWStream *o;

    if (NULL != fname) {
        o = new SkFILEWStream(fname);
    } else {
        o = new SkDebugWStream();
    }
    ConfMap::Iter iter(fConfs);
    SkTDArray<SkRTConfBase *> *confArray;

    while (iter.next(&confArray)) {
        if (!confArray->getAt(0)->isDefault()) {
            confArray->getAt(0)->print(o);
            o->newline();
        }
    }

    delete o;
}

// register a configuration variable after its value has been set by the parser.
// we maintain a vector of these things instead of just a single one because the
// user might set the value after initialization time and we need to have
// all the pointers lying around, not just one.
void SkRTConfRegistry::registerConf(SkRTConfBase *conf) {
    SkTDArray<SkRTConfBase *> *confArray;
    if (fConfs.find(conf->getName(), &confArray)) {
        if (!conf->equals(confArray->getAt(0))) {
            SkDebugf("WARNING: Skia config \"%s\" was registered more than once in incompatible ways.\n", conf->getName());
        } else {
            confArray->append(1, &conf);
        }
    } else {
        confArray = new SkTDArray<SkRTConfBase *>;
        confArray->append(1, &conf);
        fConfs.set(conf->getName(),confArray);
    }
}

template <typename T> T doParse(const char *, bool *success ) {
    SkDebugf("WARNING: Invoked non-specialized doParse function...\n");
    if (success) {
        *success = false;
    }
    return (T) 0;
}

template<> bool doParse<bool>(const char *s, bool *success) {
    if (success) {
        *success = true;
    }
    if (!strcmp(s,"1") || !strcmp(s,"true")) {
        return true;
    }
    if (!strcmp(s,"0") || !strcmp(s,"false")) {
        return false;
    }
    if (success) {
        *success = false;
    }
    return false;
}

template<> const char * doParse<const char *>(const char * s, bool *success) {
    if (success) {
        *success = true;
    }
    return s;
}

template<> int doParse<int>(const char * s, bool *success) {
    if (success) {
        *success = true;
    }
    return atoi(s);
}

template<> unsigned int doParse<unsigned int>(const char * s, bool *success) {
    if (success) {
        *success = true;
    }
    return (unsigned int) atoi(s);
}

template<> float doParse<float>(const char * s, bool *success) {
    if (success) {
        *success = true;
    }
    return (float) atof(s);
}

template<> double doParse<double>(const char * s, bool *success) {
    if (success) {
        *success = true;
    }
    return atof(s);
}

static inline void str_replace(char *s, char search, char replace) {
    for (char *ptr = s ; *ptr ; ptr++) {
        if (*ptr == search) {
            *ptr = replace;
        }
    }
}

template<typename T> bool SkRTConfRegistry::parse(const char *name, T* value) {
    SkString *str = NULL;
    SkString tmp;

    for (int i = fConfigFileKeys.count() - 1 ; i >= 0; i--) {
        if (fConfigFileKeys[i]->equals(name)) {
            str = fConfigFileValues[i];
            break;
        }
    }

    SkString environment_variable("skia.");
    environment_variable.append(name);

    const char *environment_value = getenv(environment_variable.c_str());
    if (environment_value) {
        if (NULL == str) {
            str = &tmp;
        }
        str->set(environment_value);
    } else {
        // apparently my shell doesn't let me have environment variables that
        // have periods in them, so also let the user substitute underscores.
        SkString underscore_environment_variable("skia_");
        char *underscore_name = SkStrDup(name);
        str_replace(underscore_name,'.','_');
        underscore_environment_variable.append(underscore_name);
        sk_free(underscore_name);
        environment_value = getenv(underscore_environment_variable.c_str());
        if (environment_value) {
            if (NULL == str) {
                str = &tmp;
            }
            str->set(environment_value);
        }
    }

    if (!str) {
        return false;
    }

    bool success;
    T new_value = doParse<T>(str->c_str(),&success);
    if (success) {
        *value = new_value;
    } else {
        SkDebugf("WARNING: Couldn't parse value \'%s\' for variable \'%s\'\n", str->c_str(), name);
    }
    return success;
}

// need to explicitly instantiate the parsing function for every config type we might have...

template bool SkRTConfRegistry::parse(const char *name, bool *value);
template bool SkRTConfRegistry::parse(const char *name, int *value);
template bool SkRTConfRegistry::parse(const char *name, unsigned int *value);
template bool SkRTConfRegistry::parse(const char *name, float *value);
template bool SkRTConfRegistry::parse(const char *name, double *value);
template bool SkRTConfRegistry::parse(const char *name, const char **value);

template <typename T> void SkRTConfRegistry::set(const char *name, T value) {

    SkTDArray<SkRTConfBase *> *confArray;
    if (!fConfs.find(name, &confArray)) {
        SkDebugf("WARNING: Attempting to set configuration value \"%s\", but I've never heard of that.\n", name);
        return;
    }

    for (SkRTConfBase **confBase = confArray->begin(); confBase != confArray->end(); confBase++) {
        // static_cast here is okay because there's only one kind of child class.
        SkRTConf<T> *concrete = static_cast<SkRTConf<T> *>(*confBase);

        if (concrete) {
            concrete->set(value);
        }
    }
}

template void SkRTConfRegistry::set(const char *name, bool value);
template void SkRTConfRegistry::set(const char *name, int value);
template void SkRTConfRegistry::set(const char *name, unsigned int value);
template void SkRTConfRegistry::set(const char *name, float value);
template void SkRTConfRegistry::set(const char *name, double value);
template void SkRTConfRegistry::set(const char *name, char * value);

SkRTConfRegistry &skRTConfRegistry() {
    static SkRTConfRegistry r;
    return r;
}


#ifdef SK_SUPPORT_UNITTEST

#ifdef SK_BUILD_FOR_WIN32
static void sk_setenv(const char* key, const char* value) {
    _putenv_s(key, value);
}
#else
static void sk_setenv(const char* key, const char* value) {
    setenv(key, value, 1);
}
#endif

void SkRTConfRegistry::UnitTest() {
    SkRTConfRegistry registryWithoutContents(true);

    sk_setenv("skia_nonexistent_item", "132");
    int result = 0;
    registryWithoutContents.parse("nonexistent.item", &result);
    SkASSERT(result == 132);
}

SkRTConfRegistry::SkRTConfRegistry(bool)
    : fConfs(100) {
}
#endif
