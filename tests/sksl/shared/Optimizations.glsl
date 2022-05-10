
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool flatten_compound_constructor_b() {
    ivec4 x = ivec4(ivec3(ivec2(1, 2), 3), 4);
    ivec4 y = ivec4(1, ivec3(2, ivec2(3, 4)));
    return x == y;
}
bool flatten_known_if_b() {
    int value;
    if (true) {
        value = 1;
    } else {
        value = 2;
    }
    return value == 1;
}
bool eliminate_empty_if_else_b() {
    bool check = false;
    if (check = !check) {
    } else {
    }
    return check;
}
bool eliminate_empty_else_b() {
    bool check = true;
    if (check) {
        return true;
    } else {
    }
    return false;
}
bool flatten_matching_ternary_b() {
    bool check = true;
    return check ? true : true;
}
bool flatten_expr_without_side_effects_b() {
    bool check = true;
    check;
    return check;
}
bool flatten_switch_b() {
    switch (1) {
        case 0:
            return false;
        case 1:
            return true;
        case 2:
            return false;
    }
    return false;
}
vec4 main() {
    return (((((flatten_compound_constructor_b() && flatten_known_if_b()) && eliminate_empty_if_else_b()) && eliminate_empty_else_b()) && flatten_matching_ternary_b()) && flatten_expr_without_side_effects_b()) && flatten_switch_b() ? colorGreen : colorRed;
}
