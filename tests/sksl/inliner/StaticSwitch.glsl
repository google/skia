
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float get_f() {
    {
        return 2.0;
    }
}
vec4 main() {
    float result = get_f();
    return result == 2.0 ? colorGreen : colorRed;
}
