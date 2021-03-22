
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float get_f() {
    {
        return abs(2.0);
    }
    return abs(5.0);
}
vec4 main() {
    float result = get_f();
    return result == 2.0 ? colorGreen : colorRed;
}
