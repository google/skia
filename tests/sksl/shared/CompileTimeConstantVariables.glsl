
out vec4 sk_FragColor;
const int kConstant = 0;
const int kOtherConstant = 1;
const int kAnotherConstant = 2;
const float kFloatConstant = 2.14;
const float kFloatConstantAlias = kFloatConstant;
const vec4 kConstVec = vec4(1.0, 0.2, 2.14, 1.0);
uniform vec4 colorGreen;
vec4 main() {
    const float kLocalFloatConstant = 3.14;
    const float kLocalFloatConstantAlias = kLocalFloatConstant;
    int integerInput = int(colorGreen.y);
    if (integerInput == kConstant) {
        return vec4(2.14);
    } else if (integerInput == kOtherConstant) {
        return colorGreen;
    } else if (integerInput == kAnotherConstant) {
        return kConstVec;
    } else if (kLocalFloatConstantAlias < colorGreen.x * kLocalFloatConstant) {
        return vec4(3.14);
    } else if (kFloatConstantAlias >= colorGreen.x * kFloatConstantAlias) {
        return vec4(0.0);
    } else {
        return vec4(1.0, 0.0, 0.0, 1.0);
    }
}
