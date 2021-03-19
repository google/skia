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
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
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
%411 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %43
OpStore %x3 %48
OpStore %x4 %52
%142 = OpCompositeConstruct %v2float %float_2 %float_0
%143 = OpCompositeConstruct %v2float %float_0 %float_2
%141 = OpCompositeConstruct %mat2v2float %142 %143
OpStore %x5 %141
%147 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%148 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%149 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%146 = OpCompositeConstruct %mat3v3float %147 %148 %149
OpStore %x6 %146
%153 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%154 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%155 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%156 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%152 = OpCompositeConstruct %mat4v4float %153 %154 %155 %156
OpStore %x7 %152
OpStore %x8 %float_1
OpStore %x9 %43
OpStore %x10 %48
OpStore %x11 %52
%163 = OpCompositeConstruct %v2float %float_2 %float_0
%164 = OpCompositeConstruct %v2float %float_0 %float_2
%162 = OpCompositeConstruct %mat2v2float %163 %164
OpStore %x12 %162
%167 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%168 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%169 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%166 = OpCompositeConstruct %mat3v3float %167 %168 %169
OpStore %x13 %166
%172 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%173 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%174 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%175 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%171 = OpCompositeConstruct %mat4v4float %172 %173 %174 %175
OpStore %x14 %171
OpStore %x15 %true
OpStore %x16 %102
OpStore %x17 %106
OpStore %x18 %110
OpStore %x19 %int_1
OpStore %x20 %119
OpStore %x21 %124
OpStore %x22 %129
%193 = OpLoad %float %x1
%194 = OpFOrdEqual %bool %193 %float_1
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %v2float %x2
%198 = OpFunctionCall %v2float %returns_float2
%199 = OpFOrdEqual %v2bool %197 %198
%200 = OpAll %bool %199
OpBranch %196
%196 = OpLabel
%201 = OpPhi %bool %false %130 %200 %195
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%204 = OpLoad %v3float %x3
%205 = OpFunctionCall %v3float %returns_float3
%206 = OpFOrdEqual %v3bool %204 %205
%207 = OpAll %bool %206
OpBranch %203
%203 = OpLabel
%208 = OpPhi %bool %false %196 %207 %202
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpLoad %v4float %x4
%212 = OpFunctionCall %v4float %returns_float4
%213 = OpFOrdEqual %v4bool %211 %212
%214 = OpAll %bool %213
OpBranch %210
%210 = OpLabel
%215 = OpPhi %bool %false %203 %214 %209
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpLoad %mat2v2float %x5
%219 = OpFunctionCall %mat2v2float %returns_float2x2
%220 = OpCompositeExtract %v2float %218 0
%221 = OpCompositeExtract %v2float %219 0
%222 = OpFOrdEqual %v2bool %220 %221
%223 = OpAll %bool %222
%224 = OpCompositeExtract %v2float %218 1
%225 = OpCompositeExtract %v2float %219 1
%226 = OpFOrdEqual %v2bool %224 %225
%227 = OpAll %bool %226
%228 = OpLogicalAnd %bool %223 %227
OpBranch %217
%217 = OpLabel
%229 = OpPhi %bool %false %210 %228 %216
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpLoad %mat3v3float %x6
%233 = OpFunctionCall %mat3v3float %returns_float3x3
%234 = OpCompositeExtract %v3float %232 0
%235 = OpCompositeExtract %v3float %233 0
%236 = OpFOrdEqual %v3bool %234 %235
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v3float %232 1
%239 = OpCompositeExtract %v3float %233 1
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
%243 = OpCompositeExtract %v3float %232 2
%244 = OpCompositeExtract %v3float %233 2
%245 = OpFOrdEqual %v3bool %243 %244
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %242 %246
OpBranch %231
%231 = OpLabel
%248 = OpPhi %bool %false %217 %247 %230
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
%251 = OpLoad %mat4v4float %x7
%252 = OpFunctionCall %mat4v4float %returns_float4x4
%253 = OpCompositeExtract %v4float %251 0
%254 = OpCompositeExtract %v4float %252 0
%255 = OpFOrdEqual %v4bool %253 %254
%256 = OpAll %bool %255
%257 = OpCompositeExtract %v4float %251 1
%258 = OpCompositeExtract %v4float %252 1
%259 = OpFOrdEqual %v4bool %257 %258
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %256 %260
%262 = OpCompositeExtract %v4float %251 2
%263 = OpCompositeExtract %v4float %252 2
%264 = OpFOrdEqual %v4bool %262 %263
%265 = OpAll %bool %264
%266 = OpLogicalAnd %bool %261 %265
%267 = OpCompositeExtract %v4float %251 3
%268 = OpCompositeExtract %v4float %252 3
%269 = OpFOrdEqual %v4bool %267 %268
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %266 %270
OpBranch %250
%250 = OpLabel
%272 = OpPhi %bool %false %231 %271 %249
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%275 = OpLoad %float %x8
%276 = OpFunctionCall %float %returns_half
%277 = OpFOrdEqual %bool %275 %276
OpBranch %274
%274 = OpLabel
%278 = OpPhi %bool %false %250 %277 %273
OpSelectionMerge %280 None
OpBranchConditional %278 %279 %280
%279 = OpLabel
%281 = OpLoad %v2float %x9
%282 = OpFunctionCall %v2float %returns_half2
%283 = OpFOrdEqual %v2bool %281 %282
%284 = OpAll %bool %283
OpBranch %280
%280 = OpLabel
%285 = OpPhi %bool %false %274 %284 %279
OpSelectionMerge %287 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%288 = OpLoad %v3float %x10
%289 = OpFunctionCall %v3float %returns_half3
%290 = OpFOrdEqual %v3bool %288 %289
%291 = OpAll %bool %290
OpBranch %287
%287 = OpLabel
%292 = OpPhi %bool %false %280 %291 %286
OpSelectionMerge %294 None
OpBranchConditional %292 %293 %294
%293 = OpLabel
%295 = OpLoad %v4float %x11
%296 = OpFunctionCall %v4float %returns_half4
%297 = OpFOrdEqual %v4bool %295 %296
%298 = OpAll %bool %297
OpBranch %294
%294 = OpLabel
%299 = OpPhi %bool %false %287 %298 %293
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpLoad %mat2v2float %x12
%303 = OpFunctionCall %mat2v2float %returns_half2x2
%304 = OpCompositeExtract %v2float %302 0
%305 = OpCompositeExtract %v2float %303 0
%306 = OpFOrdEqual %v2bool %304 %305
%307 = OpAll %bool %306
%308 = OpCompositeExtract %v2float %302 1
%309 = OpCompositeExtract %v2float %303 1
%310 = OpFOrdEqual %v2bool %308 %309
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %307 %311
OpBranch %301
%301 = OpLabel
%313 = OpPhi %bool %false %294 %312 %300
OpSelectionMerge %315 None
OpBranchConditional %313 %314 %315
%314 = OpLabel
%316 = OpLoad %mat3v3float %x13
%317 = OpFunctionCall %mat3v3float %returns_half3x3
%318 = OpCompositeExtract %v3float %316 0
%319 = OpCompositeExtract %v3float %317 0
%320 = OpFOrdEqual %v3bool %318 %319
%321 = OpAll %bool %320
%322 = OpCompositeExtract %v3float %316 1
%323 = OpCompositeExtract %v3float %317 1
%324 = OpFOrdEqual %v3bool %322 %323
%325 = OpAll %bool %324
%326 = OpLogicalAnd %bool %321 %325
%327 = OpCompositeExtract %v3float %316 2
%328 = OpCompositeExtract %v3float %317 2
%329 = OpFOrdEqual %v3bool %327 %328
%330 = OpAll %bool %329
%331 = OpLogicalAnd %bool %326 %330
OpBranch %315
%315 = OpLabel
%332 = OpPhi %bool %false %301 %331 %314
OpSelectionMerge %334 None
OpBranchConditional %332 %333 %334
%333 = OpLabel
%335 = OpLoad %mat4v4float %x14
%336 = OpFunctionCall %mat4v4float %returns_half4x4
%337 = OpCompositeExtract %v4float %335 0
%338 = OpCompositeExtract %v4float %336 0
%339 = OpFOrdEqual %v4bool %337 %338
%340 = OpAll %bool %339
%341 = OpCompositeExtract %v4float %335 1
%342 = OpCompositeExtract %v4float %336 1
%343 = OpFOrdEqual %v4bool %341 %342
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %340 %344
%346 = OpCompositeExtract %v4float %335 2
%347 = OpCompositeExtract %v4float %336 2
%348 = OpFOrdEqual %v4bool %346 %347
%349 = OpAll %bool %348
%350 = OpLogicalAnd %bool %345 %349
%351 = OpCompositeExtract %v4float %335 3
%352 = OpCompositeExtract %v4float %336 3
%353 = OpFOrdEqual %v4bool %351 %352
%354 = OpAll %bool %353
%355 = OpLogicalAnd %bool %350 %354
OpBranch %334
%334 = OpLabel
%356 = OpPhi %bool %false %315 %355 %333
OpSelectionMerge %358 None
OpBranchConditional %356 %357 %358
%357 = OpLabel
%359 = OpLoad %bool %x15
%360 = OpFunctionCall %bool %returns_bool
%361 = OpLogicalEqual %bool %359 %360
OpBranch %358
%358 = OpLabel
%362 = OpPhi %bool %false %334 %361 %357
OpSelectionMerge %364 None
OpBranchConditional %362 %363 %364
%363 = OpLabel
%365 = OpLoad %v2bool %x16
%366 = OpFunctionCall %v2bool %returns_bool2
%367 = OpLogicalEqual %v2bool %365 %366
%368 = OpAll %bool %367
OpBranch %364
%364 = OpLabel
%369 = OpPhi %bool %false %358 %368 %363
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%372 = OpLoad %v3bool %x17
%373 = OpFunctionCall %v3bool %returns_bool3
%374 = OpLogicalEqual %v3bool %372 %373
%375 = OpAll %bool %374
OpBranch %371
%371 = OpLabel
%376 = OpPhi %bool %false %364 %375 %370
OpSelectionMerge %378 None
OpBranchConditional %376 %377 %378
%377 = OpLabel
%379 = OpLoad %v4bool %x18
%380 = OpFunctionCall %v4bool %returns_bool4
%381 = OpLogicalEqual %v4bool %379 %380
%382 = OpAll %bool %381
OpBranch %378
%378 = OpLabel
%383 = OpPhi %bool %false %371 %382 %377
OpSelectionMerge %385 None
OpBranchConditional %383 %384 %385
%384 = OpLabel
%386 = OpLoad %int %x19
%387 = OpFunctionCall %int %returns_int
%388 = OpIEqual %bool %386 %387
OpBranch %385
%385 = OpLabel
%389 = OpPhi %bool %false %378 %388 %384
OpSelectionMerge %391 None
OpBranchConditional %389 %390 %391
%390 = OpLabel
%392 = OpLoad %v2int %x20
%393 = OpFunctionCall %v2int %returns_int2
%394 = OpIEqual %v2bool %392 %393
%395 = OpAll %bool %394
OpBranch %391
%391 = OpLabel
%396 = OpPhi %bool %false %385 %395 %390
OpSelectionMerge %398 None
OpBranchConditional %396 %397 %398
%397 = OpLabel
%399 = OpLoad %v3int %x21
%400 = OpFunctionCall %v3int %returns_int3
%401 = OpIEqual %v3bool %399 %400
%402 = OpAll %bool %401
OpBranch %398
%398 = OpLabel
%403 = OpPhi %bool %false %391 %402 %397
OpSelectionMerge %405 None
OpBranchConditional %403 %404 %405
%404 = OpLabel
%406 = OpLoad %v4int %x22
%407 = OpFunctionCall %v4int %returns_int4
%408 = OpIEqual %v4bool %406 %407
%409 = OpAll %bool %408
OpBranch %405
%405 = OpLabel
%410 = OpPhi %bool %false %398 %409 %404
OpSelectionMerge %414 None
OpBranchConditional %410 %412 %413
%412 = OpLabel
%415 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%418 = OpLoad %v4float %415
OpStore %411 %418
OpBranch %414
%413 = OpLabel
%419 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%420 = OpLoad %v4float %419
OpStore %411 %420
OpBranch %414
%414 = OpLabel
%421 = OpLoad %v4float %411
OpReturnValue %421
OpFunctionEnd
