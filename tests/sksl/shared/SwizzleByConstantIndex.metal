struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float4 constant_swizzle(Uniforms _uniforms) {
    float4 v = _uniforms.testInputs;
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    return float4(x, y, z, w);
}
float4 foldable_index(Uniforms _uniforms) {
    const int ZERO = 0;
    const int ONE = 1;
    const int TWO = 2;
    const int THREE = 3;
    float x = _uniforms.testInputs.x;
    float y = _uniforms.testInputs.y;
    float z = _uniforms.testInputs.z;
    float w = _uniforms.testInputs.w;
    return float4(x, y, z, w);
}
float4 foldable() {
    float4 v = float4(0.0, 1.0, 2.0, 3.0);
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    return float4(x, y, z, w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 a = constant_swizzle(_uniforms);
    float4 b = foldable_index(_uniforms);
    float4 c = foldable();
    _out.sk_FragColor = (all(a == float4(-1.25, 0.0, 0.75, 2.25)) && all(b == float4(-1.25, 0.0, 0.75, 2.25))) && all(c == float4(0.0, 1.0, 2.0, 3.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
