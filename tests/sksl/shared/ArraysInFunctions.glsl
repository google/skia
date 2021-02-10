
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    float a[2];
};
S returns_an_array_in_a_struct() {
    S s;
    s.a[0] = 1.0;
    s.a[1] = 2.0;
    return s;
}
float accepts_an_array(float a[2]) {
    return a[0] + a[1];
}
void modifies_an_array(inout float a[2]) {
    a[0]++;
    a[1]++;
}
vec4 main() {
    S s = returns_an_array_in_a_struct();
    float x = accepts_an_array(s.a);
    modifies_an_array(s.a);
    bool valid = (x == 3.0 && s.a[0] == 2.0) && s.a[1] == 3.0;
    return valid ? colorGreen : colorRed;
}
