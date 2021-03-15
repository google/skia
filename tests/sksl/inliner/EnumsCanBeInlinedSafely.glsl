
out vec4 sk_FragColor;
vec4 helper();
void main() {
    sk_FragColor = helper();
}
vec4 helper() {
    int temp = 1;
    switch (temp) {
        case 0:
            return vec4(0.0, 0.0, 0.0, 1.0);
        case 1:
            return vec4(0.5, 0.5, 0.5, 1.0);
        case 2:
            return vec4(1.0);
        default:
            return vec4(1.0, 0.0, 0.0, 1.0);
    }
}
