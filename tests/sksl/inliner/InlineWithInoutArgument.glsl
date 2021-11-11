
out vec4 sk_FragColor;
uniform vec4 colorGreen;
void outParameterWrite_vh4(out vec4 x) {
    x = colorGreen;
}
void outParameterWriteIndirect_vh4(out vec4 c) {
    outParameterWrite_vh4(c);
}
void inoutParameterWrite_vh4(inout vec4 x) {
    x *= x;
}
void inoutParameterWriteIndirect_vh4(inout vec4 x) {
    inoutParameterWrite_vh4(x);
}
vec4 main() {
    vec4 c;
    outParameterWrite_vh4(c);
    outParameterWriteIndirect_vh4(c);
    inoutParameterWrite_vh4(c);
    inoutParameterWriteIndirect_vh4(c);
    false;
    false;
    false;
    return c;
}
