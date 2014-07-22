
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkThreadUtils.h"

class SkiaInstance;

// Used by SkDebugf
SkiaInstance* gPluginInstance;

// Main entry point for the app we're linked into
extern int test_main();

// Tokenize a command line and store it in argc and argv.
void SkStringToProgramArgs(const SkString commandLine, int* argc, char*** argv) {
    int numBreaks = 0;
    const char* commandChars = commandLine.c_str();
    for (size_t i = 0; i < strlen(commandChars); i++) {
        if (isspace(commandChars[i])) {
            numBreaks++;
        }
    }
    int numArgs;
    if (strlen(commandChars) > 0) {
        numArgs = numBreaks + 1;
    } else {
        numArgs = 0;
    }
    *argc = numArgs;
    *argv = new char*[numArgs + 1];
    (*argv)[numArgs] = NULL;
    char* start = (char*) commandChars;
    int length = 0;
    int argIndex = 0;
    for (size_t i = 0; i < strlen(commandChars) + 1; i++) {
        if (isspace(commandChars[i]) || '\0' == commandChars[i]) {
            if (length > 0) {
                char* argument = new char[length + 1];
                memcpy(argument, start, length);
                argument[length] = '\0';
                (*argv)[argIndex++] = argument;
            }
            start = (char*) commandChars + i + 1;
            length = 0;
        } else {
            length++;
        }
    }
}

// Run the program with the given command line.
void RunProgram(const SkString& commandLine) {
    int argc;
    char** argv;
    SkStringToProgramArgs(commandLine, &argc, &argv);
    test_main();
}


// Skia's subclass of pp::Instance, our interface with the browser.
class SkiaInstance : public pp::Instance {
public:
    explicit SkiaInstance(PP_Instance instance) : pp::Instance(instance) {
        gPluginInstance = this;
    }

    virtual ~SkiaInstance() {
        gPluginInstance = NULL;
    }

    virtual void HandleMessage(const pp::Var& var_message) {
        // Receive a message from javascript.
        if (var_message.is_string()) {
            SkString msg(var_message.AsString().c_str());
            if (msg.startsWith("init")) {
                RunProgram(msg);
            }
        }
    }
};

class SkiaModule : public pp::Module {
public:
    SkiaModule() : pp::Module() {}
    virtual ~SkiaModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new SkiaInstance(instance);
    }
};

namespace pp {
Module* CreateModule() {
    return new SkiaModule();
}
}  // namespace pp
