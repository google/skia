
out vec4 sk_FragColor;
uniform int value;
vec4 switchy_h4i(int v) {
    vec4 result = vec4(1.0);
    switch (v) {
        case 0:
            result = vec4(0.5);
    }
    return result;
}
void main() {
    sk_FragColor = switchy_h4i(value);
}
