
out vec4 sk_FragColor;
void increment_vfff(inout float a, inout float b, inout float c) {
    ((a++ , b++) , c++);
}
vec4 main() {
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;
    for (int x = 0;x < 1; ++x) {
        break;
    }
    float d = c;
    b++;
    d++;
    return vec4(float(b == 2.0), float(b == 3.0), float(d == 5.0), float(d == 4.0));
}
