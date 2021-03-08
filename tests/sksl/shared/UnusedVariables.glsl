
out vec4 sk_FragColor;
vec4 main() {
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;

    float d = c;
    float e = d;
    b++;
    d++;
    return vec4(float(b == 2.0), float(b == 3.0), float(d == 5.0), float(d == 4.0));
}
