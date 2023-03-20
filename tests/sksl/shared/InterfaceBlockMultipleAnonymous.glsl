
out vec4 sk_FragColor;
layout (binding = 1) uniform testBlockA {
    vec2 x;
};
layout (binding = 2) uniform testBlockB {
    vec2 y;
};
void main() {
    sk_FragColor = vec4(x, y);
}
