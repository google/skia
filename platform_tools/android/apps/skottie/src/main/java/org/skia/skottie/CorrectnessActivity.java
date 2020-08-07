package org.skia.skottie;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.PixelCopy;
import android.view.PixelCopy.OnPixelCopyFinishedListener;
import android.view.Window;
import android.view.WindowManager;
import android.os.Bundle;
import android.widget.GridLayout;
import android.widget.ImageView.ScaleType;
import com.airbnb.lottie.LottieAnimationView;
import java.io.File;
import java.io.FileOutputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

public class CorrectnessActivity extends Activity implements OnPixelCopyFinishedListener{

    private static final int OUTPUT_DIMS = 1000;
    private static final int PADDING = 200;

    private Rect bounds = new Rect(0, PADDING, OUTPUT_DIMS, OUTPUT_DIMS+PADDING);
    private GridLayout grid;
    private List<LottieAnimationView> lottieViews = new ArrayList<>();
    private static Handler sHandler;
    private int currAsset = 0;
    private ArrayList<Integer> rawAssets = getRawIDs(R.raw.class);


    static {
        HandlerThread thread = new HandlerThread("PixelCopyHelper");
        thread.start();
        sHandler = new Handler(thread.getLooper());
    }

    // Run activity with:
    // adb shell am start -n org.skia.skottie/.CorrectnessActivity
    // Correctness PNGs will be saves to download folder
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Remove title bar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //Remove notification bar
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_correctness);
        grid = findViewById(R.id.correctness_grid);
        grid.setPadding(0, PADDING, 0, 0);
        for (int i = 0; i < 25; i++) {
            GridLayout.Spec rowSpec = GridLayout.spec(i / 5, 1, GridLayout.CENTER);
            GridLayout.Spec colSpec = GridLayout.spec(i % 5, 1, GridLayout.CENTER);
            GridLayout.LayoutParams params = new GridLayout.LayoutParams(rowSpec, colSpec);
            params.width = 200;
            params.height = 200;
            LottieAnimationView l = new LottieAnimationView(this);
            l.setScaleType(ScaleType.CENTER_INSIDE);
            lottieViews.add(l);
            grid.addView(l, params);
        }
        setLotties(rawAssets.get(currAsset));
        runCorrectness(rawAssets.get(currAsset));
    }

    private void setLotties(int asset) {
        for (int i = 0; i < lottieViews.size(); i++) {
            LottieAnimationView view = lottieViews.get(i);
            float progress = (float)i / (lottieViews.size() - 1);
            view.setAnimation(asset);
            view.setProgress(progress);
        }
    }

    private void saveAsPng(int asset, Bitmap bitmap) {
        String lottieName = getResources().getResourceName(asset).replaceAll(".+/", "");
        String outDir = Environment.getExternalStorageDirectory().getPath() +
            "/Download/" + lottieName + ".PNG";
        try {
            FileOutputStream out = new FileOutputStream(outDir);
            bitmap.compress(Bitmap.CompressFormat.PNG, /*quality ignored for PNG*/100, out);
            out.close();
        } catch (Exception e) {
            Log.e("PNG conversion", e.getMessage());
            return;
        }
        if (currAsset < (rawAssets.size() - 1)) {
            currAsset++;
            setLotties(rawAssets.get(currAsset));
            runCorrectness(rawAssets.get(currAsset));
        } else {
            writeTerminatingFile();
        }
    }

    private void writeTerminatingFile() {
        String outDir = Environment.getExternalStorageDirectory().getPath() + "/Download/done.txt";
        try {
            FileOutputStream out = new FileOutputStream(outDir);
            out.write("done".getBytes());
            out.close();
        } catch (Exception e) {
            Log.e("Terminating file", e.getMessage());
        }
    }

    @Override
    public void onPixelCopyFinished(int copyResult) {
        synchronized (this) {
            this.notify();
        }
    }

    private void runCorrectness(int asset) {
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Bitmap out = grabScreenshot();
                saveAsPng(asset, out);
            }
        }, 20000);
    }

    private Bitmap grabScreenshot() {
        Bitmap outBitmap = Bitmap.createBitmap(OUTPUT_DIMS, OUTPUT_DIMS,
            Bitmap.Config.ARGB_8888);
        synchronized (this) {
            PixelCopy.request(this.getWindow(), bounds, outBitmap, this, sHandler);
            try {
                this.wait(250);
            } catch (InterruptedException e) {
                Log.e("PixelCopy", "PixelCopy request didn't complete within 250ms");
            }
        }
        return outBitmap;
    }

    private ArrayList<Integer> getRawIDs(Class<?> clz) {
        ArrayList<Integer> out = new ArrayList<>();
        final Field[] fields = clz.getDeclaredFields();
        for (Field field : fields) {
            final int drawableId;
            try {
                drawableId = field.getInt(clz);
            } catch (Exception e) {
                continue;
            }
            out.add(drawableId);
        }
        return out;
    }
}
