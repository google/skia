
out vec4 sk_FragColor;
uniform int value;
vec4 switchy(int v) {
    switch (v) {
        case 0:
            return vec4(0.5);
    }
    return vec4(1.0);
}
void main() {
    sk_FragColor = switchy(value);
}
