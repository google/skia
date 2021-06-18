
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float foo_ff2(vec2 v) {
    return v.x * v.y;
}
void bar_vf(inout float  ) {
    float y[2];
    y[0] =  ;
    y[1] =   * 2.0;
      = foo_ff2(vec2(y[0], y[1]));
}
vec4 main() {
    float x = 10.0;
    bar_vf(x);
    return x == 200.0 ? colorGreen : colorRed;
}
