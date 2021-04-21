OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %returns_float2_f2 "returns_float2_f2"
OpName %returns_float3_f3 "returns_float3_f3"
OpName %returns_float4_f4 "returns_float4_f4"
OpName %returns_float2x2_f22 "returns_float2x2_f22"
OpName %returns_float3x3_f33 "returns_float3x3_f33"
OpName %returns_float4x4_f44 "returns_float4x4_f44"
OpName %returns_half_h "returns_half_h"
OpName %returns_half2_h2 "returns_half2_h2"
OpName %returns_half3_h3 "returns_half3_h3"
OpName %returns_half4_h4 "returns_half4_h4"
OpName %returns_half2x2_h22 "returns_half2x2_h22"
OpName %returns_half3x3_h33 "returns_half3x3_h33"
OpName %returns_half4x4_h44 "returns_half4x4_h44"
OpName %returns_bool_b "returns_bool_b"
OpName %returns_bool2_b2 "returns_bool2_b2"
OpName %returns_bool3_b3 "returns_bool3_b3"
OpName %returns_bool4_b4 "returns_bool4_b4"
OpName %returns_int_i "returns_int_i"
OpName %returns_int2_i2 "returns_int2_i2"
OpName %returns_int3_i3 "returns_int3_i3"
OpName %returns_int4_i4 "returns_int4_i4"
OpName %main "main"
OpName %x1 "x1"
OpName %x2 "x2"
OpName %x3 "x3"
OpName %x4 "x4"
OpName %x5 "x5"
OpName %x6 "x6"
OpName %x7 "x7"
OpName %x8 "x8"
OpName %x9 "x9"
OpName %x10 "x10"
OpName %x11 "x11"
OpName %x12 "x12"
OpName %x13 "x13"
OpName %x14 "x14"
OpName %x15 "x15"
OpName %x16 "x16"
OpName %x17 "x17"
OpName %x18 "x18"
OpName %x19 "x19"
OpName %x20 "x20"
OpName %x21 "x21"
OpName %x22 "x22"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%40 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%44 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%47 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%49 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%53 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%56 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%58 = OpTypeFunction %mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%64 = OpTypeFunction %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%71 = OpTypeFunction %mat4v4float
%78 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%99 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%103 = OpTypeFunction %v2bool
%105 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%107 = OpTypeFunction %v3bool
%109 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%111 = OpTypeFunction %v4bool
%113 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%115 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%119 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%122 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%124 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%127 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%129 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%132 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%133 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
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
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%41 = OpVariable %_ptr_Function_v2float Function
OpStore %41 %40
%43 = OpFunctionCall %v4float %main %41
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %44
%45 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %49
%50 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %53
%54 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %58
%59 = OpLabel
%61 = OpCompositeConstruct %v2float %float_2 %float_0
%62 = OpCompositeConstruct %v2float %float_0 %float_2
%60 = OpCompositeConstruct %mat2v2float %61 %62
OpReturnValue %60
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
%65 = OpLabel
%67 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%68 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%69 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%66 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpReturnValue %66
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
%72 = OpLabel
%74 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%75 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%76 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%73 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpReturnValue %73
OpFunctionEnd
%returns_half_h = OpFunction %float None %78
%79 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %44
%81 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %49
%82 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %53
%83 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %58
%84 = OpLabel
%86 = OpCompositeConstruct %v2float %float_2 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_2
%85 = OpCompositeConstruct %mat2v2float %86 %87
OpReturnValue %85
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
%88 = OpLabel
%90 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%91 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%92 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%89 = OpCompositeConstruct %mat3v3float %90 %91 %92
OpReturnValue %89
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
%93 = OpLabel
%95 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%96 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%97 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%98 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%94 = OpCompositeConstruct %mat4v4float %95 %96 %97 %98
OpReturnValue %94
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %99
%100 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %103
%104 = OpLabel
OpReturnValue %105
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %107
%108 = OpLabel
OpReturnValue %109
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %111
%112 = OpLabel
OpReturnValue %113
OpFunctionEnd
%returns_int_i = OpFunction %int None %115
%116 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %119
%120 = OpLabel
OpReturnValue %122
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %124
%125 = OpLabel
OpReturnValue %127
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %129
%130 = OpLabel
OpReturnValue %132
OpFunctionEnd
%main = OpFunction %v4float None %133
%134 = OpFunctionParameter %_ptr_Function_v2float
%135 = OpLabel
%x1 = OpVariable %_ptr_Function_float Function
%x2 = OpVariable %_ptr_Function_v2float Function
%x3 = OpVariable %_ptr_Function_v3float Function
%x4 = OpVariable %_ptr_Function_v4float Function
%x5 = OpVariable %_ptr_Function_mat2v2float Function
%x6 = OpVariable %_ptr_Function_mat3v3float Function
%x7 = OpVariable %_ptr_Function_mat4v4float Function
%x8 = OpVariable %_ptr_Function_float Function
%x9 = OpVariable %_ptr_Function_v2float Function
%x10 = OpVariable %_ptr_Function_v3float Function
%x11 = OpVariable %_ptr_Function_v4float Function
%x12 = OpVariable %_ptr_Function_mat2v2float Function
%x13 = OpVariable %_ptr_Function_mat3v3float Function
%x14 = OpVariable %_ptr_Function_mat4v4float Function
%x15 = OpVariable %_ptr_Function_bool Function
%x16 = OpVariable %_ptr_Function_v2bool Function
%x17 = OpVariable %_ptr_Function_v3bool Function
%x18 = OpVariable %_ptr_Function_v4bool Function
%x19 = OpVariable %_ptr_Function_int Function
%x20 = OpVariable %_ptr_Function_v2int Function
%x21 = OpVariable %_ptr_Function_v3int Function
%x22 = OpVariable %_ptr_Function_v4int Function
%415 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %47
OpStore %x3 %52
OpStore %x4 %56
%146 = OpCompositeConstruct %v2float %float_2 %float_0
%147 = OpCompositeConstruct %v2float %float_0 %float_2
%145 = OpCompositeConstruct %mat2v2float %146 %147
OpStore %x5 %145
%151 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%152 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%153 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%150 = OpCompositeConstruct %mat3v3float %151 %152 %153
OpStore %x6 %150
%157 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%158 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%159 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%160 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%156 = OpCompositeConstruct %mat4v4float %157 %158 %159 %160
OpStore %x7 %156
OpStore %x8 %float_1
OpStore %x9 %47
OpStore %x10 %52
OpStore %x11 %56
%167 = OpCompositeConstruct %v2float %float_2 %float_0
%168 = OpCompositeConstruct %v2float %float_0 %float_2
%166 = OpCompositeConstruct %mat2v2float %167 %168
OpStore %x12 %166
%171 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%172 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%173 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%170 = OpCompositeConstruct %mat3v3float %171 %172 %173
OpStore %x13 %170
%176 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%177 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%178 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%179 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%175 = OpCompositeConstruct %mat4v4float %176 %177 %178 %179
OpStore %x14 %175
OpStore %x15 %true
OpStore %x16 %105
OpStore %x17 %109
OpStore %x18 %113
OpStore %x19 %int_1
OpStore %x20 %122
OpStore %x21 %127
OpStore %x22 %132
%197 = OpLoad %float %x1
%198 = OpFOrdEqual %bool %197 %float_1
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpLoad %v2float %x2
%202 = OpFunctionCall %v2float %returns_float2_f2
%203 = OpFOrdEqual %v2bool %201 %202
%204 = OpAll %bool %203
OpBranch %200
%200 = OpLabel
%205 = OpPhi %bool %false %135 %204 %199
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpLoad %v3float %x3
%209 = OpFunctionCall %v3float %returns_float3_f3
%210 = OpFOrdEqual %v3bool %208 %209
%211 = OpAll %bool %210
OpBranch %207
%207 = OpLabel
%212 = OpPhi %bool %false %200 %211 %206
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%215 = OpLoad %v4float %x4
%216 = OpFunctionCall %v4float %returns_float4_f4
%217 = OpFOrdEqual %v4bool %215 %216
%218 = OpAll %bool %217
OpBranch %214
%214 = OpLabel
%219 = OpPhi %bool %false %207 %218 %213
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%222 = OpLoad %mat2v2float %x5
%223 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%224 = OpCompositeExtract %v2float %222 0
%225 = OpCompositeExtract %v2float %223 0
%226 = OpFOrdEqual %v2bool %224 %225
%227 = OpAll %bool %226
%228 = OpCompositeExtract %v2float %222 1
%229 = OpCompositeExtract %v2float %223 1
%230 = OpFOrdEqual %v2bool %228 %229
%231 = OpAll %bool %230
%232 = OpLogicalAnd %bool %227 %231
OpBranch %221
%221 = OpLabel
%233 = OpPhi %bool %false %214 %232 %220
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpLoad %mat3v3float %x6
%237 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%238 = OpCompositeExtract %v3float %236 0
%239 = OpCompositeExtract %v3float %237 0
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpCompositeExtract %v3float %236 1
%243 = OpCompositeExtract %v3float %237 1
%244 = OpFOrdEqual %v3bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
%247 = OpCompositeExtract %v3float %236 2
%248 = OpCompositeExtract %v3float %237 2
%249 = OpFOrdEqual %v3bool %247 %248
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %246 %250
OpBranch %235
%235 = OpLabel
%252 = OpPhi %bool %false %221 %251 %234
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %mat4v4float %x7
%256 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%257 = OpCompositeExtract %v4float %255 0
%258 = OpCompositeExtract %v4float %256 0
%259 = OpFOrdEqual %v4bool %257 %258
%260 = OpAll %bool %259
%261 = OpCompositeExtract %v4float %255 1
%262 = OpCompositeExtract %v4float %256 1
%263 = OpFOrdEqual %v4bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
%266 = OpCompositeExtract %v4float %255 2
%267 = OpCompositeExtract %v4float %256 2
%268 = OpFOrdEqual %v4bool %266 %267
%269 = OpAll %bool %268
%270 = OpLogicalAnd %bool %265 %269
%271 = OpCompositeExtract %v4float %255 3
%272 = OpCompositeExtract %v4float %256 3
%273 = OpFOrdEqual %v4bool %271 %272
%274 = OpAll %bool %273
%275 = OpLogicalAnd %bool %270 %274
OpBranch %254
%254 = OpLabel
%276 = OpPhi %bool %false %235 %275 %253
OpSelectionMerge %278 None
OpBranchConditional %276 %277 %278
%277 = OpLabel
%279 = OpLoad %float %x8
%280 = OpFunctionCall %float %returns_half_h
%281 = OpFOrdEqual %bool %279 %280
OpBranch %278
%278 = OpLabel
%282 = OpPhi %bool %false %254 %281 %277
OpSelectionMerge %284 None
OpBranchConditional %282 %283 %284
%283 = OpLabel
%285 = OpLoad %v2float %x9
%286 = OpFunctionCall %v2float %returns_half2_h2
%287 = OpFOrdEqual %v2bool %285 %286
%288 = OpAll %bool %287
OpBranch %284
%284 = OpLabel
%289 = OpPhi %bool %false %278 %288 %283
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpLoad %v3float %x10
%293 = OpFunctionCall %v3float %returns_half3_h3
%294 = OpFOrdEqual %v3bool %292 %293
%295 = OpAll %bool %294
OpBranch %291
%291 = OpLabel
%296 = OpPhi %bool %false %284 %295 %290
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %v4float %x11
%300 = OpFunctionCall %v4float %returns_half4_h4
%301 = OpFOrdEqual %v4bool %299 %300
%302 = OpAll %bool %301
OpBranch %298
%298 = OpLabel
%303 = OpPhi %bool %false %291 %302 %297
OpSelectionMerge %305 None
OpBranchConditional %303 %304 %305
%304 = OpLabel
%306 = OpLoad %mat2v2float %x12
%307 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%308 = OpCompositeExtract %v2float %306 0
%309 = OpCompositeExtract %v2float %307 0
%310 = OpFOrdEqual %v2bool %308 %309
%311 = OpAll %bool %310
%312 = OpCompositeExtract %v2float %306 1
%313 = OpCompositeExtract %v2float %307 1
%314 = OpFOrdEqual %v2bool %312 %313
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %311 %315
OpBranch %305
%305 = OpLabel
%317 = OpPhi %bool %false %298 %316 %304
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%320 = OpLoad %mat3v3float %x13
%321 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%322 = OpCompositeExtract %v3float %320 0
%323 = OpCompositeExtract %v3float %321 0
%324 = OpFOrdEqual %v3bool %322 %323
%325 = OpAll %bool %324
%326 = OpCompositeExtract %v3float %320 1
%327 = OpCompositeExtract %v3float %321 1
%328 = OpFOrdEqual %v3bool %326 %327
%329 = OpAll %bool %328
%330 = OpLogicalAnd %bool %325 %329
%331 = OpCompositeExtract %v3float %320 2
%332 = OpCompositeExtract %v3float %321 2
%333 = OpFOrdEqual %v3bool %331 %332
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %330 %334
OpBranch %319
%319 = OpLabel
%336 = OpPhi %bool %false %305 %335 %318
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %mat4v4float %x14
%340 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%341 = OpCompositeExtract %v4float %339 0
%342 = OpCompositeExtract %v4float %340 0
%343 = OpFOrdEqual %v4bool %341 %342
%344 = OpAll %bool %343
%345 = OpCompositeExtract %v4float %339 1
%346 = OpCompositeExtract %v4float %340 1
%347 = OpFOrdEqual %v4bool %345 %346
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %344 %348
%350 = OpCompositeExtract %v4float %339 2
%351 = OpCompositeExtract %v4float %340 2
%352 = OpFOrdEqual %v4bool %350 %351
%353 = OpAll %bool %352
%354 = OpLogicalAnd %bool %349 %353
%355 = OpCompositeExtract %v4float %339 3
%356 = OpCompositeExtract %v4float %340 3
%357 = OpFOrdEqual %v4bool %355 %356
%358 = OpAll %bool %357
%359 = OpLogicalAnd %bool %354 %358
OpBranch %338
%338 = OpLabel
%360 = OpPhi %bool %false %319 %359 %337
OpSelectionMerge %362 None
OpBranchConditional %360 %361 %362
%361 = OpLabel
%363 = OpLoad %bool %x15
%364 = OpFunctionCall %bool %returns_bool_b
%365 = OpLogicalEqual %bool %363 %364
OpBranch %362
%362 = OpLabel
%366 = OpPhi %bool %false %338 %365 %361
OpSelectionMerge %368 None
OpBranchConditional %366 %367 %368
%367 = OpLabel
%369 = OpLoad %v2bool %x16
%370 = OpFunctionCall %v2bool %returns_bool2_b2
%371 = OpLogicalEqual %v2bool %369 %370
%372 = OpAll %bool %371
OpBranch %368
%368 = OpLabel
%373 = OpPhi %bool %false %362 %372 %367
OpSelectionMerge %375 None
OpBranchConditional %373 %374 %375
%374 = OpLabel
%376 = OpLoad %v3bool %x17
%377 = OpFunctionCall %v3bool %returns_bool3_b3
%378 = OpLogicalEqual %v3bool %376 %377
%379 = OpAll %bool %378
OpBranch %375
%375 = OpLabel
%380 = OpPhi %bool %false %368 %379 %374
OpSelectionMerge %382 None
OpBranchConditional %380 %381 %382
%381 = OpLabel
%383 = OpLoad %v4bool %x18
%384 = OpFunctionCall %v4bool %returns_bool4_b4
%385 = OpLogicalEqual %v4bool %383 %384
%386 = OpAll %bool %385
OpBranch %382
%382 = OpLabel
%387 = OpPhi %bool %false %375 %386 %381
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpLoad %int %x19
%391 = OpFunctionCall %int %returns_int_i
%392 = OpIEqual %bool %390 %391
OpBranch %389
%389 = OpLabel
%393 = OpPhi %bool %false %382 %392 %388
OpSelectionMerge %395 None
OpBranchConditional %393 %394 %395
%394 = OpLabel
%396 = OpLoad %v2int %x20
%397 = OpFunctionCall %v2int %returns_int2_i2
%398 = OpIEqual %v2bool %396 %397
%399 = OpAll %bool %398
OpBranch %395
%395 = OpLabel
%400 = OpPhi %bool %false %389 %399 %394
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpLoad %v3int %x21
%404 = OpFunctionCall %v3int %returns_int3_i3
%405 = OpIEqual %v3bool %403 %404
%406 = OpAll %bool %405
OpBranch %402
%402 = OpLabel
%407 = OpPhi %bool %false %395 %406 %401
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%410 = OpLoad %v4int %x22
%411 = OpFunctionCall %v4int %returns_int4_i4
%412 = OpIEqual %v4bool %410 %411
%413 = OpAll %bool %412
OpBranch %409
%409 = OpLabel
%414 = OpPhi %bool %false %402 %413 %408
OpSelectionMerge %418 None
OpBranchConditional %414 %416 %417
%416 = OpLabel
%419 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%422 = OpLoad %v4float %419
OpStore %415 %422
OpBranch %418
%417 = OpLabel
%423 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%424 = OpLoad %v4float %423
OpStore %415 %424
OpBranch %418
%418 = OpLabel
%425 = OpLoad %v4float %415
OpReturnValue %425
OpFunctionEnd
