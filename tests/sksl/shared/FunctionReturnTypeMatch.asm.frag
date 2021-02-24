OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %returns_float2 "returns_float2"
OpName %returns_float3 "returns_float3"
OpName %returns_float4 "returns_float4"
OpName %returns_float2x2 "returns_float2x2"
OpName %returns_float3x3 "returns_float3x3"
OpName %returns_float4x4 "returns_float4x4"
OpName %returns_half "returns_half"
OpName %returns_half2 "returns_half2"
OpName %returns_half3 "returns_half3"
OpName %returns_half4 "returns_half4"
OpName %returns_half2x2 "returns_half2x2"
OpName %returns_half3x3 "returns_half3x3"
OpName %returns_half4x4 "returns_half4x4"
OpName %returns_bool "returns_bool"
OpName %returns_bool2 "returns_bool2"
OpName %returns_bool3 "returns_bool3"
OpName %returns_bool4 "returns_bool4"
OpName %returns_int "returns_int"
OpName %returns_int2 "returns_int2"
OpName %returns_int3 "returns_int3"
OpName %returns_int4 "returns_int4"
OpName %main "main"
OpName %_0_returns_float "_0_returns_float"
OpName %x1 "x1"
OpName %_1_returns_float2 "_1_returns_float2"
OpName %x2 "x2"
OpName %_2_returns_float3 "_2_returns_float3"
OpName %x3 "x3"
OpName %_3_returns_float4 "_3_returns_float4"
OpName %x4 "x4"
OpName %_4_returns_float2x2 "_4_returns_float2x2"
OpName %x5 "x5"
OpName %_5_returns_float3x3 "_5_returns_float3x3"
OpName %x6 "x6"
OpName %_6_returns_float4x4 "_6_returns_float4x4"
OpName %x7 "x7"
OpName %_7_returns_half "_7_returns_half"
OpName %x8 "x8"
OpName %_8_returns_half2 "_8_returns_half2"
OpName %x9 "x9"
OpName %_9_returns_half3 "_9_returns_half3"
OpName %x10 "x10"
OpName %_10_returns_half4 "_10_returns_half4"
OpName %x11 "x11"
OpName %_11_returns_half2x2 "_11_returns_half2x2"
OpName %x12 "x12"
OpName %_12_returns_half3x3 "_12_returns_half3x3"
OpName %x13 "x13"
OpName %_13_returns_half4x4 "_13_returns_half4x4"
OpName %x14 "x14"
OpName %_14_returns_bool "_14_returns_bool"
OpName %x15 "x15"
OpName %_15_returns_bool2 "_15_returns_bool2"
OpName %x16 "x16"
OpName %_16_returns_bool3 "_16_returns_bool3"
OpName %x17 "x17"
OpName %_17_returns_bool4 "_17_returns_bool4"
OpName %x18 "x18"
OpName %_18_returns_int "_18_returns_int"
OpName %x19 "x19"
OpName %_19_returns_int2 "_19_returns_int2"
OpName %x20 "x20"
OpName %_20_returns_int3 "_20_returns_int3"
OpName %x21 "x21"
OpName %_21_returns_int4 "_21_returns_int4"
OpName %x22 "x22"
OpName %_22_returns_float "_22_returns_float"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%40 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%43 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%45 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%48 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%49 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%52 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%54 = OpTypeFunction %mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%61 = OpTypeFunction %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%68 = OpTypeFunction %mat4v4float
%75 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%96 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%100 = OpTypeFunction %v2bool
%102 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%104 = OpTypeFunction %v3bool
%106 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%108 = OpTypeFunction %v4bool
%110 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%112 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%116 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%119 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%121 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%124 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%126 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%129 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_v4int = OpTypePointer Function %v4int
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %36
%37 = OpLabel
%38 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
%returns_float2 = OpFunction %v2float None %40
%41 = OpLabel
OpReturnValue %43
OpFunctionEnd
%returns_float3 = OpFunction %v3float None %45
%46 = OpLabel
OpReturnValue %48
OpFunctionEnd
%returns_float4 = OpFunction %v4float None %49
%50 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_float2x2 = OpFunction %mat2v2float None %54
%55 = OpLabel
%58 = OpCompositeConstruct %v2float %float_2 %float_0
%59 = OpCompositeConstruct %v2float %float_0 %float_2
%56 = OpCompositeConstruct %mat2v2float %58 %59
OpReturnValue %56
OpFunctionEnd
%returns_float3x3 = OpFunction %mat3v3float None %61
%62 = OpLabel
%64 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%65 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%66 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%63 = OpCompositeConstruct %mat3v3float %64 %65 %66
OpReturnValue %63
OpFunctionEnd
%returns_float4x4 = OpFunction %mat4v4float None %68
%69 = OpLabel
%71 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%74 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%70 = OpCompositeConstruct %mat4v4float %71 %72 %73 %74
OpReturnValue %70
OpFunctionEnd
%returns_half = OpFunction %float None %75
%76 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2 = OpFunction %v2float None %40
%78 = OpLabel
OpReturnValue %43
OpFunctionEnd
%returns_half3 = OpFunction %v3float None %45
%79 = OpLabel
OpReturnValue %48
OpFunctionEnd
%returns_half4 = OpFunction %v4float None %49
%80 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_half2x2 = OpFunction %mat2v2float None %54
%81 = OpLabel
%83 = OpCompositeConstruct %v2float %float_2 %float_0
%84 = OpCompositeConstruct %v2float %float_0 %float_2
%82 = OpCompositeConstruct %mat2v2float %83 %84
OpReturnValue %82
OpFunctionEnd
%returns_half3x3 = OpFunction %mat3v3float None %61
%85 = OpLabel
%87 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%88 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%89 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%86 = OpCompositeConstruct %mat3v3float %87 %88 %89
OpReturnValue %86
OpFunctionEnd
%returns_half4x4 = OpFunction %mat4v4float None %68
%90 = OpLabel
%92 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%93 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%94 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%95 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%91 = OpCompositeConstruct %mat4v4float %92 %93 %94 %95
OpReturnValue %91
OpFunctionEnd
%returns_bool = OpFunction %bool None %96
%97 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2 = OpFunction %v2bool None %100
%101 = OpLabel
OpReturnValue %102
OpFunctionEnd
%returns_bool3 = OpFunction %v3bool None %104
%105 = OpLabel
OpReturnValue %106
OpFunctionEnd
%returns_bool4 = OpFunction %v4bool None %108
%109 = OpLabel
OpReturnValue %110
OpFunctionEnd
%returns_int = OpFunction %int None %112
%113 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2 = OpFunction %v2int None %116
%117 = OpLabel
OpReturnValue %119
OpFunctionEnd
%returns_int3 = OpFunction %v3int None %121
%122 = OpLabel
OpReturnValue %124
OpFunctionEnd
%returns_int4 = OpFunction %v4int None %126
%127 = OpLabel
OpReturnValue %129
OpFunctionEnd
%main = OpFunction %v4float None %49
%130 = OpLabel
%_0_returns_float = OpVariable %_ptr_Function_float Function
%x1 = OpVariable %_ptr_Function_float Function
%_1_returns_float2 = OpVariable %_ptr_Function_v2float Function
%x2 = OpVariable %_ptr_Function_v2float Function
%_2_returns_float3 = OpVariable %_ptr_Function_v3float Function
%x3 = OpVariable %_ptr_Function_v3float Function
%_3_returns_float4 = OpVariable %_ptr_Function_v4float Function
%x4 = OpVariable %_ptr_Function_v4float Function
%_4_returns_float2x2 = OpVariable %_ptr_Function_mat2v2float Function
%x5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_returns_float3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x6 = OpVariable %_ptr_Function_mat3v3float Function
%_6_returns_float4x4 = OpVariable %_ptr_Function_mat4v4float Function
%x7 = OpVariable %_ptr_Function_mat4v4float Function
%_7_returns_half = OpVariable %_ptr_Function_float Function
%x8 = OpVariable %_ptr_Function_float Function
%_8_returns_half2 = OpVariable %_ptr_Function_v2float Function
%x9 = OpVariable %_ptr_Function_v2float Function
%_9_returns_half3 = OpVariable %_ptr_Function_v3float Function
%x10 = OpVariable %_ptr_Function_v3float Function
%_10_returns_half4 = OpVariable %_ptr_Function_v4float Function
%x11 = OpVariable %_ptr_Function_v4float Function
%_11_returns_half2x2 = OpVariable %_ptr_Function_mat2v2float Function
%x12 = OpVariable %_ptr_Function_mat2v2float Function
%_12_returns_half3x3 = OpVariable %_ptr_Function_mat3v3float Function
%x13 = OpVariable %_ptr_Function_mat3v3float Function
%_13_returns_half4x4 = OpVariable %_ptr_Function_mat4v4float Function
%x14 = OpVariable %_ptr_Function_mat4v4float Function
%_14_returns_bool = OpVariable %_ptr_Function_bool Function
%x15 = OpVariable %_ptr_Function_bool Function
%_15_returns_bool2 = OpVariable %_ptr_Function_v2bool Function
%x16 = OpVariable %_ptr_Function_v2bool Function
%_16_returns_bool3 = OpVariable %_ptr_Function_v3bool Function
%x17 = OpVariable %_ptr_Function_v3bool Function
%_17_returns_bool4 = OpVariable %_ptr_Function_v4bool Function
%x18 = OpVariable %_ptr_Function_v4bool Function
%_18_returns_int = OpVariable %_ptr_Function_int Function
%x19 = OpVariable %_ptr_Function_int Function
%_19_returns_int2 = OpVariable %_ptr_Function_v2int Function
%x20 = OpVariable %_ptr_Function_v2int Function
%_20_returns_int3 = OpVariable %_ptr_Function_v3int Function
%x21 = OpVariable %_ptr_Function_v3int Function
%_21_returns_int4 = OpVariable %_ptr_Function_v4int Function
%x22 = OpVariable %_ptr_Function_v4int Function
%_22_returns_float = OpVariable %_ptr_Function_float Function
%434 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %43
OpStore %x3 %48
OpStore %x4 %52
%147 = OpCompositeConstruct %v2float %float_2 %float_0
%148 = OpCompositeConstruct %v2float %float_0 %float_2
%146 = OpCompositeConstruct %mat2v2float %147 %148
OpStore %x5 %146
%153 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
OpStore %x6 %152
%160 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%161 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%162 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%163 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%159 = OpCompositeConstruct %mat4v4float %160 %161 %162 %163
OpStore %x7 %159
OpStore %x8 %float_1
OpStore %x9 %43
OpStore %x10 %48
OpStore %x11 %52
%175 = OpCompositeConstruct %v2float %float_2 %float_0
%176 = OpCompositeConstruct %v2float %float_0 %float_2
%174 = OpCompositeConstruct %mat2v2float %175 %176
OpStore %x12 %174
%180 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%181 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%182 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%179 = OpCompositeConstruct %mat3v3float %180 %181 %182
OpStore %x13 %179
%186 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%187 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%188 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%189 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%185 = OpCompositeConstruct %mat4v4float %186 %187 %188 %189
OpStore %x14 %185
OpStore %x15 %true
OpStore %x16 %102
OpStore %x17 %106
OpStore %x18 %110
OpStore %x19 %int_1
OpStore %x20 %119
OpStore %x21 %124
OpStore %x22 %129
%216 = OpLoad %float %x1
%217 = OpFOrdEqual %bool %216 %float_1
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%220 = OpLoad %v2float %x2
%221 = OpFunctionCall %v2float %returns_float2
%222 = OpFOrdEqual %v2bool %220 %221
%223 = OpAll %bool %222
OpBranch %219
%219 = OpLabel
%224 = OpPhi %bool %false %130 %223 %218
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%227 = OpLoad %v3float %x3
%228 = OpFunctionCall %v3float %returns_float3
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
OpBranch %226
%226 = OpLabel
%231 = OpPhi %bool %false %219 %230 %225
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %v4float %x4
%235 = OpFunctionCall %v4float %returns_float4
%236 = OpFOrdEqual %v4bool %234 %235
%237 = OpAll %bool %236
OpBranch %233
%233 = OpLabel
%238 = OpPhi %bool %false %226 %237 %232
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpLoad %mat2v2float %x5
%242 = OpFunctionCall %mat2v2float %returns_float2x2
%243 = OpCompositeExtract %v2float %241 0
%244 = OpCompositeExtract %v2float %242 0
%245 = OpFOrdEqual %v2bool %243 %244
%246 = OpAll %bool %245
%247 = OpCompositeExtract %v2float %241 1
%248 = OpCompositeExtract %v2float %242 1
%249 = OpFOrdEqual %v2bool %247 %248
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %246 %250
OpBranch %240
%240 = OpLabel
%252 = OpPhi %bool %false %233 %251 %239
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %mat3v3float %x6
%256 = OpFunctionCall %mat3v3float %returns_float3x3
%257 = OpCompositeExtract %v3float %255 0
%258 = OpCompositeExtract %v3float %256 0
%259 = OpFOrdEqual %v3bool %257 %258
%260 = OpAll %bool %259
%261 = OpCompositeExtract %v3float %255 1
%262 = OpCompositeExtract %v3float %256 1
%263 = OpFOrdEqual %v3bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
%266 = OpCompositeExtract %v3float %255 2
%267 = OpCompositeExtract %v3float %256 2
%268 = OpFOrdEqual %v3bool %266 %267
%269 = OpAll %bool %268
%270 = OpLogicalAnd %bool %265 %269
OpBranch %254
%254 = OpLabel
%271 = OpPhi %bool %false %240 %270 %253
OpSelectionMerge %273 None
OpBranchConditional %271 %272 %273
%272 = OpLabel
%274 = OpLoad %mat4v4float %x7
%275 = OpFunctionCall %mat4v4float %returns_float4x4
%276 = OpCompositeExtract %v4float %274 0
%277 = OpCompositeExtract %v4float %275 0
%278 = OpFOrdEqual %v4bool %276 %277
%279 = OpAll %bool %278
%280 = OpCompositeExtract %v4float %274 1
%281 = OpCompositeExtract %v4float %275 1
%282 = OpFOrdEqual %v4bool %280 %281
%283 = OpAll %bool %282
%284 = OpLogicalAnd %bool %279 %283
%285 = OpCompositeExtract %v4float %274 2
%286 = OpCompositeExtract %v4float %275 2
%287 = OpFOrdEqual %v4bool %285 %286
%288 = OpAll %bool %287
%289 = OpLogicalAnd %bool %284 %288
%290 = OpCompositeExtract %v4float %274 3
%291 = OpCompositeExtract %v4float %275 3
%292 = OpFOrdEqual %v4bool %290 %291
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %289 %293
OpBranch %273
%273 = OpLabel
%295 = OpPhi %bool %false %254 %294 %272
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %float %x8
%299 = OpFunctionCall %float %returns_half
%300 = OpFOrdEqual %bool %298 %299
OpBranch %297
%297 = OpLabel
%301 = OpPhi %bool %false %273 %300 %296
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpLoad %v2float %x9
%305 = OpFunctionCall %v2float %returns_half2
%306 = OpFOrdEqual %v2bool %304 %305
%307 = OpAll %bool %306
OpBranch %303
%303 = OpLabel
%308 = OpPhi %bool %false %297 %307 %302
OpSelectionMerge %310 None
OpBranchConditional %308 %309 %310
%309 = OpLabel
%311 = OpLoad %v3float %x10
%312 = OpFunctionCall %v3float %returns_half3
%313 = OpFOrdEqual %v3bool %311 %312
%314 = OpAll %bool %313
OpBranch %310
%310 = OpLabel
%315 = OpPhi %bool %false %303 %314 %309
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%318 = OpLoad %v4float %x11
%319 = OpFunctionCall %v4float %returns_half4
%320 = OpFOrdEqual %v4bool %318 %319
%321 = OpAll %bool %320
OpBranch %317
%317 = OpLabel
%322 = OpPhi %bool %false %310 %321 %316
OpSelectionMerge %324 None
OpBranchConditional %322 %323 %324
%323 = OpLabel
%325 = OpLoad %mat2v2float %x12
%326 = OpFunctionCall %mat2v2float %returns_half2x2
%327 = OpCompositeExtract %v2float %325 0
%328 = OpCompositeExtract %v2float %326 0
%329 = OpFOrdEqual %v2bool %327 %328
%330 = OpAll %bool %329
%331 = OpCompositeExtract %v2float %325 1
%332 = OpCompositeExtract %v2float %326 1
%333 = OpFOrdEqual %v2bool %331 %332
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %330 %334
OpBranch %324
%324 = OpLabel
%336 = OpPhi %bool %false %317 %335 %323
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %mat3v3float %x13
%340 = OpFunctionCall %mat3v3float %returns_half3x3
%341 = OpCompositeExtract %v3float %339 0
%342 = OpCompositeExtract %v3float %340 0
%343 = OpFOrdEqual %v3bool %341 %342
%344 = OpAll %bool %343
%345 = OpCompositeExtract %v3float %339 1
%346 = OpCompositeExtract %v3float %340 1
%347 = OpFOrdEqual %v3bool %345 %346
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %344 %348
%350 = OpCompositeExtract %v3float %339 2
%351 = OpCompositeExtract %v3float %340 2
%352 = OpFOrdEqual %v3bool %350 %351
%353 = OpAll %bool %352
%354 = OpLogicalAnd %bool %349 %353
OpBranch %338
%338 = OpLabel
%355 = OpPhi %bool %false %324 %354 %337
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%358 = OpLoad %mat4v4float %x14
%359 = OpFunctionCall %mat4v4float %returns_half4x4
%360 = OpCompositeExtract %v4float %358 0
%361 = OpCompositeExtract %v4float %359 0
%362 = OpFOrdEqual %v4bool %360 %361
%363 = OpAll %bool %362
%364 = OpCompositeExtract %v4float %358 1
%365 = OpCompositeExtract %v4float %359 1
%366 = OpFOrdEqual %v4bool %364 %365
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %363 %367
%369 = OpCompositeExtract %v4float %358 2
%370 = OpCompositeExtract %v4float %359 2
%371 = OpFOrdEqual %v4bool %369 %370
%372 = OpAll %bool %371
%373 = OpLogicalAnd %bool %368 %372
%374 = OpCompositeExtract %v4float %358 3
%375 = OpCompositeExtract %v4float %359 3
%376 = OpFOrdEqual %v4bool %374 %375
%377 = OpAll %bool %376
%378 = OpLogicalAnd %bool %373 %377
OpBranch %357
%357 = OpLabel
%379 = OpPhi %bool %false %338 %378 %356
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %bool %x15
%383 = OpFunctionCall %bool %returns_bool
%384 = OpLogicalEqual %bool %382 %383
OpBranch %381
%381 = OpLabel
%385 = OpPhi %bool %false %357 %384 %380
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpLoad %v2bool %x16
%389 = OpFunctionCall %v2bool %returns_bool2
%390 = OpLogicalEqual %v2bool %388 %389
%391 = OpAll %bool %390
OpBranch %387
%387 = OpLabel
%392 = OpPhi %bool %false %381 %391 %386
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpLoad %v3bool %x17
%396 = OpFunctionCall %v3bool %returns_bool3
%397 = OpLogicalEqual %v3bool %395 %396
%398 = OpAll %bool %397
OpBranch %394
%394 = OpLabel
%399 = OpPhi %bool %false %387 %398 %393
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%402 = OpLoad %v4bool %x18
%403 = OpFunctionCall %v4bool %returns_bool4
%404 = OpLogicalEqual %v4bool %402 %403
%405 = OpAll %bool %404
OpBranch %401
%401 = OpLabel
%406 = OpPhi %bool %false %394 %405 %400
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpLoad %int %x19
%410 = OpFunctionCall %int %returns_int
%411 = OpIEqual %bool %409 %410
OpBranch %408
%408 = OpLabel
%412 = OpPhi %bool %false %401 %411 %407
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpLoad %v2int %x20
%416 = OpFunctionCall %v2int %returns_int2
%417 = OpIEqual %v2bool %415 %416
%418 = OpAll %bool %417
OpBranch %414
%414 = OpLabel
%419 = OpPhi %bool %false %408 %418 %413
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%422 = OpLoad %v3int %x21
%423 = OpFunctionCall %v3int %returns_int3
%424 = OpIEqual %v3bool %422 %423
%425 = OpAll %bool %424
OpBranch %421
%421 = OpLabel
%426 = OpPhi %bool %false %414 %425 %420
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%429 = OpLoad %v4int %x22
%430 = OpFunctionCall %v4int %returns_int4
%431 = OpIEqual %v4bool %429 %430
%432 = OpAll %bool %431
OpBranch %428
%428 = OpLabel
%433 = OpPhi %bool %false %421 %432 %427
OpSelectionMerge %437 None
OpBranchConditional %433 %435 %436
%435 = OpLabel
%438 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%441 = OpLoad %v4float %438
OpStore %434 %441
OpBranch %437
%436 = OpLabel
%442 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%443 = OpLoad %v4float %442
OpStore %434 %443
OpBranch %437
%437 = OpLabel
%444 = OpLoad %v4float %434
OpReturnValue %444
OpFunctionEnd
