
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool inside_while_loop_b() {
    while (unknownInput == 123.0) {
        return false;
    }
    return true;
}
bool inside_infinite_do_loop_b() {
    do {
        return true;
    } while (true);
}
bool inside_infinite_while_loop_b() {
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
vec4 main() {
    return (((inside_while_loop_b() && inside_infinite_do_loop_b()) && inside_infinite_while_loop_b()) && after_do_loop_b()) && after_while_loop_b() ? colorGreen : colorRed;
}
