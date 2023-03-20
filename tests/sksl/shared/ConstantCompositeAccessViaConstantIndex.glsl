
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform float testArray[5];
const float globalArray[5] = float[5](1.0, 1.0, 1.0, 1.0, 1.0);
const vec2 globalVector = vec2(1.0);
const mat2 globalMatrix = mat2(1.0, 1.0, 1.0, 1.0);
vec4 main() {
    const float localArray[5] = float[5](0.0, 1.0, 2.0, 3.0, 4.0);
    const vec2 localVector = vec2(1.0);
    const mat2 localMatrix = mat2(0.0, 1.0, 2.0, 3.0);
    if (((((globalArray == testArray || globalVector == colorRed.xy) || globalMatrix == testMatrix2x2) || localArray == testArray) || localVector == colorRed.xy) || localMatrix == testMatrix2x2) {
        return colorRed;
    }
    return vec4(0.0, 1.0, 0.0, 1.0);
}
