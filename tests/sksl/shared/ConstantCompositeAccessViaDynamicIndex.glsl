
out vec4 sk_FragColor;
int zero = 0;
const float globalArray[2] = float[2](1.0, 1.0);
const vec2 globalVector = vec2(1.0);
const mat2 globalMatrix = mat2(1.0, 1.0, 1.0, 1.0);
vec4 main() {
    const float localArray[2] = float[2](0.0, 1.0);
    const vec2 localVector = vec2(1.0);
    const mat2 localMatrix = mat2(0.0, 1.0, 2.0, 3.0);
    return vec4(globalArray[zero] * localArray[zero], globalVector[zero] * localVector[zero], globalMatrix[zero] * localMatrix[zero]);
}
