
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <jsapi.h>

#include "SkJS.h"
#include "SkString.h"

#ifdef _WIN32_WCE
extern "C" {
    void abort() {
        SkASSERT(0);
    }

    unsigned int _control87(unsigned int _new, unsigned int mask ) {
        SkASSERT(0);
        return 0;
    }

    time_t mktime(struct tm *timeptr ) {
        SkASSERT(0);
        return 0;
    }

//  int errno;

    char *strdup(const char *) {
        SkASSERT(0);
        return 0;
    }

    char *strerror(int errnum) {
        SkASSERT(0);
        return 0;
    }

    int isatty(void* fd) {
        SkASSERT(0);
        return 0;
    }

    int putenv(const char *envstring) {
        SkASSERT(0);
        return 0;
    }

    char *getenv(const char *varname) {
        SkASSERT(0);
        return 0;
    }

    void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
        SkASSERT(0);
    }

    struct tm * localtime(const time_t *timer) {
        SkASSERT(0);
        return 0;
    }

    size_t strftime(char *strDest, size_t maxsize, const char *format,
        const struct tm *timeptr ) {
        SkASSERT(0);
        return 0;
    }

}
#endif

static JSBool
global_enumerate(JSContext *cx, JSObject *obj)
{
#ifdef LAZY_STANDARD_CLASSES
    return JS_EnumerateStandardClasses(cx, obj);
#else
    return JS_TRUE;
#endif
}

static JSBool
global_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp)
{
#ifdef LAZY_STANDARD_CLASSES
    if ((flags & JSRESOLVE_ASSIGNING) == 0) {
        JSBool resolved;

        if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
            return JS_FALSE;
        if (resolved) {
            *objp = obj;
            return JS_TRUE;
        }
    }
#endif

#if defined(SHELL_HACK) && defined(DEBUG) && defined(XP_UNIX)
    if ((flags & (JSRESOLVE_QUALIFIED | JSRESOLVE_ASSIGNING)) == 0) {
        /*
         * Do this expensive hack only for unoptimized Unix builds, which are
         * not used for benchmarking.
         */
        char *path, *comp, *full;
        const char *name;
        JSBool ok, found;
        JSFunction *fun;

        if (!JSVAL_IS_STRING(id))
            return JS_TRUE;
        path = getenv("PATH");
        if (!path)
            return JS_TRUE;
        path = JS_strdup(cx, path);
        if (!path)
            return JS_FALSE;
        name = JS_GetStringBytes(JSVAL_TO_STRING(id));
        ok = JS_TRUE;
        for (comp = strtok(path, ":"); comp; comp = strtok(NULL, ":")) {
            if (*comp != '\0') {
                full = JS_smprintf("%s/%s", comp, name);
                if (!full) {
                    JS_ReportOutOfMemory(cx);
                    ok = JS_FALSE;
                    break;
                }
            } else {
                full = (char *)name;
            }
            found = (access(full, X_OK) == 0);
            if (*comp != '\0')
                free(full);
            if (found) {
                fun = JS_DefineFunction(cx, obj, name, Exec, 0, JSPROP_ENUMERATE);
                ok = (fun != NULL);
                if (ok)
                    *objp = obj;
                break;
            }
        }
        JS_free(cx, path);
        return ok;
    }
#else
    return JS_TRUE;
#endif
}

JSClass global_class = {
    "global", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    global_enumerate, (JSResolveOp) global_resolve,
    JS_ConvertStub,   JS_FinalizeStub
};

SkJS::SkJS(void* hwnd) : SkOSWindow(hwnd) {
    if ((fRuntime = JS_NewRuntime(0x100000)) == NULL) {
        SkASSERT(0);
        return;
    }
    if ((fContext = JS_NewContext(fRuntime, 0x1000)) == NULL) {
        SkASSERT(0);
        return;
    }
    ;
    if ((fGlobal = JS_NewObject(fContext, &global_class, NULL, NULL)) == NULL) {
        SkASSERT(0);
        return;
    }
    if (JS_InitStandardClasses(fContext, fGlobal) == NULL) {
        SkASSERT(0);
        return;
    }
    setConfig(SkBitmap::kARGB32_Config);
    updateSize();
    setVisibleP(true);
    InitializeDisplayables(getBitmap(), fContext, fGlobal, NULL);
}

SkJS::~SkJS() {
    DisposeDisplayables();
    JS_DestroyContext(fContext);
    JS_DestroyRuntime(fRuntime);
    JS_ShutDown();
}

SkBool SkJS::EvaluateScript(const char* script, jsval* rVal) {
    return JS_EvaluateScript(fContext, fGlobal, script, strlen(script),
        "memory" /* no file name */, 0 /* no line number */, rVal);
}

SkBool SkJS::ValueToString(jsval value, SkString* string) {
     JSString* str = JS_ValueToString(fContext, value);
     if (str == NULL)
         return false;
     string->set(JS_GetStringBytes(str));
     return true;
}

#ifdef SK_DEBUG
void SkJS::Test(void* hwnd) {
    SkJS js(hwnd);
    jsval val;
    SkBool success = js.EvaluateScript("22/7", &val);
    SkASSERT(success);
    SkString string;
    success = js.ValueToString(val, &string);
    SkASSERT(success);
    SkASSERT(strcmp(string.c_str(), "3.142857142857143") == 0);
    success = js.EvaluateScript(
        "var rect = new rectangle();"
        "rect.left = 4;"
        "rect.top = 10;"
        "rect.right = 20;"
        "rect.bottom = 30;"
        "rect.width = rect.height + 20;"
        "rect.draw();"
        , &val);
    SkASSERT(success);
    success = js.ValueToString(val, &string);
    SkASSERT(success);
}
#endifASSERT(success);
