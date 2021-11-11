
out vec4 sk_FragColor;
uniform vec4 colorGreen;
void outParameterWrite_vh4(out vec4 x) {
    x = colorGreen;
}
void inoutParameterWrite_vh4(inout vec4 x) {
    x *= x;
}
void inoutParameterRead_vh4(inout vec4 x) {
}
void inoutParameterIgnore_vh4(inout vec4 x) {
}
void outParameterIgnore_vh4(out vec4 x) {
}
vec4 main() {
    vec4 c;
    outParameterWrite_vh4(c);
    inoutParameterWrite_vh4(c);
    inoutParameterRead_vh4(c);
    inoutParameterIgnore_vh4(c);
    outParameterIgnore_vh4(c);
    return c;
}
