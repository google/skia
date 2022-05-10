
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool flatten_known_if_b() {
    int value;
    {
        value = 1;
    }
    return value == 1;
}
bool eliminate_empty_if_else_b() {
    bool check = false;
    check = !check;
    return check;
}
bool eliminate_empty_else_b() {
    bool check = true;
    if (check) {
        return true;
    }
    return false;
}
bool flatten_matching_ternary_b() {
    return true;
}
bool flatten_expr_without_side_effects_b() {
    bool check = true;
    return check;
}
bool flatten_switch_b() {
    {
        return true;
    }
}
vec4 main() {
    ivec4 _0_x = ivec4(1, 2, 3, 4);
    ivec4 _1_y = ivec4(1, 2, 3, 4);
    return (((((_0_x == _1_y && flatten_known_if_b()) && eliminate_empty_if_else_b()) && eliminate_empty_else_b()) && flatten_matching_ternary_b()) && flatten_expr_without_side_effects_b()) && flatten_switch_b() ? colorGreen : colorRed;
}
