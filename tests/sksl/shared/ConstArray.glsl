
out vec4 sk_FragColor;
const float test[4] = float[4](1.0, 2.0, 3.0, 4.0);
void main() {
    sk_FragColor = vec4(test[0], test[1], test[2], test[3]);
}
