package com.example;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;
import java.util.Timer;
import java.util.TimerTask;






import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;




public class HelloSkiaActivity extends Activity implements SurfaceHolder.Callback
{
    private SkiaSurfaceView fMainView;

    private native int drawIntoBitmap(long elapsedTime);
    private native void create();

    public native void nativeOnResume();
    public native void nativeOnPause();
    public native void nativeOnStop();
    public native void nativeSetSurface(Surface surface);
    private static native void addPoint(float x, float y);
    private static native void endLine();
    private static native void startLine( float x, float y);



    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Makes and sets a SkiaDrawView as the only thing seen in this activity
        fMainView = new SkiaSurfaceView(this);
        setContentView(fMainView);
        fMainView.getHolder().addCallback(this);

        try {
            // Load Skia and then the app shared object in this order
            System.loadLibrary("skia_android");
    //        System.loadLibrary("hello_skia_ndk");

        } catch (UnsatisfiedLinkError e) {
            Log.d("HelloSkia", "Link Error: " + e);
            return;
        }


    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int arg1, int arg2, int arg3) {
        nativeSetSurface(holder.getSurface());

    }
    @Override
    public void surfaceCreated(SurfaceHolder arg0) {
        // TODO Auto-generated method stub

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder arg0) {
        nativeSetSurface(null);

    }

    @Override
    protected void onStart() {
        super.onStart();
        create();
    }

    @Override
    protected void onResume() {
        super.onResume();
        nativeOnResume();
    }

    @Override
    protected void onPause() {	        super.onPause();
        nativeOnPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        nativeOnStop();
    }


    private class SkiaSurfaceView extends SurfaceView
    {

        public SkiaSurfaceView(Context context) {
            super(context);
            // TODO Auto-generated constructor stub
        }


        @Override
        public boolean onTouchEvent(MotionEvent e) {

            // translate into gl space
            float x = e.getRawX();
            float y = e.getRawY();

            //       x -= getWidth()/2;
            //       y-= getHeight()/2;

            // s        y = -y;

            switch (e.getAction()) {

                case MotionEvent.ACTION_DOWN:{

                    startLine(x, y);
                }
                break;
                case MotionEvent.ACTION_MOVE:{

                    addPoint(x, y);
                }
                break;
                case MotionEvent.ACTION_UP:{
                    addPoint(x, y);
                    endLine();
                }
                break;
            }

            return true;
        }

    }




}
