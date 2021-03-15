
out vec4 sk_FragColor;
uniform float a;
uniform vec4 b;
void main() {
    sk_FragColor.x = length(a);
    sk_FragColor.x = length(b);
}
