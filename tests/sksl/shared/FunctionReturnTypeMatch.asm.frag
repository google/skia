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
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
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
OpDecorate %165 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%45 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%49 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%mat2v2float = OpTypeMatrix %v2float 2
%54 = OpTypeFunction %mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%61 = OpTypeFunction %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%68 = OpTypeFunction %mat4v4float
%75 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%99 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%103 = OpTypeFunction %v2bool
%v3bool = OpTypeVector %bool 3
%107 = OpTypeFunction %v3bool
%v4bool = OpTypeVector %bool 4
%111 = OpTypeFunction %v4bool
%int = OpTypeInt 32 1
%115 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%119 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%v3int = OpTypeVector %int 3
%124 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%v4int = OpTypeVector %int 4
%129 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
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
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%38 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %40
%41 = OpLabel
%43 = OpCompositeConstruct %v2float %float_2 %float_2
OpReturnValue %43
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %45
%46 = OpLabel
%48 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpReturnValue %48
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %49
%50 = OpLabel
%52 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpReturnValue %52
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %54
%55 = OpLabel
%58 = OpCompositeConstruct %v2float %float_2 %float_0
%59 = OpCompositeConstruct %v2float %float_0 %float_2
%56 = OpCompositeConstruct %mat2v2float %58 %59
OpReturnValue %56
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %61
%62 = OpLabel
%64 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%65 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%66 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%63 = OpCompositeConstruct %mat3v3float %64 %65 %66
OpReturnValue %63
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %68
%69 = OpLabel
%71 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%74 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%70 = OpCompositeConstruct %mat4v4float %71 %72 %73 %74
OpReturnValue %70
OpFunctionEnd
%returns_half_h = OpFunction %float None %75
%76 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %40
%78 = OpLabel
%79 = OpCompositeConstruct %v2float %float_2 %float_2
OpReturnValue %79
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %45
%80 = OpLabel
%81 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpReturnValue %81
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %49
%82 = OpLabel
%83 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpReturnValue %83
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %54
%84 = OpLabel
%86 = OpCompositeConstruct %v2float %float_2 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_2
%85 = OpCompositeConstruct %mat2v2float %86 %87
OpReturnValue %85
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %61
%88 = OpLabel
%90 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%91 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%92 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%89 = OpCompositeConstruct %mat3v3float %90 %91 %92
OpReturnValue %89
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %68
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
%105 = OpCompositeConstruct %v2bool %true %true
OpReturnValue %105
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %107
%108 = OpLabel
%109 = OpCompositeConstruct %v3bool %true %true %true
OpReturnValue %109
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %111
%112 = OpLabel
%113 = OpCompositeConstruct %v4bool %true %true %true %true
OpReturnValue %113
OpFunctionEnd
%returns_int_i = OpFunction %int None %115
%116 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %119
%120 = OpLabel
%122 = OpCompositeConstruct %v2int %int_2 %int_2
OpReturnValue %122
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %124
%125 = OpLabel
%127 = OpCompositeConstruct %v3int %int_3 %int_3 %int_3
OpReturnValue %127
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %129
%130 = OpLabel
%132 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
OpReturnValue %132
OpFunctionEnd
%main = OpFunction %v4float None %49
%133 = OpLabel
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
%426 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
%138 = OpCompositeConstruct %v2float %float_2 %float_2
OpStore %x2 %138
%141 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpStore %x3 %141
%144 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpStore %x4 %144
%148 = OpCompositeConstruct %v2float %float_2 %float_0
%149 = OpCompositeConstruct %v2float %float_0 %float_2
%147 = OpCompositeConstruct %mat2v2float %148 %149
OpStore %x5 %147
%153 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
OpStore %x6 %152
%159 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%160 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%161 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%162 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%158 = OpCompositeConstruct %mat4v4float %159 %160 %161 %162
OpStore %x7 %158
OpStore %x8 %float_1
%165 = OpCompositeConstruct %v2float %float_2 %float_2
OpStore %x9 %165
%167 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpStore %x10 %167
%169 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpStore %x11 %169
%172 = OpCompositeConstruct %v2float %float_2 %float_0
%173 = OpCompositeConstruct %v2float %float_0 %float_2
%171 = OpCompositeConstruct %mat2v2float %172 %173
OpStore %x12 %171
%176 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%177 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%178 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%175 = OpCompositeConstruct %mat3v3float %176 %177 %178
OpStore %x13 %175
%181 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%182 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%183 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%184 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%180 = OpCompositeConstruct %mat4v4float %181 %182 %183 %184
OpStore %x14 %180
OpStore %x15 %true
%189 = OpCompositeConstruct %v2bool %true %true
OpStore %x16 %189
%192 = OpCompositeConstruct %v3bool %true %true %true
OpStore %x17 %192
%195 = OpCompositeConstruct %v4bool %true %true %true %true
OpStore %x18 %195
OpStore %x19 %int_1
%200 = OpCompositeConstruct %v2int %int_2 %int_2
OpStore %x20 %200
%203 = OpCompositeConstruct %v3int %int_3 %int_3 %int_3
OpStore %x21 %203
%206 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
OpStore %x22 %206
%208 = OpLoad %float %x1
%209 = OpFOrdEqual %bool %208 %float_1
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpLoad %v2float %x2
%213 = OpFunctionCall %v2float %returns_float2_f2
%214 = OpFOrdEqual %v2bool %212 %213
%215 = OpAll %bool %214
OpBranch %211
%211 = OpLabel
%216 = OpPhi %bool %false %133 %215 %210
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %v3float %x3
%220 = OpFunctionCall %v3float %returns_float3_f3
%221 = OpFOrdEqual %v3bool %219 %220
%222 = OpAll %bool %221
OpBranch %218
%218 = OpLabel
%223 = OpPhi %bool %false %211 %222 %217
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %v4float %x4
%227 = OpFunctionCall %v4float %returns_float4_f4
%228 = OpFOrdEqual %v4bool %226 %227
%229 = OpAll %bool %228
OpBranch %225
%225 = OpLabel
%230 = OpPhi %bool %false %218 %229 %224
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat2v2float %x5
%234 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%235 = OpCompositeExtract %v2float %233 0
%236 = OpCompositeExtract %v2float %234 0
%237 = OpFOrdEqual %v2bool %235 %236
%238 = OpAll %bool %237
%239 = OpCompositeExtract %v2float %233 1
%240 = OpCompositeExtract %v2float %234 1
%241 = OpFOrdEqual %v2bool %239 %240
%242 = OpAll %bool %241
%243 = OpLogicalAnd %bool %238 %242
OpBranch %232
%232 = OpLabel
%244 = OpPhi %bool %false %225 %243 %231
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%247 = OpLoad %mat3v3float %x6
%248 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%249 = OpCompositeExtract %v3float %247 0
%250 = OpCompositeExtract %v3float %248 0
%251 = OpFOrdEqual %v3bool %249 %250
%252 = OpAll %bool %251
%253 = OpCompositeExtract %v3float %247 1
%254 = OpCompositeExtract %v3float %248 1
%255 = OpFOrdEqual %v3bool %253 %254
%256 = OpAll %bool %255
%257 = OpLogicalAnd %bool %252 %256
%258 = OpCompositeExtract %v3float %247 2
%259 = OpCompositeExtract %v3float %248 2
%260 = OpFOrdEqual %v3bool %258 %259
%261 = OpAll %bool %260
%262 = OpLogicalAnd %bool %257 %261
OpBranch %246
%246 = OpLabel
%263 = OpPhi %bool %false %232 %262 %245
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpLoad %mat4v4float %x7
%267 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%268 = OpCompositeExtract %v4float %266 0
%269 = OpCompositeExtract %v4float %267 0
%270 = OpFOrdEqual %v4bool %268 %269
%271 = OpAll %bool %270
%272 = OpCompositeExtract %v4float %266 1
%273 = OpCompositeExtract %v4float %267 1
%274 = OpFOrdEqual %v4bool %272 %273
%275 = OpAll %bool %274
%276 = OpLogicalAnd %bool %271 %275
%277 = OpCompositeExtract %v4float %266 2
%278 = OpCompositeExtract %v4float %267 2
%279 = OpFOrdEqual %v4bool %277 %278
%280 = OpAll %bool %279
%281 = OpLogicalAnd %bool %276 %280
%282 = OpCompositeExtract %v4float %266 3
%283 = OpCompositeExtract %v4float %267 3
%284 = OpFOrdEqual %v4bool %282 %283
%285 = OpAll %bool %284
%286 = OpLogicalAnd %bool %281 %285
OpBranch %265
%265 = OpLabel
%287 = OpPhi %bool %false %246 %286 %264
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpLoad %float %x8
%291 = OpFunctionCall %float %returns_half_h
%292 = OpFOrdEqual %bool %290 %291
OpBranch %289
%289 = OpLabel
%293 = OpPhi %bool %false %265 %292 %288
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpLoad %v2float %x9
%297 = OpFunctionCall %v2float %returns_half2_h2
%298 = OpFOrdEqual %v2bool %296 %297
%299 = OpAll %bool %298
OpBranch %295
%295 = OpLabel
%300 = OpPhi %bool %false %289 %299 %294
OpSelectionMerge %302 None
OpBranchConditional %300 %301 %302
%301 = OpLabel
%303 = OpLoad %v3float %x10
%304 = OpFunctionCall %v3float %returns_half3_h3
%305 = OpFOrdEqual %v3bool %303 %304
%306 = OpAll %bool %305
OpBranch %302
%302 = OpLabel
%307 = OpPhi %bool %false %295 %306 %301
OpSelectionMerge %309 None
OpBranchConditional %307 %308 %309
%308 = OpLabel
%310 = OpLoad %v4float %x11
%311 = OpFunctionCall %v4float %returns_half4_h4
%312 = OpFOrdEqual %v4bool %310 %311
%313 = OpAll %bool %312
OpBranch %309
%309 = OpLabel
%314 = OpPhi %bool %false %302 %313 %308
OpSelectionMerge %316 None
OpBranchConditional %314 %315 %316
%315 = OpLabel
%317 = OpLoad %mat2v2float %x12
%318 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%319 = OpCompositeExtract %v2float %317 0
%320 = OpCompositeExtract %v2float %318 0
%321 = OpFOrdEqual %v2bool %319 %320
%322 = OpAll %bool %321
%323 = OpCompositeExtract %v2float %317 1
%324 = OpCompositeExtract %v2float %318 1
%325 = OpFOrdEqual %v2bool %323 %324
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %322 %326
OpBranch %316
%316 = OpLabel
%328 = OpPhi %bool %false %309 %327 %315
OpSelectionMerge %330 None
OpBranchConditional %328 %329 %330
%329 = OpLabel
%331 = OpLoad %mat3v3float %x13
%332 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%333 = OpCompositeExtract %v3float %331 0
%334 = OpCompositeExtract %v3float %332 0
%335 = OpFOrdEqual %v3bool %333 %334
%336 = OpAll %bool %335
%337 = OpCompositeExtract %v3float %331 1
%338 = OpCompositeExtract %v3float %332 1
%339 = OpFOrdEqual %v3bool %337 %338
%340 = OpAll %bool %339
%341 = OpLogicalAnd %bool %336 %340
%342 = OpCompositeExtract %v3float %331 2
%343 = OpCompositeExtract %v3float %332 2
%344 = OpFOrdEqual %v3bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
OpBranch %330
%330 = OpLabel
%347 = OpPhi %bool %false %316 %346 %329
OpSelectionMerge %349 None
OpBranchConditional %347 %348 %349
%348 = OpLabel
%350 = OpLoad %mat4v4float %x14
%351 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%352 = OpCompositeExtract %v4float %350 0
%353 = OpCompositeExtract %v4float %351 0
%354 = OpFOrdEqual %v4bool %352 %353
%355 = OpAll %bool %354
%356 = OpCompositeExtract %v4float %350 1
%357 = OpCompositeExtract %v4float %351 1
%358 = OpFOrdEqual %v4bool %356 %357
%359 = OpAll %bool %358
%360 = OpLogicalAnd %bool %355 %359
%361 = OpCompositeExtract %v4float %350 2
%362 = OpCompositeExtract %v4float %351 2
%363 = OpFOrdEqual %v4bool %361 %362
%364 = OpAll %bool %363
%365 = OpLogicalAnd %bool %360 %364
%366 = OpCompositeExtract %v4float %350 3
%367 = OpCompositeExtract %v4float %351 3
%368 = OpFOrdEqual %v4bool %366 %367
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %365 %369
OpBranch %349
%349 = OpLabel
%371 = OpPhi %bool %false %330 %370 %348
OpSelectionMerge %373 None
OpBranchConditional %371 %372 %373
%372 = OpLabel
%374 = OpLoad %bool %x15
%375 = OpFunctionCall %bool %returns_bool_b
%376 = OpLogicalEqual %bool %374 %375
OpBranch %373
%373 = OpLabel
%377 = OpPhi %bool %false %349 %376 %372
OpSelectionMerge %379 None
OpBranchConditional %377 %378 %379
%378 = OpLabel
%380 = OpLoad %v2bool %x16
%381 = OpFunctionCall %v2bool %returns_bool2_b2
%382 = OpLogicalEqual %v2bool %380 %381
%383 = OpAll %bool %382
OpBranch %379
%379 = OpLabel
%384 = OpPhi %bool %false %373 %383 %378
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpLoad %v3bool %x17
%388 = OpFunctionCall %v3bool %returns_bool3_b3
%389 = OpLogicalEqual %v3bool %387 %388
%390 = OpAll %bool %389
OpBranch %386
%386 = OpLabel
%391 = OpPhi %bool %false %379 %390 %385
OpSelectionMerge %393 None
OpBranchConditional %391 %392 %393
%392 = OpLabel
%394 = OpLoad %v4bool %x18
%395 = OpFunctionCall %v4bool %returns_bool4_b4
%396 = OpLogicalEqual %v4bool %394 %395
%397 = OpAll %bool %396
OpBranch %393
%393 = OpLabel
%398 = OpPhi %bool %false %386 %397 %392
OpSelectionMerge %400 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%401 = OpLoad %int %x19
%402 = OpFunctionCall %int %returns_int_i
%403 = OpIEqual %bool %401 %402
OpBranch %400
%400 = OpLabel
%404 = OpPhi %bool %false %393 %403 %399
OpSelectionMerge %406 None
OpBranchConditional %404 %405 %406
%405 = OpLabel
%407 = OpLoad %v2int %x20
%408 = OpFunctionCall %v2int %returns_int2_i2
%409 = OpIEqual %v2bool %407 %408
%410 = OpAll %bool %409
OpBranch %406
%406 = OpLabel
%411 = OpPhi %bool %false %400 %410 %405
OpSelectionMerge %413 None
OpBranchConditional %411 %412 %413
%412 = OpLabel
%414 = OpLoad %v3int %x21
%415 = OpFunctionCall %v3int %returns_int3_i3
%416 = OpIEqual %v3bool %414 %415
%417 = OpAll %bool %416
OpBranch %413
%413 = OpLabel
%418 = OpPhi %bool %false %406 %417 %412
OpSelectionMerge %420 None
OpBranchConditional %418 %419 %420
%419 = OpLabel
%421 = OpLoad %v4int %x22
%422 = OpFunctionCall %v4int %returns_int4_i4
%423 = OpIEqual %v4bool %421 %422
%424 = OpAll %bool %423
OpBranch %420
%420 = OpLabel
%425 = OpPhi %bool %false %413 %424 %419
OpSelectionMerge %429 None
OpBranchConditional %425 %427 %428
%427 = OpLabel
%430 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%433 = OpLoad %v4float %430
OpStore %426 %433
OpBranch %429
%428 = OpLabel
%434 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%435 = OpLoad %v4float %434
OpStore %426 %435
OpBranch %429
%429 = OpLabel
%436 = OpLoad %v4float %426
OpReturnValue %436
OpFunctionEnd
