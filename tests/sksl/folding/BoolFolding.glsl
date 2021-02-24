
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool _0_test;
    bool _1_a = true;
    bool _2_b = false;
    bool _3_c = true;
    bool _4_d = false;
    bool _5_e = true;
    bool _6_f = false;
    bool _7_g = true;
    bool _8_h = false;
    bool _9_i = true;
    bool _10_j = false;
    return ((((((((_1_a && !_2_b) && _3_c) && !_4_d) && _5_e) && !_6_f) && _7_g) && !_8_h) && _9_i) && !_10_j ? colorGreen : colorRed;

}
