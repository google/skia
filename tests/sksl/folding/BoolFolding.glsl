
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool _0_a = true;
    bool _1_b = false;
    bool _2_c = true;
    bool _3_d = false;
    bool _4_e = true;
    bool _5_f = false;
    bool _6_g = true;
    bool _7_h = false;
    bool _8_i = true;
    bool _9_j = false;
    return ((((((((_0_a && !_1_b) && _2_c) && !_3_d) && _4_e) && !_5_f) && _6_g) && !_7_h) && _8_i) && !_9_j ? colorGreen : colorRed;
}
