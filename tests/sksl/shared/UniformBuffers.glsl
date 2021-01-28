
out vec4 sk_FragColor;
layout (binding = 0, set = 0) uniform testBlock {
    float myHalf;
    vec4 myHalf4;
};
vec4 main() {
    return myHalf4 * myHalf;
}
