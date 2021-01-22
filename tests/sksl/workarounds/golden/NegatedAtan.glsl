#version 400
out vec4 sk_FragColor;
void main() {
    vec2 x = vec2(sqrt(2.0));
    sk_FragColor.x = atan(x.x, -1.0 * x.y);
}
