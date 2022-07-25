
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    vec4 _tempVec0;
    vec4 _tempVec1;
    bool ok = true;
    ok = ok && mat2(testInputs.xy, testInputs.zw) == mat2(-1.25, 0.0, 0.75, 2.25);
    ok = ok && mat2(testInputs.xy, testInputs.zw) == mat2(-1.25, 0.0, 0.75, 2.25);
    ok = ok && mat2(colorGreen.xy, colorGreen.zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen.xy, colorGreen.zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(vec4(ivec4(colorGreen)).xy, vec4(ivec4(colorGreen)).zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen.xy, colorGreen.zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen.xy, colorGreen.zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(vec4(bvec4(colorGreen)).xy, vec4(bvec4(colorGreen)).zw) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && ((_tempVec0 = colorGreen - colorRed), mat2(_tempVec0.xy, _tempVec0.zw)) == mat2(-1.0, 1.0, 0.0, 0.0);
    ok = ok && ((_tempVec1 = colorGreen + 5.0), mat2(_tempVec1.xy, _tempVec1.zw)) == mat2(5.0, 6.0, 5.0, 6.0);
    return ok ? colorGreen : colorRed;
}
