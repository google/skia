
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void foo(out mediump float x) {
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    x = 42.0;
}
void main() {
    mediump float _2_y = 0.0;
    {
        foo(_2_y);
    }


    sk_FragColor.x = 0.0;
}
