
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
void out_param_func1_vh(inout float v) {
    v = colorGreen.y;
}
int out_param_func2_ih(out float v) {
    v = colorRed.x;
    return int(v);
}
vec4 main() {
    float testArray[2];
    out_param_func1_vh(testArray[out_param_func2_ih(testArray[0])]);
    return testArray[0] == 1.0 && testArray[1] == 1.0 ? colorGreen : colorRed;
}
