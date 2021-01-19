
vec4 main() {
    bool _1_g = true == true;
    bool _2_h = false == true;
    bool _3_i = true == true;
    bool _4_j = false == true;
    return ((_1_g && !_2_h) && _3_i) && !_4_j ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);

}
