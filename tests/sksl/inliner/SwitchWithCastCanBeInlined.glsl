
out vec4 sk_FragColor;
uniform vec4 color;
vec4 switchy_h4h4(vec4 c) {
    vec4 result;
    switch (int(c.x)) {
        case 1:
            result = c.yyyy;
            break;
        default:
            result = c.zzzz;
            break;
    }
    return result;
}
void main() {
    sk_FragColor = switchy_h4h4(color);
}
