
out vec4 sk_FragColor;
float userfunc_ff(float v) {
    return v + 1.0;
}
vec4 main() {
    float b = 2.0;
    float c = 3.0;
    b = 2.0;
    b = c + 77.0;
    b = sin(c + 77.0);
    userfunc_ff(c + 77.0);
    b = userfunc_ff(c + 77.0);
    b = (b = cos(c));
    for (int x = 0;x < 1; ++x) {
        continue;
    }
    float d = c;
    b = 3.0;
    d++;
    return vec4(float(b == 2.0), float(b == 3.0), float(d == 5.0), float(d == 4.0));
}
