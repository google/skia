
out vec4 sk_FragColor;
vec4 a;
vec4 b;
void main() {
    sk_FragColor.x = float(lessThanEqual(a, b).x ? 1 : 0);
}
