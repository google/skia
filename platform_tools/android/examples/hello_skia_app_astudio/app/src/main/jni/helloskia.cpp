#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include "SkPath.h"


#include "logger.h"
#include "renderer.h"

#define LOG_TAG "HiSkia"

static ANativeWindow *window = 0;
static Renderer *renderer = 0;


extern "C" {
    JNIEXPORT int JNICALL Java_com_example_HelloSkiaActivity_drawIntoBitmap(JNIEnv * env, jobject obj, jlong time);
    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_create(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnResume(JNIEnv* jenv, jobject obj);

    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnPause(JNIEnv* jenv, jobject obj);

    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnStop(JNIEnv* jenv, jobject obj);

    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface);

    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_addPoint(JNIEnv * env, jobject obj,  jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_endLine(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_startLine(JNIEnv * env, jobject obj,  jfloat x, jfloat y);


};



/**
 * Draws something into the given bitmap
 * @param  env
 * @param  thiz
 * @param  dstBitmap   The bitmap to place the results of skia into
 * @param  elapsedTime The number of milliseconds since the app was started
 */
JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_create(JNIEnv * env, jobject obj)
{

	LOG_INFO("nativeOnStart");
	renderer = new Renderer();
	    return;
}



JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnResume(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnResume");
    renderer->start();
    return;
}

JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnPause(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnPause");
    renderer->stop();
    return;
}

JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeOnStop(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnStop");
    renderer->stop(); // make sure its stopped first
    delete renderer;
    renderer = 0;
    return;
}

JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
{
    if (surface != 0) {
        window = ANativeWindow_fromSurface(jenv, surface);
        LOG_INFO("Got window %p", window);
        renderer->setWindow(window);
    } else {
        LOG_INFO("Releasing window");
        ANativeWindow_release(window);
    }

    return;
}

JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_addPoint(JNIEnv * env, jobject obj,  jfloat x, jfloat y)
{
	renderer->addPoint(x, y);
}
 JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_endLine(JNIEnv * env, jobject obj)
 {

 }

 JNIEXPORT void JNICALL Java_com_example_HelloSkiaActivity_startLine(JNIEnv * env, jobject obj,  jfloat x, jfloat y)
 {
	 renderer->beginLine(x, y);
 }







