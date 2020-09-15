
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    mediump float x = 1.0;
    {
        x *= 2.0;
    }


    sk_FragColor.x = x;
}
