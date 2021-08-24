
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 valueIsNaN = 0.0 / testInputs.yyyy;
    vec4 valueIsNumber = 1.0 / testInputs;
    return ((((((isnan(valueIsNaN.x) && all(isnan(valueIsNaN.xy))) && all(isnan(valueIsNaN.xyz))) && all(isnan(valueIsNaN))) && !isnan(valueIsNumber.x)) && !any(isnan(valueIsNumber.xy))) && !any(isnan(valueIsNumber.xyz))) && !any(isnan(valueIsNumber)) ? colorGreen : colorRed;
}
