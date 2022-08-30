
out vec4 sk_FragColor;
uniform vec4 colorRed;
vec4 switchRedAndGreen_h4h4(vec4 v) {
    return v.yxzw;
}
vec4 main() {
    return switchRedAndGreen_h4h4(switchRedAndGreen_h4h4(switchRedAndGreen_h4h4(colorRed)));
}
