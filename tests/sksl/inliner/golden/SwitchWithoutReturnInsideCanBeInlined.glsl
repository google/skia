
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _1_result = vec4(1.0);
    switch (value) {
        case 0:
            _1_result = vec4(0.5);
    }
    sk_FragColor = _1_result;

}
