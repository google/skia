
out vec4 sk_FragColor;
struct S {
    float y;
};
layout (binding = 0, set = 0) readonly buffer testStorageBuffer {
    float[] testArr;
};
layout (binding = 1, set = 0) readonly buffer testStorageBufferStruct {
    S[] testArrStruct;
};
float unsizedInParameterA_ff(float x[]) {
    return x[0];
}
float unsizedInParameterB_fS(S x[]) {
    return x[0].y;
}
float unsizedInParameterC_ff(float x[]) {
    return x[0];
}
float unsizedInParameterD_fS(S x[]) {
    return x[0].y;
}
float unsizedInParameterE_ff(float _skAnonymousParam0[]) {
    return 0.0;
}
float unsizedInParameterF_fS(S _skAnonymousParam0[]) {
    return 0.0;
}
vec4 getColor_h4f(float arr[]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
vec4 getColor_helper_h4f(float arr[]) {
    return getColor_h4f(arr);
}
void main() {
    sk_FragColor = getColor_helper_h4f(testArr);
    unsizedInParameterA_ff(testArr);
    unsizedInParameterB_fS(testArrStruct);
    unsizedInParameterC_ff(testArr);
    unsizedInParameterD_fS(testArrStruct);
    unsizedInParameterE_ff(testArr);
    unsizedInParameterF_fS(testArrStruct);
}
