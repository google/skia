package org.skia.androidkitdemo1;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Paint;
import org.skia.androidkit.RuntimeShaderBuilder;
import org.skia.androidkit.Surface;
import org.skia.androidkit.Text;
import org.skia.androidkit.util.SurfaceRenderer;

import java.io.InputStream;

class TextRenderer extends SurfaceRenderer {
    Surface mSurface;
    Text mText;
    Paint foreground;
    private static final String sksl =
            "uniform half u_time;                                  " +
                    "uniform half u_w;                                     " +
                    "uniform half u_h;                                     " +

                    "float f(vec3 p) {                                     " +
                    "   p.z -= u_time * 10.;                               " +
                    "   float a = p.z * .1;                                " +
                    "   p.xy *= mat2(cos(a), sin(a), -sin(a), cos(a));     " +
                    "   return .1 - length(cos(p.xy) + sin(p.yz));         " +
                    "}                                                     " +

                    "half4 main(vec2 fragcoord) {                          " +
                    "   vec3 d = .5 - fragcoord.xy1 / u_h;                 " +
                    "   vec3 p=vec3(0);                                    " +
                    "   for (int i = 0; i < 32; i++) p += f(p) * d;        " +
                    "   return ((sin(p) + vec3(2, 5, 9)) / length(p)).xyz1;" +
                    "}";
    private RuntimeShaderBuilder builder;
    private float W;
    private float H;
    @Override
    protected void onSurfaceInitialized(Surface surface) {
        mSurface = surface;
        W = surface.getWidth();
        H = surface.getHeight();
        mText = new Text("Hello World!");
        foreground = new Paint().setColor(0, 1, 0, 1);

        builder = new RuntimeShaderBuilder(sksl);
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        canvas.drawColor(1, 1, 1, 1);
        builder.setUniform("u_time", ms / 1000.0f)
               .setUniform("u_w", W)
               .setUniform("u_h", H);
        foreground.setShader(builder.makeShader());
        mText.renderText(canvas, foreground, 0, 200);
    }
}

public class TextActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    Surface mSurface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        TextRenderer renderer = new TextRenderer();
        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(renderer);
    }
}