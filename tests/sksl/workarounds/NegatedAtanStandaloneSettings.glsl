
out vec4 sk_FragColor;
uniform float unknownInput;
void main() {
    vec2 x = vec2(unknownInput);
    sk_FragColor.x = atan(x.x, -x.y);
}
