#include "com_skia_SkiaIntentService.h"

#include <stdint.h>
#include <stdio.h>

extern int main(int argc, char * const argv[]);

void cleanUp(JNIEnv* env, jobjectArray jstrs, const char** strs, int32_t count) {
    for (int32_t i = 0; i < count; ++i)
        env->ReleaseStringUTFChars(
            (jstring) env->GetObjectArrayElement(jstrs, i), strs[i]);
}

JNIEXPORT jint JNICALL Java_com_skia_SkiaIntentService_run(
        JNIEnv* env,
        jobject,
        jobjectArray args) {

    // Convert command line arguments to C format.
    int argc = env->GetArrayLength(args);
    const char** argv = new const char*[argc];
    for (int32_t i = 0; i < argc; ++i) {
        jstring str = (jstring) env->GetObjectArrayElement(args, i);
        argv[i] = env->GetStringUTFChars(str, NULL);
        if (NULL == argv[i]) {
            cleanUp(env, args, argv, i - 1);
            return 1;
        }
    }

    // Execute program main()
    int retval = main(argc, (char* const*) argv);

    // Clean up temporaries and return the exit code.
    cleanUp(env, args, argv, argc);
    delete[] argv;
    return retval;
}
