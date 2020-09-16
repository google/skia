
out vec4 sk_FragColor;
uniform int i;
uniform float f;
void main() {
    float output = abs(f) + float(abs(i));
    sk_FragColor = vec4(output);
}
