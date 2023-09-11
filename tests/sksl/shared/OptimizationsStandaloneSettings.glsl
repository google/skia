
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool flatten_known_if_b() {
    int value = 1;
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
bool eliminate_no_op_arithmetic_b() {
    const int ONE = 1;
    int x = ONE;
    return x == 1;
}
bool flatten_switch_b() {
    {
        return true;
    }
}
vec4 main() {
    return ((((((true && flatten_known_if_b()) && eliminate_empty_if_else_b()) && eliminate_empty_else_b()) && flatten_matching_ternary_b()) && flatten_expr_without_side_effects_b()) && eliminate_no_op_arithmetic_b()) && flatten_switch_b() ? colorGreen : colorRed;
}
