
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_return_b() {
    do {
        return true;
    } while (false);
}
bool test_break_b() {
    do {
        break;
    } while (false);
    return true;
}
bool test_continue_b() {
    do {
        continue;
    } while (false);
    return true;
}
bool test_if_return_b() {
    do {
        if (colorGreen.y > 0.0) {
            return true;
        } else {
            break;
        }
        continue;
    } while (false);
    return false;
}
bool test_if_break_b() {
    do {
        if (colorGreen.y > 0.0) {
            break;
        } else {
            continue;
        }
    } while (false);
    return true;
}
bool test_else_b() {
    do {
        if (colorGreen.y == 0.0) {
            return false;
        } else {
            return true;
        }
    } while (false);
}
bool test_loop_return_b() {
    for (int x = 0;x < 0; ++x) {
        return false;
    }
    return true;
}
bool test_loop_break_b() {
    for (int x = 0;x <= 1; ++x) {
        break;
    }
    return true;
}
vec4 main() {
    return ((((((test_return_b() && test_break_b()) && test_continue_b()) && test_if_return_b()) && test_if_break_b()) && test_else_b()) && test_loop_return_b()) && test_loop_break_b() ? colorGreen : colorRed;
}
