
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool return_on_both_sides_b() {
    if (unknownInput == 1.0) return true; else return true;
}
bool for_inside_body_b() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body_b() {
    for (int x = 0;x <= 10; ++x) {
        true;
    }
    return true;
}
bool for_with_double_sided_conditional_return_b() {
    for (int x = 0;x <= 10; ++x) {
        if (unknownInput == 1.0) return true; else return true;
    }
}
bool if_else_chain_b() {
    if (unknownInput == 1.0) return true; else if (unknownInput == 2.0) return false; else if (unknownInput == 3.0) return true; else if (unknownInput == 4.0) return false; else return true;
}
bool conditional_inside_while_loop_b() {
    while (unknownInput == 123.0) {
        return true;
    }
}
bool inside_do_loop_b() {
    do {
        return true;
    } while (true);
}
bool inside_while_loop_b() {
    while (true) {
        return true;
    }
}
bool after_do_loop_b() {
    do {
        break;
    } while (true);
    return true;
}
bool after_while_loop_b() {
    while (true) {
        break;
    }
    return true;
}
bool switch_with_all_returns_b() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
            return true;
        default:
            return true;
    }
}
bool switch_only_default_b() {
    switch (int(unknownInput)) {
        case 0:
        default:
            return true;
    }
}
bool switch_fallthrough_b() {
    switch (int(unknownInput)) {
        case 1:
            return true;
        case 2:
        default:
            return true;
    }
}
bool switch_fallthrough_twice_b() {
    switch (int(unknownInput)) {
        case 1:
        case 2:
        default:
            return true;
    }
}
bool switch_with_break_in_loop_b() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                break;
            }
        default:
            return true;
    }
}
bool switch_with_continue_in_loop_b() {
    switch (int(unknownInput)) {
        case 1:
            for (int x = 0;x <= 10; ++x) {
                continue;
            }
        default:
            return true;
    }
}
bool switch_with_if_that_returns_b() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return true; else return true;
        default:
            return true;
    }
}
bool switch_with_one_sided_if_then_fallthrough_b() {
    switch (int(unknownInput)) {
        case 1:
            if (unknownInput == 123.0) return true;
        default:
            return true;
    }
}
vec4 main() {
    return (((((((((((((((((true && return_on_both_sides_b()) && for_inside_body_b()) && after_for_body_b()) && for_with_double_sided_conditional_return_b()) && if_else_chain_b()) && conditional_inside_while_loop_b()) && inside_do_loop_b()) && inside_while_loop_b()) && after_do_loop_b()) && after_while_loop_b()) && switch_with_all_returns_b()) && switch_only_default_b()) && switch_fallthrough_b()) && switch_fallthrough_twice_b()) && switch_with_break_in_loop_b()) && switch_with_continue_in_loop_b()) && switch_with_if_that_returns_b()) && switch_with_one_sided_if_then_fallthrough_b() ? colorGreen : colorRed;
}
