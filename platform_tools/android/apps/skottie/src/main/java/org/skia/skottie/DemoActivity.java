package org.skia.skottie;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import java.util.ArrayList;
import java.util.List;

public class DemoActivity extends Activity implements OnClickListener {
    private List<SkottieView> skotties = new ArrayList<>();
    private boolean playing = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.demo_layout);

        LinearLayout skottieContainer = findViewById(R.id.skottie_container);
        for (int i = 0; i < skottieContainer.getChildCount(); i++) {
            SkottieView s = (SkottieView)skottieContainer.getChildAt(i);
            skotties.add(s);
            s.play();
        }

        Button play = findViewById(R.id.play);
        play.setOnClickListener(this);
        Button reset = findViewById(R.id.reset);
        reset.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch(view.getId()) {
            case R.id.play:
                for (SkottieView s : skotties) {
                    if (playing) {
                        s.pause();
                    } else {
                        s.play();
                    }
                }
                playing = !playing;
                break;
            case R.id.reset:
                for (SkottieView s : skotties) {
                    s.seek(0f);
                }
                break;
        }
    }
}
