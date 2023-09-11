
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D uTextureSampler_0_Stage1;
layout (binding = 0) uniform uniformBuffer {
    layout (offset = 0) vec4 sk_RTAdjust;
    layout (offset = 16) vec2 uIncrement_Stage1_c0;
    layout (offset = 32) vec4[7] uKernel_Stage1_c0;
    layout (offset = 144) mat3 umatrix_Stage1_c0_c0;
    layout (offset = 192) vec4 uborder_Stage1_c0_c0_c0;
    layout (offset = 208) vec4 usubset_Stage1_c0_c0_c0;
    layout (offset = 224) vec4 unorm_Stage1_c0_c0_c0;
};
layout (location = 0) in vec2 vLocalCoord_Stage0;
vec4 MatrixEffect_Stage1_c0_c0_h4h4f2(vec4 _input, vec2 _coords) {
    vec2 _1_inCoord = (umatrix_Stage1_c0_c0 * vec3(_coords, 1.0)).xy;
    _1_inCoord *= unorm_Stage1_c0_c0_c0.xy;
    vec2 _2_subsetCoord;
    _2_subsetCoord.x = _1_inCoord.x;
    _2_subsetCoord.y = _1_inCoord.y;
    vec2 _3_clampedCoord = _2_subsetCoord;
    vec4 _4_textureColor = texture(uTextureSampler_0_Stage1, _3_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
    float _5_snappedX = floor(_1_inCoord.x + 0.001) + 0.5;
    if (_5_snappedX < usubset_Stage1_c0_c0_c0.x || _5_snappedX > usubset_Stage1_c0_c0_c0.z) {
        _4_textureColor = uborder_Stage1_c0_c0_c0;
    }
    return _4_textureColor;
}
void main() {
    vec4 outputColor_Stage0;
    vec4 outputCoverage_Stage0;
    {
        outputColor_Stage0 = vec4(1.0);
        outputCoverage_Stage0 = vec4(1.0);
    }
    vec4 _6_output = vec4(0.0);
    vec2 _7_coord = vLocalCoord_Stage0 - 12.0 * uIncrement_Stage1_c0;
    vec2 _8_coordSampled = vec2(0.0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[0].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[0].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[0].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[0].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[1].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[1].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[1].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[1].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[2].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[2].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[2].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[2].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[3].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[3].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[3].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[3].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[4].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[4].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[4].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[4].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[5].x;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[5].y;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[5].z;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[5].w;
    _7_coord += uIncrement_Stage1_c0;
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * uKernel_Stage1_c0[6].x;
    _7_coord += uIncrement_Stage1_c0;
    _6_output *= outputColor_Stage0;
    vec4 output_Stage1 = _6_output;
    {
        sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
}
