#version 400
out vec4 sk_FragColor;
uniform float x;
uniform float y;
void main() {
    float z = exp2((y + 2.0) * log2(x + 1.0));
    sk_FragColor = vec4(z);
}
