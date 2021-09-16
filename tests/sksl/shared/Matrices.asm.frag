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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
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
OpName %h4 "h4"
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
OpName %_9_f4 "_9_f4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %713 RelaxedPrecision
OpDecorate %750 RelaxedPrecision
OpDecorate %772 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %887 RelaxedPrecision
OpDecorate %888 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%float_7 = OpConstant %float 7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%474 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %26
%27 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%h4 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%33 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%37 = OpLoad %mat2v2float %33
OpStore %m1 %37
%39 = OpLoad %bool %ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %mat2v2float %m1
%47 = OpCompositeConstruct %v2float %float_1 %float_2
%48 = OpCompositeConstruct %v2float %float_3 %float_4
%49 = OpCompositeConstruct %mat2v2float %47 %48
%51 = OpCompositeExtract %v2float %42 0
%52 = OpCompositeExtract %v2float %49 0
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpCompositeExtract %v2float %42 1
%56 = OpCompositeExtract %v2float %49 1
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpLogicalAnd %bool %54 %58
OpBranch %41
%41 = OpLabel
%60 = OpPhi %bool %false %27 %59 %40
OpStore %ok %60
%62 = OpLoad %mat2v2float %m1
OpStore %m3 %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v2float %m3
%67 = OpCompositeConstruct %v2float %float_1 %float_2
%68 = OpCompositeConstruct %v2float %float_3 %float_4
%69 = OpCompositeConstruct %mat2v2float %67 %68
%70 = OpCompositeExtract %v2float %66 0
%71 = OpCompositeExtract %v2float %69 0
%72 = OpFOrdEqual %v2bool %70 %71
%73 = OpAll %bool %72
%74 = OpCompositeExtract %v2float %66 1
%75 = OpCompositeExtract %v2float %69 1
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %73 %77
OpBranch %65
%65 = OpLabel
%79 = OpPhi %bool %false %41 %78 %64
OpStore %ok %79
%83 = OpCompositeConstruct %v2float %float_6 %float_0
%84 = OpCompositeConstruct %v2float %float_0 %float_6
%82 = OpCompositeConstruct %mat2v2float %83 %84
OpStore %m4 %82
%85 = OpLoad %bool %ok
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %mat2v2float %m4
%89 = OpCompositeConstruct %v2float %float_6 %float_0
%90 = OpCompositeConstruct %v2float %float_0 %float_6
%91 = OpCompositeConstruct %mat2v2float %89 %90
%92 = OpCompositeExtract %v2float %88 0
%93 = OpCompositeExtract %v2float %91 0
%94 = OpFOrdEqual %v2bool %92 %93
%95 = OpAll %bool %94
%96 = OpCompositeExtract %v2float %88 1
%97 = OpCompositeExtract %v2float %91 1
%98 = OpFOrdEqual %v2bool %96 %97
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %95 %99
OpBranch %87
%87 = OpLabel
%101 = OpPhi %bool %false %65 %100 %86
OpStore %ok %101
%102 = OpLoad %mat2v2float %m3
%103 = OpLoad %mat2v2float %m4
%104 = OpMatrixTimesMatrix %mat2v2float %102 %103
OpStore %m3 %104
%105 = OpLoad %bool %ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %mat2v2float %m3
%112 = OpCompositeConstruct %v2float %float_6 %float_12
%113 = OpCompositeConstruct %v2float %float_18 %float_24
%114 = OpCompositeConstruct %mat2v2float %112 %113
%115 = OpCompositeExtract %v2float %108 0
%116 = OpCompositeExtract %v2float %114 0
%117 = OpFOrdEqual %v2bool %115 %116
%118 = OpAll %bool %117
%119 = OpCompositeExtract %v2float %108 1
%120 = OpCompositeExtract %v2float %114 1
%121 = OpFOrdEqual %v2bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
OpBranch %107
%107 = OpLabel
%124 = OpPhi %bool %false %87 %123 %106
OpStore %ok %124
%127 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%128 = OpLoad %v2float %127
%129 = OpCompositeExtract %float %128 1
%131 = OpCompositeConstruct %v2float %129 %float_0
%132 = OpCompositeConstruct %v2float %float_0 %129
%130 = OpCompositeConstruct %mat2v2float %131 %132
OpStore %m5 %130
%133 = OpLoad %bool %ok
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %mat2v2float %m5
%137 = OpCompositeConstruct %v2float %float_4 %float_0
%138 = OpCompositeConstruct %v2float %float_0 %float_4
%139 = OpCompositeConstruct %mat2v2float %137 %138
%140 = OpCompositeExtract %v2float %136 0
%141 = OpCompositeExtract %v2float %139 0
%142 = OpFOrdEqual %v2bool %140 %141
%143 = OpAll %bool %142
%144 = OpCompositeExtract %v2float %136 1
%145 = OpCompositeExtract %v2float %139 1
%146 = OpFOrdEqual %v2bool %144 %145
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %143 %147
OpBranch %135
%135 = OpLabel
%149 = OpPhi %bool %false %107 %148 %134
OpStore %ok %149
%150 = OpLoad %mat2v2float %m1
%151 = OpLoad %mat2v2float %m5
%152 = OpCompositeExtract %v2float %150 0
%153 = OpCompositeExtract %v2float %151 0
%154 = OpFAdd %v2float %152 %153
%155 = OpCompositeExtract %v2float %150 1
%156 = OpCompositeExtract %v2float %151 1
%157 = OpFAdd %v2float %155 %156
%158 = OpCompositeConstruct %mat2v2float %154 %157
OpStore %m1 %158
%159 = OpLoad %bool %ok
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %mat2v2float %m1
%165 = OpCompositeConstruct %v2float %float_5 %float_2
%166 = OpCompositeConstruct %v2float %float_3 %float_8
%167 = OpCompositeConstruct %mat2v2float %165 %166
%168 = OpCompositeExtract %v2float %162 0
%169 = OpCompositeExtract %v2float %167 0
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpCompositeExtract %v2float %162 1
%173 = OpCompositeExtract %v2float %167 1
%174 = OpFOrdEqual %v2bool %172 %173
%175 = OpAll %bool %174
%176 = OpLogicalAnd %bool %171 %175
OpBranch %161
%161 = OpLabel
%177 = OpPhi %bool %false %135 %176 %160
OpStore %ok %177
%180 = OpCompositeConstruct %v2float %float_5 %float_6
%181 = OpCompositeConstruct %v2float %float_7 %float_8
%182 = OpCompositeConstruct %mat2v2float %180 %181
OpStore %m7 %182
%183 = OpLoad %bool %ok
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %mat2v2float %m7
%187 = OpCompositeConstruct %v2float %float_5 %float_6
%188 = OpCompositeConstruct %v2float %float_7 %float_8
%189 = OpCompositeConstruct %mat2v2float %187 %188
%190 = OpCompositeExtract %v2float %186 0
%191 = OpCompositeExtract %v2float %189 0
%192 = OpFOrdEqual %v2bool %190 %191
%193 = OpAll %bool %192
%194 = OpCompositeExtract %v2float %186 1
%195 = OpCompositeExtract %v2float %189 1
%196 = OpFOrdEqual %v2bool %194 %195
%197 = OpAll %bool %196
%198 = OpLogicalAnd %bool %193 %197
OpBranch %185
%185 = OpLabel
%199 = OpPhi %bool %false %161 %198 %184
OpStore %ok %199
%206 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%207 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%208 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%205 = OpCompositeConstruct %mat3v3float %206 %207 %208
OpStore %m9 %205
%209 = OpLoad %bool %ok
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%212 = OpLoad %mat3v3float %m9
%213 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%215 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%216 = OpCompositeConstruct %mat3v3float %213 %214 %215
%218 = OpCompositeExtract %v3float %212 0
%219 = OpCompositeExtract %v3float %216 0
%220 = OpFOrdEqual %v3bool %218 %219
%221 = OpAll %bool %220
%222 = OpCompositeExtract %v3float %212 1
%223 = OpCompositeExtract %v3float %216 1
%224 = OpFOrdEqual %v3bool %222 %223
%225 = OpAll %bool %224
%226 = OpLogicalAnd %bool %221 %225
%227 = OpCompositeExtract %v3float %212 2
%228 = OpCompositeExtract %v3float %216 2
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
%231 = OpLogicalAnd %bool %226 %230
OpBranch %211
%211 = OpLabel
%232 = OpPhi %bool %false %185 %231 %210
OpStore %ok %232
%238 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%239 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%240 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%241 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%237 = OpCompositeConstruct %mat4v4float %238 %239 %240 %241
OpStore %m10 %237
%242 = OpLoad %bool %ok
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%245 = OpLoad %mat4v4float %m10
%246 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%247 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%248 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%249 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%250 = OpCompositeConstruct %mat4v4float %246 %247 %248 %249
%252 = OpCompositeExtract %v4float %245 0
%253 = OpCompositeExtract %v4float %250 0
%254 = OpFOrdEqual %v4bool %252 %253
%255 = OpAll %bool %254
%256 = OpCompositeExtract %v4float %245 1
%257 = OpCompositeExtract %v4float %250 1
%258 = OpFOrdEqual %v4bool %256 %257
%259 = OpAll %bool %258
%260 = OpLogicalAnd %bool %255 %259
%261 = OpCompositeExtract %v4float %245 2
%262 = OpCompositeExtract %v4float %250 2
%263 = OpFOrdEqual %v4bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
%266 = OpCompositeExtract %v4float %245 3
%267 = OpCompositeExtract %v4float %250 3
%268 = OpFOrdEqual %v4bool %266 %267
%269 = OpAll %bool %268
%270 = OpLogicalAnd %bool %265 %269
OpBranch %244
%244 = OpLabel
%271 = OpPhi %bool %false %211 %270 %243
OpStore %ok %271
%274 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%275 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%276 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%277 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%278 = OpCompositeConstruct %mat4v4float %274 %275 %276 %277
OpStore %m11 %278
%279 = OpLoad %mat4v4float %m11
%280 = OpLoad %mat4v4float %m10
%281 = OpCompositeExtract %v4float %279 0
%282 = OpCompositeExtract %v4float %280 0
%283 = OpFSub %v4float %281 %282
%284 = OpCompositeExtract %v4float %279 1
%285 = OpCompositeExtract %v4float %280 1
%286 = OpFSub %v4float %284 %285
%287 = OpCompositeExtract %v4float %279 2
%288 = OpCompositeExtract %v4float %280 2
%289 = OpFSub %v4float %287 %288
%290 = OpCompositeExtract %v4float %279 3
%291 = OpCompositeExtract %v4float %280 3
%292 = OpFSub %v4float %290 %291
%293 = OpCompositeConstruct %mat4v4float %283 %286 %289 %292
OpStore %m11 %293
%294 = OpLoad %bool %ok
OpSelectionMerge %296 None
OpBranchConditional %294 %295 %296
%295 = OpLabel
%297 = OpLoad %mat4v4float %m11
%298 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%299 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%300 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%301 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%302 = OpCompositeConstruct %mat4v4float %298 %299 %300 %301
%303 = OpCompositeExtract %v4float %297 0
%304 = OpCompositeExtract %v4float %302 0
%305 = OpFOrdEqual %v4bool %303 %304
%306 = OpAll %bool %305
%307 = OpCompositeExtract %v4float %297 1
%308 = OpCompositeExtract %v4float %302 1
%309 = OpFOrdEqual %v4bool %307 %308
%310 = OpAll %bool %309
%311 = OpLogicalAnd %bool %306 %310
%312 = OpCompositeExtract %v4float %297 2
%313 = OpCompositeExtract %v4float %302 2
%314 = OpFOrdEqual %v4bool %312 %313
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %311 %315
%317 = OpCompositeExtract %v4float %297 3
%318 = OpCompositeExtract %v4float %302 3
%319 = OpFOrdEqual %v4bool %317 %318
%320 = OpAll %bool %319
%321 = OpLogicalAnd %bool %316 %320
OpBranch %296
%296 = OpLabel
%322 = OpPhi %bool %false %244 %321 %295
OpStore %ok %322
%325 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%326 = OpLoad %mat2v2float %325
%327 = OpCompositeExtract %float %326 0 0
%328 = OpCompositeExtract %float %326 0 1
%329 = OpCompositeExtract %float %326 1 0
%330 = OpCompositeExtract %float %326 1 1
%331 = OpCompositeConstruct %v4float %327 %328 %329 %330
OpStore %h4 %331
%332 = OpLoad %bool %ok
OpSelectionMerge %334 None
OpBranchConditional %332 %333 %334
%333 = OpLabel
%335 = OpLoad %v4float %h4
%336 = OpVectorShuffle %v2float %335 %335 0 1
%337 = OpLoad %v4float %h4
%338 = OpCompositeExtract %float %337 2
%339 = OpCompositeConstruct %v2float %338 %float_4
%340 = OpCompositeConstruct %mat2v2float %336 %339
%341 = OpCompositeConstruct %v2float %float_1 %float_2
%342 = OpCompositeConstruct %v2float %float_3 %float_4
%343 = OpCompositeConstruct %mat2v2float %341 %342
%344 = OpCompositeExtract %v2float %340 0
%345 = OpCompositeExtract %v2float %343 0
%346 = OpFOrdEqual %v2bool %344 %345
%347 = OpAll %bool %346
%348 = OpCompositeExtract %v2float %340 1
%349 = OpCompositeExtract %v2float %343 1
%350 = OpFOrdEqual %v2bool %348 %349
%351 = OpAll %bool %350
%352 = OpLogicalAnd %bool %347 %351
OpBranch %334
%334 = OpLabel
%353 = OpPhi %bool %false %296 %352 %333
OpStore %ok %353
%354 = OpLoad %bool %ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%357 = OpLoad %v4float %h4
%358 = OpVectorShuffle %v2float %357 %357 0 1
%359 = OpLoad %v4float %h4
%360 = OpCompositeExtract %float %359 2
%361 = OpLoad %v4float %h4
%362 = OpCompositeExtract %float %361 3
%363 = OpLoad %v4float %h4
%364 = OpVectorShuffle %v2float %363 %363 0 1
%365 = OpLoad %v4float %h4
%366 = OpVectorShuffle %v2float %365 %365 2 3
%367 = OpLoad %v4float %h4
%368 = OpCompositeExtract %float %367 0
%369 = OpCompositeExtract %float %358 0
%370 = OpCompositeExtract %float %358 1
%371 = OpCompositeConstruct %v3float %369 %370 %360
%372 = OpCompositeExtract %float %364 0
%373 = OpCompositeExtract %float %364 1
%374 = OpCompositeConstruct %v3float %362 %372 %373
%375 = OpCompositeExtract %float %366 0
%376 = OpCompositeExtract %float %366 1
%377 = OpCompositeConstruct %v3float %375 %376 %368
%378 = OpCompositeConstruct %mat3v3float %371 %374 %377
%379 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%380 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%381 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%382 = OpCompositeConstruct %mat3v3float %379 %380 %381
%383 = OpCompositeExtract %v3float %378 0
%384 = OpCompositeExtract %v3float %382 0
%385 = OpFOrdEqual %v3bool %383 %384
%386 = OpAll %bool %385
%387 = OpCompositeExtract %v3float %378 1
%388 = OpCompositeExtract %v3float %382 1
%389 = OpFOrdEqual %v3bool %387 %388
%390 = OpAll %bool %389
%391 = OpLogicalAnd %bool %386 %390
%392 = OpCompositeExtract %v3float %378 2
%393 = OpCompositeExtract %v3float %382 2
%394 = OpFOrdEqual %v3bool %392 %393
%395 = OpAll %bool %394
%396 = OpLogicalAnd %bool %391 %395
OpBranch %356
%356 = OpLabel
%397 = OpPhi %bool %false %334 %396 %355
OpStore %ok %397
%398 = OpLoad %bool %ok
OpSelectionMerge %400 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%401 = OpLoad %v4float %h4
%402 = OpVectorShuffle %v2float %401 %401 0 1
%403 = OpLoad %v4float %h4
%404 = OpVectorShuffle %v2float %403 %403 2 3
%405 = OpLoad %v4float %h4
%406 = OpVectorShuffle %v3float %405 %405 0 1 2
%407 = OpLoad %v4float %h4
%408 = OpCompositeExtract %float %407 3
%409 = OpLoad %v4float %h4
%410 = OpLoad %v4float %h4
%411 = OpVectorShuffle %v3float %410 %410 1 2 3
%412 = OpCompositeExtract %float %402 0
%413 = OpCompositeExtract %float %402 1
%414 = OpCompositeExtract %float %404 0
%415 = OpCompositeExtract %float %404 1
%416 = OpCompositeConstruct %v4float %412 %413 %414 %415
%417 = OpCompositeExtract %float %406 0
%418 = OpCompositeExtract %float %406 1
%419 = OpCompositeExtract %float %406 2
%420 = OpCompositeConstruct %v4float %417 %418 %419 %408
%421 = OpCompositeExtract %float %411 0
%422 = OpCompositeExtract %float %411 1
%423 = OpCompositeExtract %float %411 2
%424 = OpCompositeConstruct %v4float %float_1 %421 %422 %423
%425 = OpCompositeConstruct %mat4v4float %416 %420 %409 %424
%426 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%427 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%428 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%429 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%430 = OpCompositeConstruct %mat4v4float %426 %427 %428 %429
%431 = OpCompositeExtract %v4float %425 0
%432 = OpCompositeExtract %v4float %430 0
%433 = OpFOrdEqual %v4bool %431 %432
%434 = OpAll %bool %433
%435 = OpCompositeExtract %v4float %425 1
%436 = OpCompositeExtract %v4float %430 1
%437 = OpFOrdEqual %v4bool %435 %436
%438 = OpAll %bool %437
%439 = OpLogicalAnd %bool %434 %438
%440 = OpCompositeExtract %v4float %425 2
%441 = OpCompositeExtract %v4float %430 2
%442 = OpFOrdEqual %v4bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
%445 = OpCompositeExtract %v4float %425 3
%446 = OpCompositeExtract %v4float %430 3
%447 = OpFOrdEqual %v4bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
OpBranch %400
%400 = OpLabel
%450 = OpPhi %bool %false %356 %449 %399
OpStore %ok %450
%451 = OpLoad %bool %ok
OpReturnValue %451
OpFunctionEnd
%test_comma_b = OpFunction %bool None %26
%452 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%455 = OpCompositeConstruct %v2float %float_1 %float_2
%456 = OpCompositeConstruct %v2float %float_3 %float_4
%457 = OpCompositeConstruct %mat2v2float %455 %456
OpStore %x %457
%459 = OpCompositeConstruct %v2float %float_2 %float_4
%460 = OpCompositeConstruct %v2float %float_6 %float_8
%461 = OpCompositeConstruct %mat2v2float %459 %460
%462 = OpMatrixTimesScalar %mat2v2float %461 %float_0_5
OpStore %y %462
%463 = OpLoad %mat2v2float %x
%464 = OpLoad %mat2v2float %y
%465 = OpCompositeExtract %v2float %463 0
%466 = OpCompositeExtract %v2float %464 0
%467 = OpFOrdEqual %v2bool %465 %466
%468 = OpAll %bool %467
%469 = OpCompositeExtract %v2float %463 1
%470 = OpCompositeExtract %v2float %464 1
%471 = OpFOrdEqual %v2bool %469 %470
%472 = OpAll %bool %471
%473 = OpLogicalAnd %bool %468 %472
OpReturnValue %473
OpFunctionEnd
%main = OpFunction %v4float None %474
%475 = OpFunctionParameter %_ptr_Function_v2float
%476 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_9_f4 = OpVariable %_ptr_Function_v4float Function
%878 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%479 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%480 = OpLoad %mat2v2float %479
OpStore %_1_m1 %480
%481 = OpLoad %bool %_0_ok
OpSelectionMerge %483 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%484 = OpLoad %mat2v2float %_1_m1
%485 = OpCompositeConstruct %v2float %float_1 %float_2
%486 = OpCompositeConstruct %v2float %float_3 %float_4
%487 = OpCompositeConstruct %mat2v2float %485 %486
%488 = OpCompositeExtract %v2float %484 0
%489 = OpCompositeExtract %v2float %487 0
%490 = OpFOrdEqual %v2bool %488 %489
%491 = OpAll %bool %490
%492 = OpCompositeExtract %v2float %484 1
%493 = OpCompositeExtract %v2float %487 1
%494 = OpFOrdEqual %v2bool %492 %493
%495 = OpAll %bool %494
%496 = OpLogicalAnd %bool %491 %495
OpBranch %483
%483 = OpLabel
%497 = OpPhi %bool %false %476 %496 %482
OpStore %_0_ok %497
%499 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %499
%500 = OpLoad %bool %_0_ok
OpSelectionMerge %502 None
OpBranchConditional %500 %501 %502
%501 = OpLabel
%503 = OpLoad %mat2v2float %_2_m3
%504 = OpCompositeConstruct %v2float %float_1 %float_2
%505 = OpCompositeConstruct %v2float %float_3 %float_4
%506 = OpCompositeConstruct %mat2v2float %504 %505
%507 = OpCompositeExtract %v2float %503 0
%508 = OpCompositeExtract %v2float %506 0
%509 = OpFOrdEqual %v2bool %507 %508
%510 = OpAll %bool %509
%511 = OpCompositeExtract %v2float %503 1
%512 = OpCompositeExtract %v2float %506 1
%513 = OpFOrdEqual %v2bool %511 %512
%514 = OpAll %bool %513
%515 = OpLogicalAnd %bool %510 %514
OpBranch %502
%502 = OpLabel
%516 = OpPhi %bool %false %483 %515 %501
OpStore %_0_ok %516
%519 = OpCompositeConstruct %v2float %float_6 %float_0
%520 = OpCompositeConstruct %v2float %float_0 %float_6
%518 = OpCompositeConstruct %mat2v2float %519 %520
OpStore %_3_m4 %518
%521 = OpLoad %bool %_0_ok
OpSelectionMerge %523 None
OpBranchConditional %521 %522 %523
%522 = OpLabel
%524 = OpLoad %mat2v2float %_3_m4
%525 = OpCompositeConstruct %v2float %float_6 %float_0
%526 = OpCompositeConstruct %v2float %float_0 %float_6
%527 = OpCompositeConstruct %mat2v2float %525 %526
%528 = OpCompositeExtract %v2float %524 0
%529 = OpCompositeExtract %v2float %527 0
%530 = OpFOrdEqual %v2bool %528 %529
%531 = OpAll %bool %530
%532 = OpCompositeExtract %v2float %524 1
%533 = OpCompositeExtract %v2float %527 1
%534 = OpFOrdEqual %v2bool %532 %533
%535 = OpAll %bool %534
%536 = OpLogicalAnd %bool %531 %535
OpBranch %523
%523 = OpLabel
%537 = OpPhi %bool %false %502 %536 %522
OpStore %_0_ok %537
%538 = OpLoad %mat2v2float %_2_m3
%539 = OpLoad %mat2v2float %_3_m4
%540 = OpMatrixTimesMatrix %mat2v2float %538 %539
OpStore %_2_m3 %540
%541 = OpLoad %bool %_0_ok
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpLoad %mat2v2float %_2_m3
%545 = OpCompositeConstruct %v2float %float_6 %float_12
%546 = OpCompositeConstruct %v2float %float_18 %float_24
%547 = OpCompositeConstruct %mat2v2float %545 %546
%548 = OpCompositeExtract %v2float %544 0
%549 = OpCompositeExtract %v2float %547 0
%550 = OpFOrdEqual %v2bool %548 %549
%551 = OpAll %bool %550
%552 = OpCompositeExtract %v2float %544 1
%553 = OpCompositeExtract %v2float %547 1
%554 = OpFOrdEqual %v2bool %552 %553
%555 = OpAll %bool %554
%556 = OpLogicalAnd %bool %551 %555
OpBranch %543
%543 = OpLabel
%557 = OpPhi %bool %false %523 %556 %542
OpStore %_0_ok %557
%559 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%560 = OpLoad %v2float %559
%561 = OpCompositeExtract %float %560 1
%563 = OpCompositeConstruct %v2float %561 %float_0
%564 = OpCompositeConstruct %v2float %float_0 %561
%562 = OpCompositeConstruct %mat2v2float %563 %564
OpStore %_4_m5 %562
%565 = OpLoad %bool %_0_ok
OpSelectionMerge %567 None
OpBranchConditional %565 %566 %567
%566 = OpLabel
%568 = OpLoad %mat2v2float %_4_m5
%569 = OpCompositeConstruct %v2float %float_4 %float_0
%570 = OpCompositeConstruct %v2float %float_0 %float_4
%571 = OpCompositeConstruct %mat2v2float %569 %570
%572 = OpCompositeExtract %v2float %568 0
%573 = OpCompositeExtract %v2float %571 0
%574 = OpFOrdEqual %v2bool %572 %573
%575 = OpAll %bool %574
%576 = OpCompositeExtract %v2float %568 1
%577 = OpCompositeExtract %v2float %571 1
%578 = OpFOrdEqual %v2bool %576 %577
%579 = OpAll %bool %578
%580 = OpLogicalAnd %bool %575 %579
OpBranch %567
%567 = OpLabel
%581 = OpPhi %bool %false %543 %580 %566
OpStore %_0_ok %581
%582 = OpLoad %mat2v2float %_1_m1
%583 = OpLoad %mat2v2float %_4_m5
%584 = OpCompositeExtract %v2float %582 0
%585 = OpCompositeExtract %v2float %583 0
%586 = OpFAdd %v2float %584 %585
%587 = OpCompositeExtract %v2float %582 1
%588 = OpCompositeExtract %v2float %583 1
%589 = OpFAdd %v2float %587 %588
%590 = OpCompositeConstruct %mat2v2float %586 %589
OpStore %_1_m1 %590
%591 = OpLoad %bool %_0_ok
OpSelectionMerge %593 None
OpBranchConditional %591 %592 %593
%592 = OpLabel
%594 = OpLoad %mat2v2float %_1_m1
%595 = OpCompositeConstruct %v2float %float_5 %float_2
%596 = OpCompositeConstruct %v2float %float_3 %float_8
%597 = OpCompositeConstruct %mat2v2float %595 %596
%598 = OpCompositeExtract %v2float %594 0
%599 = OpCompositeExtract %v2float %597 0
%600 = OpFOrdEqual %v2bool %598 %599
%601 = OpAll %bool %600
%602 = OpCompositeExtract %v2float %594 1
%603 = OpCompositeExtract %v2float %597 1
%604 = OpFOrdEqual %v2bool %602 %603
%605 = OpAll %bool %604
%606 = OpLogicalAnd %bool %601 %605
OpBranch %593
%593 = OpLabel
%607 = OpPhi %bool %false %567 %606 %592
OpStore %_0_ok %607
%609 = OpCompositeConstruct %v2float %float_5 %float_6
%610 = OpCompositeConstruct %v2float %float_7 %float_8
%611 = OpCompositeConstruct %mat2v2float %609 %610
OpStore %_5_m7 %611
%612 = OpLoad %bool %_0_ok
OpSelectionMerge %614 None
OpBranchConditional %612 %613 %614
%613 = OpLabel
%615 = OpLoad %mat2v2float %_5_m7
%616 = OpCompositeConstruct %v2float %float_5 %float_6
%617 = OpCompositeConstruct %v2float %float_7 %float_8
%618 = OpCompositeConstruct %mat2v2float %616 %617
%619 = OpCompositeExtract %v2float %615 0
%620 = OpCompositeExtract %v2float %618 0
%621 = OpFOrdEqual %v2bool %619 %620
%622 = OpAll %bool %621
%623 = OpCompositeExtract %v2float %615 1
%624 = OpCompositeExtract %v2float %618 1
%625 = OpFOrdEqual %v2bool %623 %624
%626 = OpAll %bool %625
%627 = OpLogicalAnd %bool %622 %626
OpBranch %614
%614 = OpLabel
%628 = OpPhi %bool %false %593 %627 %613
OpStore %_0_ok %628
%631 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%632 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%633 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%630 = OpCompositeConstruct %mat3v3float %631 %632 %633
OpStore %_6_m9 %630
%634 = OpLoad %bool %_0_ok
OpSelectionMerge %636 None
OpBranchConditional %634 %635 %636
%635 = OpLabel
%637 = OpLoad %mat3v3float %_6_m9
%638 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%639 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%640 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%641 = OpCompositeConstruct %mat3v3float %638 %639 %640
%642 = OpCompositeExtract %v3float %637 0
%643 = OpCompositeExtract %v3float %641 0
%644 = OpFOrdEqual %v3bool %642 %643
%645 = OpAll %bool %644
%646 = OpCompositeExtract %v3float %637 1
%647 = OpCompositeExtract %v3float %641 1
%648 = OpFOrdEqual %v3bool %646 %647
%649 = OpAll %bool %648
%650 = OpLogicalAnd %bool %645 %649
%651 = OpCompositeExtract %v3float %637 2
%652 = OpCompositeExtract %v3float %641 2
%653 = OpFOrdEqual %v3bool %651 %652
%654 = OpAll %bool %653
%655 = OpLogicalAnd %bool %650 %654
OpBranch %636
%636 = OpLabel
%656 = OpPhi %bool %false %614 %655 %635
OpStore %_0_ok %656
%659 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%660 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%661 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%662 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%658 = OpCompositeConstruct %mat4v4float %659 %660 %661 %662
OpStore %_7_m10 %658
%663 = OpLoad %bool %_0_ok
OpSelectionMerge %665 None
OpBranchConditional %663 %664 %665
%664 = OpLabel
%666 = OpLoad %mat4v4float %_7_m10
%667 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%668 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%669 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%670 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%671 = OpCompositeConstruct %mat4v4float %667 %668 %669 %670
%672 = OpCompositeExtract %v4float %666 0
%673 = OpCompositeExtract %v4float %671 0
%674 = OpFOrdEqual %v4bool %672 %673
%675 = OpAll %bool %674
%676 = OpCompositeExtract %v4float %666 1
%677 = OpCompositeExtract %v4float %671 1
%678 = OpFOrdEqual %v4bool %676 %677
%679 = OpAll %bool %678
%680 = OpLogicalAnd %bool %675 %679
%681 = OpCompositeExtract %v4float %666 2
%682 = OpCompositeExtract %v4float %671 2
%683 = OpFOrdEqual %v4bool %681 %682
%684 = OpAll %bool %683
%685 = OpLogicalAnd %bool %680 %684
%686 = OpCompositeExtract %v4float %666 3
%687 = OpCompositeExtract %v4float %671 3
%688 = OpFOrdEqual %v4bool %686 %687
%689 = OpAll %bool %688
%690 = OpLogicalAnd %bool %685 %689
OpBranch %665
%665 = OpLabel
%691 = OpPhi %bool %false %636 %690 %664
OpStore %_0_ok %691
%693 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%694 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%695 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%696 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%697 = OpCompositeConstruct %mat4v4float %693 %694 %695 %696
OpStore %_8_m11 %697
%698 = OpLoad %mat4v4float %_8_m11
%699 = OpLoad %mat4v4float %_7_m10
%700 = OpCompositeExtract %v4float %698 0
%701 = OpCompositeExtract %v4float %699 0
%702 = OpFSub %v4float %700 %701
%703 = OpCompositeExtract %v4float %698 1
%704 = OpCompositeExtract %v4float %699 1
%705 = OpFSub %v4float %703 %704
%706 = OpCompositeExtract %v4float %698 2
%707 = OpCompositeExtract %v4float %699 2
%708 = OpFSub %v4float %706 %707
%709 = OpCompositeExtract %v4float %698 3
%710 = OpCompositeExtract %v4float %699 3
%711 = OpFSub %v4float %709 %710
%712 = OpCompositeConstruct %mat4v4float %702 %705 %708 %711
OpStore %_8_m11 %712
%713 = OpLoad %bool %_0_ok
OpSelectionMerge %715 None
OpBranchConditional %713 %714 %715
%714 = OpLabel
%716 = OpLoad %mat4v4float %_8_m11
%717 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%718 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%719 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%720 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%721 = OpCompositeConstruct %mat4v4float %717 %718 %719 %720
%722 = OpCompositeExtract %v4float %716 0
%723 = OpCompositeExtract %v4float %721 0
%724 = OpFOrdEqual %v4bool %722 %723
%725 = OpAll %bool %724
%726 = OpCompositeExtract %v4float %716 1
%727 = OpCompositeExtract %v4float %721 1
%728 = OpFOrdEqual %v4bool %726 %727
%729 = OpAll %bool %728
%730 = OpLogicalAnd %bool %725 %729
%731 = OpCompositeExtract %v4float %716 2
%732 = OpCompositeExtract %v4float %721 2
%733 = OpFOrdEqual %v4bool %731 %732
%734 = OpAll %bool %733
%735 = OpLogicalAnd %bool %730 %734
%736 = OpCompositeExtract %v4float %716 3
%737 = OpCompositeExtract %v4float %721 3
%738 = OpFOrdEqual %v4bool %736 %737
%739 = OpAll %bool %738
%740 = OpLogicalAnd %bool %735 %739
OpBranch %715
%715 = OpLabel
%741 = OpPhi %bool %false %665 %740 %714
OpStore %_0_ok %741
%743 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_2
%744 = OpLoad %mat2v2float %743
%745 = OpCompositeExtract %float %744 0 0
%746 = OpCompositeExtract %float %744 0 1
%747 = OpCompositeExtract %float %744 1 0
%748 = OpCompositeExtract %float %744 1 1
%749 = OpCompositeConstruct %v4float %745 %746 %747 %748
OpStore %_9_f4 %749
%750 = OpLoad %bool %_0_ok
OpSelectionMerge %752 None
OpBranchConditional %750 %751 %752
%751 = OpLabel
%753 = OpLoad %v4float %_9_f4
%754 = OpVectorShuffle %v2float %753 %753 0 1
%755 = OpLoad %v4float %_9_f4
%756 = OpCompositeExtract %float %755 2
%757 = OpCompositeConstruct %v2float %756 %float_4
%758 = OpCompositeConstruct %mat2v2float %754 %757
%759 = OpCompositeConstruct %v2float %float_1 %float_2
%760 = OpCompositeConstruct %v2float %float_3 %float_4
%761 = OpCompositeConstruct %mat2v2float %759 %760
%762 = OpCompositeExtract %v2float %758 0
%763 = OpCompositeExtract %v2float %761 0
%764 = OpFOrdEqual %v2bool %762 %763
%765 = OpAll %bool %764
%766 = OpCompositeExtract %v2float %758 1
%767 = OpCompositeExtract %v2float %761 1
%768 = OpFOrdEqual %v2bool %766 %767
%769 = OpAll %bool %768
%770 = OpLogicalAnd %bool %765 %769
OpBranch %752
%752 = OpLabel
%771 = OpPhi %bool %false %715 %770 %751
OpStore %_0_ok %771
%772 = OpLoad %bool %_0_ok
OpSelectionMerge %774 None
OpBranchConditional %772 %773 %774
%773 = OpLabel
%775 = OpLoad %v4float %_9_f4
%776 = OpVectorShuffle %v2float %775 %775 0 1
%777 = OpLoad %v4float %_9_f4
%778 = OpCompositeExtract %float %777 2
%779 = OpLoad %v4float %_9_f4
%780 = OpCompositeExtract %float %779 3
%781 = OpLoad %v4float %_9_f4
%782 = OpVectorShuffle %v2float %781 %781 0 1
%783 = OpLoad %v4float %_9_f4
%784 = OpVectorShuffle %v2float %783 %783 2 3
%785 = OpLoad %v4float %_9_f4
%786 = OpCompositeExtract %float %785 0
%787 = OpCompositeExtract %float %776 0
%788 = OpCompositeExtract %float %776 1
%789 = OpCompositeConstruct %v3float %787 %788 %778
%790 = OpCompositeExtract %float %782 0
%791 = OpCompositeExtract %float %782 1
%792 = OpCompositeConstruct %v3float %780 %790 %791
%793 = OpCompositeExtract %float %784 0
%794 = OpCompositeExtract %float %784 1
%795 = OpCompositeConstruct %v3float %793 %794 %786
%796 = OpCompositeConstruct %mat3v3float %789 %792 %795
%797 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%798 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%799 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%800 = OpCompositeConstruct %mat3v3float %797 %798 %799
%801 = OpCompositeExtract %v3float %796 0
%802 = OpCompositeExtract %v3float %800 0
%803 = OpFOrdEqual %v3bool %801 %802
%804 = OpAll %bool %803
%805 = OpCompositeExtract %v3float %796 1
%806 = OpCompositeExtract %v3float %800 1
%807 = OpFOrdEqual %v3bool %805 %806
%808 = OpAll %bool %807
%809 = OpLogicalAnd %bool %804 %808
%810 = OpCompositeExtract %v3float %796 2
%811 = OpCompositeExtract %v3float %800 2
%812 = OpFOrdEqual %v3bool %810 %811
%813 = OpAll %bool %812
%814 = OpLogicalAnd %bool %809 %813
OpBranch %774
%774 = OpLabel
%815 = OpPhi %bool %false %752 %814 %773
OpStore %_0_ok %815
%816 = OpLoad %bool %_0_ok
OpSelectionMerge %818 None
OpBranchConditional %816 %817 %818
%817 = OpLabel
%819 = OpLoad %v4float %_9_f4
%820 = OpVectorShuffle %v2float %819 %819 0 1
%821 = OpLoad %v4float %_9_f4
%822 = OpVectorShuffle %v2float %821 %821 2 3
%823 = OpLoad %v4float %_9_f4
%824 = OpVectorShuffle %v3float %823 %823 0 1 2
%825 = OpLoad %v4float %_9_f4
%826 = OpCompositeExtract %float %825 3
%827 = OpLoad %v4float %_9_f4
%828 = OpLoad %v4float %_9_f4
%829 = OpVectorShuffle %v3float %828 %828 1 2 3
%830 = OpCompositeExtract %float %820 0
%831 = OpCompositeExtract %float %820 1
%832 = OpCompositeExtract %float %822 0
%833 = OpCompositeExtract %float %822 1
%834 = OpCompositeConstruct %v4float %830 %831 %832 %833
%835 = OpCompositeExtract %float %824 0
%836 = OpCompositeExtract %float %824 1
%837 = OpCompositeExtract %float %824 2
%838 = OpCompositeConstruct %v4float %835 %836 %837 %826
%839 = OpCompositeExtract %float %829 0
%840 = OpCompositeExtract %float %829 1
%841 = OpCompositeExtract %float %829 2
%842 = OpCompositeConstruct %v4float %float_1 %839 %840 %841
%843 = OpCompositeConstruct %mat4v4float %834 %838 %827 %842
%844 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%845 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%846 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%847 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%848 = OpCompositeConstruct %mat4v4float %844 %845 %846 %847
%849 = OpCompositeExtract %v4float %843 0
%850 = OpCompositeExtract %v4float %848 0
%851 = OpFOrdEqual %v4bool %849 %850
%852 = OpAll %bool %851
%853 = OpCompositeExtract %v4float %843 1
%854 = OpCompositeExtract %v4float %848 1
%855 = OpFOrdEqual %v4bool %853 %854
%856 = OpAll %bool %855
%857 = OpLogicalAnd %bool %852 %856
%858 = OpCompositeExtract %v4float %843 2
%859 = OpCompositeExtract %v4float %848 2
%860 = OpFOrdEqual %v4bool %858 %859
%861 = OpAll %bool %860
%862 = OpLogicalAnd %bool %857 %861
%863 = OpCompositeExtract %v4float %843 3
%864 = OpCompositeExtract %v4float %848 3
%865 = OpFOrdEqual %v4bool %863 %864
%866 = OpAll %bool %865
%867 = OpLogicalAnd %bool %862 %866
OpBranch %818
%818 = OpLabel
%868 = OpPhi %bool %false %774 %867 %817
OpStore %_0_ok %868
%869 = OpLoad %bool %_0_ok
OpSelectionMerge %871 None
OpBranchConditional %869 %870 %871
%870 = OpLabel
%872 = OpFunctionCall %bool %test_half_b
OpBranch %871
%871 = OpLabel
%873 = OpPhi %bool %false %818 %872 %870
OpSelectionMerge %875 None
OpBranchConditional %873 %874 %875
%874 = OpLabel
%876 = OpFunctionCall %bool %test_comma_b
OpBranch %875
%875 = OpLabel
%877 = OpPhi %bool %false %871 %876 %874
OpSelectionMerge %881 None
OpBranchConditional %877 %879 %880
%879 = OpLabel
%882 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%885 = OpLoad %v4float %882
OpStore %878 %885
OpBranch %881
%880 = OpLabel
%886 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%887 = OpLoad %v4float %886
OpStore %878 %887
OpBranch %881
%881 = OpLabel
%888 = OpLoad %v4float %878
OpReturnValue %888
OpFunctionEnd
