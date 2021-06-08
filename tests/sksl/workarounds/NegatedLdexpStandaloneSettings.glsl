
out vec4 sk_FragColor;
uniform float unknownFloat;
uniform int unknownInt;
void main() {
    float m = unknownFloat;
    int x = unknownInt;
    sk_FragColor.x = ldexp(m, -x);
    sk_FragColor.y = ldexp(m, -(x + 1));
    sk_FragColor.z = ldexp(m, -x - 1);
}
