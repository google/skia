
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
vec4 MatrixEffect_Stage1_c0_c0(vec4 _input, vec2 _coords) {
    vec4 _output;
    vec4 _0_TextureEffect_Stage1_c0_c0_c0;
    vec2 _1_coords = (umatrix_Stage1_c0_c0 * vec3(_coords, 1.0)).xy;
    vec4 _2_output;
    vec2 _3_inCoord = _1_coords;
    _3_inCoord *= unorm_Stage1_c0_c0_c0.xy;
    vec2 _4_subsetCoord;
    _4_subsetCoord.x = _3_inCoord.x;
    _4_subsetCoord.y = _3_inCoord.y;
    vec2 _5_clampedCoord;
    _5_clampedCoord = _4_subsetCoord;
    vec4 _6_textureColor = texture(uTextureSampler_0_Stage1, _5_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
    float _7_snappedX = floor(_3_inCoord.x + 0.0010000000474974513) + 0.5;
    if (_7_snappedX < usubset_Stage1_c0_c0_c0.x || _7_snappedX > usubset_Stage1_c0_c0_c0.z) {
        _6_textureColor = uborder_Stage1_c0_c0_c0;
    }
    return _6_textureColor;

}
void main() {
    vec4 outputColor_Stage0;
    vec4 outputCoverage_Stage0;
    {
        outputColor_Stage0 = vec4(1.0);
        outputCoverage_Stage0 = vec4(1.0);
    }
    vec4 output_Stage1;
    vec4 _8_GaussianConvolution_Stage1_c0;
    vec4 _9_output;
    _9_output = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 _10_coord = vLocalCoord_Stage0 - 12.0 * uIncrement_Stage1_c0;
    vec2 _11_coordSampled = vec2(0.0, 0.0);
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[0].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[0].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[0].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[0].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[1].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[1].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[1].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[1].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[2].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[2].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[2].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[2].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[3].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[3].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[3].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[3].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[4].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[4].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[4].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[4].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[5].x;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[5].y;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[5].z;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[5].w;
    _10_coord += uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(outputColor_Stage0, _11_coordSampled) * uKernel_Stage1_c0[6].x;
    _10_coord += uIncrement_Stage1_c0;
    _9_output *= outputColor_Stage0;
    output_Stage1 = _9_output;

    {
        sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
}
