package org.skia.skottie;

import android.content.Context;
import android.util.AttributeSet;

public class SkottieViewBuilder {
    protected AttributeSet attrs;
    protected int defStyleAttr;

    // if true, backing view will be surface view
    protected boolean advancedFeatures;
    // TODO private variable backgroundColor

    public void setAttrs(AttributeSet attrs) {
      this.attrs = attrs;
    }

    public void setDefStyleAttr(int defStyleAttr) {
      this.defStyleAttr = defStyleAttr;
    }

    public void setAdvancedFeatures(boolean advancedFeatures) {
      this.advancedFeatures = advancedFeatures;
    }

    public SkottieView build(Context context) {
      return new SkottieView(context, this);
    }
}
