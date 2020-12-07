package org.skia.skottie;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

import com.airbnb.lottie.LottieAnimationView;
import com.airbnb.lottie.LottieDrawable;
import com.airbnb.lottie.RenderMode;

public class PerfActivity extends Activity {

    int rawAssets[] = {
            R.raw.star, R.raw.movie_loading, R.raw.uk,  R.raw.white_material_wave_loading,
            R.raw.check_animation, R.raw.confetti, R.raw.gears,
            R.raw.hand_sanitizer, R.raw.heart_preloader, R.raw.i_was_scared_after_that_brouhaha,
            R.raw.im_thirsty, R.raw.true_will, R.raw.workout_monkey_stay_healthy,
            R.raw.ripple_loading_animation, R.raw.signature, R.raw.asdasd, R.raw.celebration,
            R.raw.check
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Remove title bar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

        //Remove notification bar
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);
        // Run activity with:
        // adb shell am start -n org.skia.skottie/.PerfActivity --ei renderer 0 --ei file 0
        // renderer is 0 to 2, file is 0 to 14
        Intent intent = getIntent();
        int renderer = intent.getIntExtra("renderer", 0);
        int file = intent.getIntExtra("file", 0);
        if (file < 0 || file >= rawAssets.length) {
            return;
        }
        switch (renderer) {
            case 0:
                // render with airbnb hardware lottie player
                runLottie(rawAssets[file], true);
                break;
            case 1:
                // render with airbnb software lottie player
                runLottie(rawAssets[file], false);
                break;
            case 2:
                // render with skottie player
                runSkottie(rawAssets[file]);
                break;
            default:
                break;
        }

    }

    void runSkottie(int id) {
        SkottieView view = new SkottieView(this);
        view.setSource(getResources().openRawResource(id));
        view.start();
        setContentView(view);
    }

    void runLottie(int id, boolean useHardware) {
        LottieAnimationView lottie = new LottieAnimationView(this);
        lottie.setAnimation(id);
        if (useHardware) {
            lottie.setRenderMode(RenderMode.HARDWARE);
        } else {
            lottie.setRenderMode(RenderMode.SOFTWARE);
        }
        lottie.setRepeatMode(LottieDrawable.RESTART);
        lottie.setRepeatCount(LottieDrawable.INFINITE);
        lottie.playAnimation();
        setContentView(lottie);
    }
}
