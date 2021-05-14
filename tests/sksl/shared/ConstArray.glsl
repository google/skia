
out vec4 sk_FragColor;
const float test[4] = float[4](0.0, 1.0, 0.0, 1.0);
vec4 main() {
    return vec4(test[0], test[1], test[2], test[3]);
}
