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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%39 = OpConstantComposite %mat2v2float %37 %38
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%71 = OpConstantComposite %v2float %float_6 %float_0
%72 = OpConstantComposite %v2float %float_0 %float_6
%77 = OpConstantComposite %mat2v2float %71 %72
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%96 = OpConstantComposite %v2float %float_6 %float_12
%97 = OpConstantComposite %v2float %float_18 %float_24
%98 = OpConstantComposite %mat2v2float %96 %97
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%120 = OpConstantComposite %v2float %float_4 %float_0
%121 = OpConstantComposite %v2float %float_0 %float_4
%122 = OpConstantComposite %mat2v2float %120 %121
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%146 = OpConstantComposite %v2float %float_5 %float_2
%147 = OpConstantComposite %v2float %float_3 %float_8
%148 = OpConstantComposite %mat2v2float %146 %147
%float_7 = OpConstant %float 7
%159 = OpConstantComposite %v2float %float_5 %float_6
%160 = OpConstantComposite %v2float %float_7 %float_8
%161 = OpConstantComposite %mat2v2float %159 %160
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%180 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%181 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%182 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%187 = OpConstantComposite %mat3v3float %180 %181 %182
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%206 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%207 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%208 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%209 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%214 = OpConstantComposite %mat4v4float %206 %207 %208 %209
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%234 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%235 = OpConstantComposite %mat4v4float %234 %234 %234 %234
%255 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%256 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%257 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%258 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%259 = OpConstantComposite %mat4v4float %255 %256 %257 %258
%291 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%46 = OpCompositeExtract %v2float %44 0
%47 = OpFOrdEqual %v2bool %46 %37
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %44 1
%50 = OpFOrdEqual %v2bool %49 %38
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %48 %51
OpBranch %43
%43 = OpLabel
%53 = OpPhi %bool %false %26 %52 %42
OpStore %ok %53
%55 = OpLoad %mat2v2float %m1
OpStore %m3 %55
%56 = OpLoad %bool %ok
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %mat2v2float %m3
%60 = OpCompositeExtract %v2float %59 0
%61 = OpFOrdEqual %v2bool %60 %37
%62 = OpAll %bool %61
%63 = OpCompositeExtract %v2float %59 1
%64 = OpFOrdEqual %v2bool %63 %38
%65 = OpAll %bool %64
%66 = OpLogicalAnd %bool %62 %65
OpBranch %58
%58 = OpLabel
%67 = OpPhi %bool %false %43 %66 %57
OpStore %ok %67
%70 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %m4 %70
%73 = OpLoad %bool %ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %mat2v2float %m4
%78 = OpCompositeExtract %v2float %76 0
%79 = OpFOrdEqual %v2bool %78 %71
%80 = OpAll %bool %79
%81 = OpCompositeExtract %v2float %76 1
%82 = OpFOrdEqual %v2bool %81 %72
%83 = OpAll %bool %82
%84 = OpLogicalAnd %bool %80 %83
OpBranch %75
%75 = OpLabel
%85 = OpPhi %bool %false %58 %84 %74
OpStore %ok %85
%86 = OpLoad %mat2v2float %m3
%87 = OpLoad %mat2v2float %m4
%88 = OpMatrixTimesMatrix %mat2v2float %86 %87
OpStore %m3 %88
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %mat2v2float %m3
%99 = OpCompositeExtract %v2float %92 0
%100 = OpFOrdEqual %v2bool %99 %96
%101 = OpAll %bool %100
%102 = OpCompositeExtract %v2float %92 1
%103 = OpFOrdEqual %v2bool %102 %97
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %101 %104
OpBranch %91
%91 = OpLabel
%106 = OpPhi %bool %false %75 %105 %90
OpStore %ok %106
%110 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%111 = OpLoad %v2float %110
%112 = OpCompositeExtract %float %111 1
%114 = OpCompositeConstruct %v2float %112 %float_0
%115 = OpCompositeConstruct %v2float %float_0 %112
%113 = OpCompositeConstruct %mat2v2float %114 %115
OpStore %m5 %113
%116 = OpLoad %bool %ok
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpLoad %mat2v2float %m5
%123 = OpCompositeExtract %v2float %119 0
%124 = OpFOrdEqual %v2bool %123 %120
%125 = OpAll %bool %124
%126 = OpCompositeExtract %v2float %119 1
%127 = OpFOrdEqual %v2bool %126 %121
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %125 %128
OpBranch %118
%118 = OpLabel
%130 = OpPhi %bool %false %91 %129 %117
OpStore %ok %130
%131 = OpLoad %mat2v2float %m1
%132 = OpLoad %mat2v2float %m5
%133 = OpCompositeExtract %v2float %131 0
%134 = OpCompositeExtract %v2float %132 0
%135 = OpFAdd %v2float %133 %134
%136 = OpCompositeExtract %v2float %131 1
%137 = OpCompositeExtract %v2float %132 1
%138 = OpFAdd %v2float %136 %137
%139 = OpCompositeConstruct %mat2v2float %135 %138
OpStore %m1 %139
%140 = OpLoad %bool %ok
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpLoad %mat2v2float %m1
%149 = OpCompositeExtract %v2float %143 0
%150 = OpFOrdEqual %v2bool %149 %146
%151 = OpAll %bool %150
%152 = OpCompositeExtract %v2float %143 1
%153 = OpFOrdEqual %v2bool %152 %147
%154 = OpAll %bool %153
%155 = OpLogicalAnd %bool %151 %154
OpBranch %142
%142 = OpLabel
%156 = OpPhi %bool %false %118 %155 %141
OpStore %ok %156
OpStore %m7 %161
%162 = OpLoad %bool %ok
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpLoad %mat2v2float %m7
%166 = OpCompositeExtract %v2float %165 0
%167 = OpFOrdEqual %v2bool %166 %159
%168 = OpAll %bool %167
%169 = OpCompositeExtract %v2float %165 1
%170 = OpFOrdEqual %v2bool %169 %160
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %168 %171
OpBranch %164
%164 = OpLabel
%173 = OpPhi %bool %false %142 %172 %163
OpStore %ok %173
%179 = OpCompositeConstruct %mat3v3float %180 %181 %182
OpStore %m9 %179
%183 = OpLoad %bool %ok
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %mat3v3float %m9
%189 = OpCompositeExtract %v3float %186 0
%190 = OpFOrdEqual %v3bool %189 %180
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v3float %186 1
%193 = OpFOrdEqual %v3bool %192 %181
%194 = OpAll %bool %193
%195 = OpLogicalAnd %bool %191 %194
%196 = OpCompositeExtract %v3float %186 2
%197 = OpFOrdEqual %v3bool %196 %182
%198 = OpAll %bool %197
%199 = OpLogicalAnd %bool %195 %198
OpBranch %185
%185 = OpLabel
%200 = OpPhi %bool %false %164 %199 %184
OpStore %ok %200
%205 = OpCompositeConstruct %mat4v4float %206 %207 %208 %209
OpStore %m10 %205
%210 = OpLoad %bool %ok
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpLoad %mat4v4float %m10
%216 = OpCompositeExtract %v4float %213 0
%217 = OpFOrdEqual %v4bool %216 %206
%218 = OpAll %bool %217
%219 = OpCompositeExtract %v4float %213 1
%220 = OpFOrdEqual %v4bool %219 %207
%221 = OpAll %bool %220
%222 = OpLogicalAnd %bool %218 %221
%223 = OpCompositeExtract %v4float %213 2
%224 = OpFOrdEqual %v4bool %223 %208
%225 = OpAll %bool %224
%226 = OpLogicalAnd %bool %222 %225
%227 = OpCompositeExtract %v4float %213 3
%228 = OpFOrdEqual %v4bool %227 %209
%229 = OpAll %bool %228
%230 = OpLogicalAnd %bool %226 %229
OpBranch %212
%212 = OpLabel
%231 = OpPhi %bool %false %185 %230 %211
OpStore %ok %231
OpStore %m11 %235
%236 = OpLoad %mat4v4float %m11
%237 = OpLoad %mat4v4float %m10
%238 = OpCompositeExtract %v4float %236 0
%239 = OpCompositeExtract %v4float %237 0
%240 = OpFSub %v4float %238 %239
%241 = OpCompositeExtract %v4float %236 1
%242 = OpCompositeExtract %v4float %237 1
%243 = OpFSub %v4float %241 %242
%244 = OpCompositeExtract %v4float %236 2
%245 = OpCompositeExtract %v4float %237 2
%246 = OpFSub %v4float %244 %245
%247 = OpCompositeExtract %v4float %236 3
%248 = OpCompositeExtract %v4float %237 3
%249 = OpFSub %v4float %247 %248
%250 = OpCompositeConstruct %mat4v4float %240 %243 %246 %249
OpStore %m11 %250
%251 = OpLoad %bool %ok
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
%254 = OpLoad %mat4v4float %m11
%260 = OpCompositeExtract %v4float %254 0
%261 = OpFOrdEqual %v4bool %260 %255
%262 = OpAll %bool %261
%263 = OpCompositeExtract %v4float %254 1
%264 = OpFOrdEqual %v4bool %263 %256
%265 = OpAll %bool %264
%266 = OpLogicalAnd %bool %262 %265
%267 = OpCompositeExtract %v4float %254 2
%268 = OpFOrdEqual %v4bool %267 %257
%269 = OpAll %bool %268
%270 = OpLogicalAnd %bool %266 %269
%271 = OpCompositeExtract %v4float %254 3
%272 = OpFOrdEqual %v4bool %271 %258
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %270 %273
OpBranch %253
%253 = OpLabel
%275 = OpPhi %bool %false %212 %274 %252
OpStore %ok %275
%276 = OpLoad %bool %ok
OpReturnValue %276
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%277 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%280 = OpLoad %mat2v2float %x
%281 = OpLoad %mat2v2float %y
%282 = OpCompositeExtract %v2float %280 0
%283 = OpCompositeExtract %v2float %281 0
%284 = OpFOrdEqual %v2bool %282 %283
%285 = OpAll %bool %284
%286 = OpCompositeExtract %v2float %280 1
%287 = OpCompositeExtract %v2float %281 1
%288 = OpFOrdEqual %v2bool %286 %287
%289 = OpAll %bool %288
%290 = OpLogicalAnd %bool %285 %289
OpReturnValue %290
OpFunctionEnd
%main = OpFunction %v4float None %291
%292 = OpFunctionParameter %_ptr_Function_v2float
%293 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%489 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
%296 = OpLoad %bool %_0_ok
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %mat2v2float %_1_m1
%300 = OpCompositeExtract %v2float %299 0
%301 = OpFOrdEqual %v2bool %300 %37
%302 = OpAll %bool %301
%303 = OpCompositeExtract %v2float %299 1
%304 = OpFOrdEqual %v2bool %303 %38
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %302 %305
OpBranch %298
%298 = OpLabel
%307 = OpPhi %bool %false %293 %306 %297
OpStore %_0_ok %307
%309 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %309
%310 = OpLoad %bool %_0_ok
OpSelectionMerge %312 None
OpBranchConditional %310 %311 %312
%311 = OpLabel
%313 = OpLoad %mat2v2float %_2_m3
%314 = OpCompositeExtract %v2float %313 0
%315 = OpFOrdEqual %v2bool %314 %37
%316 = OpAll %bool %315
%317 = OpCompositeExtract %v2float %313 1
%318 = OpFOrdEqual %v2bool %317 %38
%319 = OpAll %bool %318
%320 = OpLogicalAnd %bool %316 %319
OpBranch %312
%312 = OpLabel
%321 = OpPhi %bool %false %298 %320 %311
OpStore %_0_ok %321
%323 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %_3_m4 %323
%324 = OpLoad %bool %_0_ok
OpSelectionMerge %326 None
OpBranchConditional %324 %325 %326
%325 = OpLabel
%327 = OpLoad %mat2v2float %_3_m4
%328 = OpCompositeExtract %v2float %327 0
%329 = OpFOrdEqual %v2bool %328 %71
%330 = OpAll %bool %329
%331 = OpCompositeExtract %v2float %327 1
%332 = OpFOrdEqual %v2bool %331 %72
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %330 %333
OpBranch %326
%326 = OpLabel
%335 = OpPhi %bool %false %312 %334 %325
OpStore %_0_ok %335
%336 = OpLoad %mat2v2float %_2_m3
%337 = OpLoad %mat2v2float %_3_m4
%338 = OpMatrixTimesMatrix %mat2v2float %336 %337
OpStore %_2_m3 %338
%339 = OpLoad %bool %_0_ok
OpSelectionMerge %341 None
OpBranchConditional %339 %340 %341
%340 = OpLabel
%342 = OpLoad %mat2v2float %_2_m3
%343 = OpCompositeExtract %v2float %342 0
%344 = OpFOrdEqual %v2bool %343 %96
%345 = OpAll %bool %344
%346 = OpCompositeExtract %v2float %342 1
%347 = OpFOrdEqual %v2bool %346 %97
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %345 %348
OpBranch %341
%341 = OpLabel
%350 = OpPhi %bool %false %326 %349 %340
OpStore %_0_ok %350
%352 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%353 = OpLoad %v2float %352
%354 = OpCompositeExtract %float %353 1
%356 = OpCompositeConstruct %v2float %354 %float_0
%357 = OpCompositeConstruct %v2float %float_0 %354
%355 = OpCompositeConstruct %mat2v2float %356 %357
OpStore %_4_m5 %355
%358 = OpLoad %bool %_0_ok
OpSelectionMerge %360 None
OpBranchConditional %358 %359 %360
%359 = OpLabel
%361 = OpLoad %mat2v2float %_4_m5
%362 = OpCompositeExtract %v2float %361 0
%363 = OpFOrdEqual %v2bool %362 %120
%364 = OpAll %bool %363
%365 = OpCompositeExtract %v2float %361 1
%366 = OpFOrdEqual %v2bool %365 %121
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %364 %367
OpBranch %360
%360 = OpLabel
%369 = OpPhi %bool %false %341 %368 %359
OpStore %_0_ok %369
%370 = OpLoad %mat2v2float %_1_m1
%371 = OpLoad %mat2v2float %_4_m5
%372 = OpCompositeExtract %v2float %370 0
%373 = OpCompositeExtract %v2float %371 0
%374 = OpFAdd %v2float %372 %373
%375 = OpCompositeExtract %v2float %370 1
%376 = OpCompositeExtract %v2float %371 1
%377 = OpFAdd %v2float %375 %376
%378 = OpCompositeConstruct %mat2v2float %374 %377
OpStore %_1_m1 %378
%379 = OpLoad %bool %_0_ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %mat2v2float %_1_m1
%383 = OpCompositeExtract %v2float %382 0
%384 = OpFOrdEqual %v2bool %383 %146
%385 = OpAll %bool %384
%386 = OpCompositeExtract %v2float %382 1
%387 = OpFOrdEqual %v2bool %386 %147
%388 = OpAll %bool %387
%389 = OpLogicalAnd %bool %385 %388
OpBranch %381
%381 = OpLabel
%390 = OpPhi %bool %false %360 %389 %380
OpStore %_0_ok %390
OpStore %_5_m7 %161
%392 = OpLoad %bool %_0_ok
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpLoad %mat2v2float %_5_m7
%396 = OpCompositeExtract %v2float %395 0
%397 = OpFOrdEqual %v2bool %396 %159
%398 = OpAll %bool %397
%399 = OpCompositeExtract %v2float %395 1
%400 = OpFOrdEqual %v2bool %399 %160
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %398 %401
OpBranch %394
%394 = OpLabel
%403 = OpPhi %bool %false %381 %402 %393
OpStore %_0_ok %403
%405 = OpCompositeConstruct %mat3v3float %180 %181 %182
OpStore %_6_m9 %405
%406 = OpLoad %bool %_0_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpLoad %mat3v3float %_6_m9
%410 = OpCompositeExtract %v3float %409 0
%411 = OpFOrdEqual %v3bool %410 %180
%412 = OpAll %bool %411
%413 = OpCompositeExtract %v3float %409 1
%414 = OpFOrdEqual %v3bool %413 %181
%415 = OpAll %bool %414
%416 = OpLogicalAnd %bool %412 %415
%417 = OpCompositeExtract %v3float %409 2
%418 = OpFOrdEqual %v3bool %417 %182
%419 = OpAll %bool %418
%420 = OpLogicalAnd %bool %416 %419
OpBranch %408
%408 = OpLabel
%421 = OpPhi %bool %false %394 %420 %407
OpStore %_0_ok %421
%423 = OpCompositeConstruct %mat4v4float %206 %207 %208 %209
OpStore %_7_m10 %423
%424 = OpLoad %bool %_0_ok
OpSelectionMerge %426 None
OpBranchConditional %424 %425 %426
%425 = OpLabel
%427 = OpLoad %mat4v4float %_7_m10
%428 = OpCompositeExtract %v4float %427 0
%429 = OpFOrdEqual %v4bool %428 %206
%430 = OpAll %bool %429
%431 = OpCompositeExtract %v4float %427 1
%432 = OpFOrdEqual %v4bool %431 %207
%433 = OpAll %bool %432
%434 = OpLogicalAnd %bool %430 %433
%435 = OpCompositeExtract %v4float %427 2
%436 = OpFOrdEqual %v4bool %435 %208
%437 = OpAll %bool %436
%438 = OpLogicalAnd %bool %434 %437
%439 = OpCompositeExtract %v4float %427 3
%440 = OpFOrdEqual %v4bool %439 %209
%441 = OpAll %bool %440
%442 = OpLogicalAnd %bool %438 %441
OpBranch %426
%426 = OpLabel
%443 = OpPhi %bool %false %408 %442 %425
OpStore %_0_ok %443
OpStore %_8_m11 %235
%445 = OpLoad %mat4v4float %_8_m11
%446 = OpLoad %mat4v4float %_7_m10
%447 = OpCompositeExtract %v4float %445 0
%448 = OpCompositeExtract %v4float %446 0
%449 = OpFSub %v4float %447 %448
%450 = OpCompositeExtract %v4float %445 1
%451 = OpCompositeExtract %v4float %446 1
%452 = OpFSub %v4float %450 %451
%453 = OpCompositeExtract %v4float %445 2
%454 = OpCompositeExtract %v4float %446 2
%455 = OpFSub %v4float %453 %454
%456 = OpCompositeExtract %v4float %445 3
%457 = OpCompositeExtract %v4float %446 3
%458 = OpFSub %v4float %456 %457
%459 = OpCompositeConstruct %mat4v4float %449 %452 %455 %458
OpStore %_8_m11 %459
%460 = OpLoad %bool %_0_ok
OpSelectionMerge %462 None
OpBranchConditional %460 %461 %462
%461 = OpLabel
%463 = OpLoad %mat4v4float %_8_m11
%464 = OpCompositeExtract %v4float %463 0
%465 = OpFOrdEqual %v4bool %464 %255
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v4float %463 1
%468 = OpFOrdEqual %v4bool %467 %256
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %466 %469
%471 = OpCompositeExtract %v4float %463 2
%472 = OpFOrdEqual %v4bool %471 %257
%473 = OpAll %bool %472
%474 = OpLogicalAnd %bool %470 %473
%475 = OpCompositeExtract %v4float %463 3
%476 = OpFOrdEqual %v4bool %475 %258
%477 = OpAll %bool %476
%478 = OpLogicalAnd %bool %474 %477
OpBranch %462
%462 = OpLabel
%479 = OpPhi %bool %false %426 %478 %461
OpStore %_0_ok %479
%480 = OpLoad %bool %_0_ok
OpSelectionMerge %482 None
OpBranchConditional %480 %481 %482
%481 = OpLabel
%483 = OpFunctionCall %bool %test_half_b
OpBranch %482
%482 = OpLabel
%484 = OpPhi %bool %false %462 %483 %481
OpSelectionMerge %486 None
OpBranchConditional %484 %485 %486
%485 = OpLabel
%487 = OpFunctionCall %bool %test_comma_b
OpBranch %486
%486 = OpLabel
%488 = OpPhi %bool %false %482 %487 %485
OpSelectionMerge %493 None
OpBranchConditional %488 %491 %492
%491 = OpLabel
%494 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%497 = OpLoad %v4float %494
OpStore %489 %497
OpBranch %493
%492 = OpLabel
%498 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%499 = OpLoad %v4float %498
OpStore %489 %499
OpBranch %493
%493 = OpLabel
%500 = OpLoad %v4float %489
OpReturnValue %500
OpFunctionEnd
