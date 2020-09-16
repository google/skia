#version 400
int _absemulation(int x) {
    return x * sign(x);
}
out vec4 sk_FragColor;
uniform int i;
uniform float f;
void main() {
    float output = abs(f) + float(_absemulation(i));
    sk_FragColor = vec4(output);
}
