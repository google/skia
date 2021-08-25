
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 infiniteValue = vec4(testMatrix2x2) / colorGreen.x;
    vec4 finiteValue = vec4(testMatrix2x2) / colorGreen.y;
    return ((((((isinf(infiniteValue.x) && all(isinf(infiniteValue.xy))) && all(isinf(infiniteValue.xyz))) && all(isinf(infiniteValue))) && !isinf(finiteValue.x)) && !any(isinf(finiteValue.xy))) && !any(isinf(finiteValue.xyz))) && !any(isinf(finiteValue)) ? colorGreen : colorRed;
}
