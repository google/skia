package org.skia.viewer;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;
import android.util.Log;

import java.io.IOException;

/** JNI interface to native layer. */
public class JniInterface {
  static {
    System.loadLibrary("hello_ar_native");
    System.loadLibrary("viewer");
  }

  private static final String TAG = "JniInterface";
  static AssetManager assetManager;

  public static native long createNativeApplication(AssetManager assetManager);

  public static native void destroyNativeApplication(long nativeApplication);

  public static native void onPause(long nativeApplication);

  public static native void onResume(long nativeApplication, Context context, Activity activity);

  /** Allocate OpenGL resources for rendering. */
  public static native void onGlSurfaceCreated(long nativeApplication);

  /**
   * Called on the OpenGL thread before onGlSurfaceDrawFrame when the view port width, height, or
   * display rotation may have changed.
   */
  public static native void onDisplayGeometryChanged(
      long nativeApplication, int displayRotation, int width, int height);

  /** Main render loop, called on the OpenGL thread. */
  public static native void onGlSurfaceDrawFrame(long nativeApplication);

  /** OnTouch event, called on the OpenGL thread. */
  public static native void onTouched(long nativeApplication, float x, float y);

  /** Get plane count in current session. Used to disable the "searching for surfaces" snackbar. */
  public static native boolean hasDetectedPlanes(long nativeApplication);

  public static Bitmap loadImage(String imageName) {

    try {
      return BitmapFactory.decodeStream(assetManager.open(imageName));
    } catch (IOException e) {
      Log.e(TAG, "Cannot open image " + imageName);
      return null;
    }
  }

  public static void loadTexture(int target, Bitmap bitmap) {
    GLUtils.texImage2D(target, 0, bitmap, 0);
  }
}
