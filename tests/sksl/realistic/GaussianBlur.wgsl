diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSIn {
  @location(0) vLocalCoord_Stage0: vec2<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct uniformBuffer {
  @size(16) sk_RTAdjust: vec4<f32>,
  @size(16) uIncrement_Stage1_c0: vec2<f16>,
  @size(112) uKernel_Stage1_c0: array<_skArrayElement_h4, 7>,
  @size(48) umatrix_Stage1_c0_c0: mat3x3<f32>,
  @size(16) uborder_Stage1_c0_c0_c0: vec4<f16>,
  @size(16) usubset_Stage1_c0_c0_c0: vec4<f32>,
  unorm_Stage1_c0_c0_c0: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _uniform0 : uniformBuffer;
@group(0) @binding(10001) var uTextureSampler_0_Stage1_Sampler: sampler;
@group(0) @binding(10002) var uTextureSampler_0_Stage1_Texture: texture_2d<f32>;
fn MatrixEffect_Stage1_c0_c0_h4h4f2(_input: vec4<f16>, _coords: vec2<f32>) -> vec4<f16> {
  {
    var _1_inCoord: vec2<f32> = (_uniform0.umatrix_Stage1_c0_c0 * vec3<f32>(_coords, 1.0)).xy;
    _1_inCoord = _1_inCoord * _uniform0.unorm_Stage1_c0_c0_c0.xy;
    var _2_subsetCoord: vec2<f32>;
    _2_subsetCoord.x = _1_inCoord.x;
    _2_subsetCoord.y = _1_inCoord.y;
    let _3_clampedCoord: vec2<f32> = _2_subsetCoord;
    var _4_textureColor: vec4<f16> = vec4<f16>(textureSample(uTextureSampler_0_Stage1_Texture, uTextureSampler_0_Stage1_Sampler, _3_clampedCoord * _uniform0.unorm_Stage1_c0_c0_c0.zw));
    let _5_snappedX: f32 = floor(_1_inCoord.x + 0.001) + 0.5;
    if (_5_snappedX < _uniform0.usubset_Stage1_c0_c0_c0.x) || (_5_snappedX > _uniform0.usubset_Stage1_c0_c0_c0.z) {
      {
        _4_textureColor = _uniform0.uborder_Stage1_c0_c0_c0;
      }
    }
    return _4_textureColor;
  }
}
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    var outputColor_Stage0: vec4<f16>;
    var outputCoverage_Stage0: vec4<f16>;
    {
      outputColor_Stage0 = vec4<f16>(1.0h);
      outputCoverage_Stage0 = vec4<f16>(1.0h);
    }
    var _6_output: vec4<f16> = vec4<f16>(0.0h);
    var _7_coord: vec2<f32> = _stageIn.vLocalCoord_Stage0 - vec2<f32>(12.0h * _uniform0.uIncrement_Stage1_c0);
    var _8_coordSampled: vec2<f32> = vec2<f32>(0.0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[0].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[0].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[0].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[0].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[1].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[1].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[1].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[1].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[2].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[2].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[2].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[2].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[3].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[3].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[3].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[3].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[4].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[4].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[4].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[4].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[5].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[5].y;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[5].z;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[5].w;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output = _6_output + MatrixEffect_Stage1_c0_c0_h4h4f2(outputColor_Stage0, _8_coordSampled) * _skUnpacked__uniform0_uKernel_Stage1_c0[6].x;
    _7_coord = _7_coord + vec2<f32>(_uniform0.uIncrement_Stage1_c0);
    _6_output = _6_output * outputColor_Stage0;
    let output_Stage1: vec4<f16> = _6_output;
    {
      (*_stageOut).sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
struct _skArrayElement_h4 {
  @align(16) e : vec4<f16>
};
var<private> _skUnpacked__uniform0_uKernel_Stage1_c0: array<vec4<f16>, 7>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__uniform0_uKernel_Stage1_c0 = array<vec4<f16>, 7>(_uniform0.uKernel_Stage1_c0[0].e, _uniform0.uKernel_Stage1_c0[1].e, _uniform0.uKernel_Stage1_c0[2].e, _uniform0.uKernel_Stage1_c0[3].e, _uniform0.uKernel_Stage1_c0[4].e, _uniform0.uKernel_Stage1_c0[5].e, _uniform0.uKernel_Stage1_c0[6].e);
}
