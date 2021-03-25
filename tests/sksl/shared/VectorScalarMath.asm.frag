OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_int_b "test_int_b"
OpName %ok "ok"
OpName %inputRed "inputRed"
OpName %inputGreen "inputGreen"
OpName %x "x"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_inputRed "_1_inputRed"
OpName %_2_inputGreen "_2_inputGreen"
OpName %_3_x "_3_x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%65 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%80 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%93 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%109 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%124 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%137 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%160 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%173 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%203 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%215 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%258 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%278 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%292 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%306 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%321 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%v2float = OpTypeVector %float 2
%float_0_5 = OpConstant %float 0.5
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%339 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%352 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%375 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%388 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%415 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%427 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %19
%20 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%31 = OpLoad %v4float %28
%32 = OpCompositeExtract %float %31 0
%33 = OpConvertFToS %int %32
%34 = OpCompositeExtract %float %31 1
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %31 2
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %31 3
%39 = OpConvertFToS %int %38
%40 = OpCompositeConstruct %v4int %33 %35 %37 %39
OpStore %inputRed %40
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%44 = OpLoad %v4float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpConvertFToS %int %45
%47 = OpCompositeExtract %float %44 1
%48 = OpConvertFToS %int %47
%49 = OpCompositeExtract %float %44 2
%50 = OpConvertFToS %int %49
%51 = OpCompositeExtract %float %44 3
%52 = OpConvertFToS %int %51
%53 = OpCompositeConstruct %v4int %46 %48 %50 %52
OpStore %inputGreen %53
%55 = OpLoad %v4int %inputRed
%57 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%58 = OpIAdd %v4int %55 %57
OpStore %x %58
%60 = OpLoad %bool %ok
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %v4int %x
%66 = OpIEqual %v4bool %63 %65
%68 = OpAll %bool %66
OpBranch %62
%62 = OpLabel
%69 = OpPhi %bool %false %20 %68 %61
OpStore %ok %69
%70 = OpLoad %v4int %inputGreen
%71 = OpVectorShuffle %v4int %70 %70 1 3 0 2
%72 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%73 = OpISub %v4int %71 %72
OpStore %x %73
%74 = OpLoad %bool %ok
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4int %x
%81 = OpIEqual %v4bool %77 %80
%82 = OpAll %bool %81
OpBranch %76
%76 = OpLabel
%83 = OpPhi %bool %false %62 %82 %75
OpStore %ok %83
%84 = OpLoad %v4int %inputRed
%85 = OpLoad %v4int %inputGreen
%86 = OpCompositeExtract %int %85 1
%87 = OpCompositeConstruct %v4int %86 %86 %86 %86
%88 = OpIAdd %v4int %84 %87
OpStore %x %88
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4int %x
%94 = OpIEqual %v4bool %92 %93
%95 = OpAll %bool %94
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %76 %95 %90
OpStore %ok %96
%97 = OpLoad %v4int %inputGreen
%98 = OpVectorShuffle %v3int %97 %97 3 1 3
%101 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%102 = OpIMul %v3int %98 %101
%103 = OpLoad %v4int %x
%104 = OpVectorShuffle %v4int %103 %102 4 5 6 3
OpStore %x %104
%105 = OpLoad %bool %ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4int %x
%110 = OpIEqual %v4bool %108 %109
%111 = OpAll %bool %110
OpBranch %107
%107 = OpLabel
%112 = OpPhi %bool %false %91 %111 %106
OpStore %ok %112
%113 = OpLoad %v4int %x
%114 = OpVectorShuffle %v2int %113 %113 2 3
%116 = OpCompositeConstruct %v2int %int_3 %int_3
%117 = OpSDiv %v2int %114 %116
%118 = OpLoad %v4int %x
%119 = OpVectorShuffle %v4int %118 %117 4 5 2 3
OpStore %x %119
%120 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpLoad %v4int %x
%125 = OpIEqual %v4bool %123 %124
%126 = OpAll %bool %125
OpBranch %122
%122 = OpLabel
%127 = OpPhi %bool %false %107 %126 %121
OpStore %ok %127
%128 = OpLoad %v4int %inputRed
%130 = OpCompositeConstruct %v4int %int_5 %int_5 %int_5 %int_5
%131 = OpIMul %v4int %128 %130
%132 = OpVectorShuffle %v4int %131 %131 1 0 3 2
OpStore %x %132
%133 = OpLoad %bool %ok
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %v4int %x
%138 = OpIEqual %v4bool %136 %137
%139 = OpAll %bool %138
OpBranch %135
%135 = OpLabel
%140 = OpPhi %bool %false %122 %139 %134
OpStore %ok %140
%141 = OpLoad %v4int %inputRed
%142 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%143 = OpIAdd %v4int %142 %141
OpStore %x %143
%144 = OpLoad %bool %ok
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpLoad %v4int %x
%148 = OpIEqual %v4bool %147 %65
%149 = OpAll %bool %148
OpBranch %146
%146 = OpLabel
%150 = OpPhi %bool %false %135 %149 %145
OpStore %ok %150
%152 = OpLoad %v4int %inputGreen
%153 = OpVectorShuffle %v4int %152 %152 1 3 0 2
%154 = OpCompositeConstruct %v4int %int_10 %int_10 %int_10 %int_10
%155 = OpISub %v4int %154 %153
OpStore %x %155
%156 = OpLoad %bool %ok
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpLoad %v4int %x
%161 = OpIEqual %v4bool %159 %160
%162 = OpAll %bool %161
OpBranch %158
%158 = OpLabel
%163 = OpPhi %bool %false %146 %162 %157
OpStore %ok %163
%164 = OpLoad %v4int %inputRed
%165 = OpCompositeExtract %int %164 0
%166 = OpLoad %v4int %inputGreen
%167 = OpCompositeConstruct %v4int %165 %165 %165 %165
%168 = OpIAdd %v4int %167 %166
OpStore %x %168
%169 = OpLoad %bool %ok
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %v4int %x
%174 = OpIEqual %v4bool %172 %173
%175 = OpAll %bool %174
OpBranch %171
%171 = OpLabel
%176 = OpPhi %bool %false %158 %175 %170
OpStore %ok %176
%177 = OpLoad %v4int %inputGreen
%178 = OpVectorShuffle %v3int %177 %177 3 1 3
%179 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%180 = OpIMul %v3int %179 %178
%181 = OpLoad %v4int %x
%182 = OpVectorShuffle %v4int %181 %180 4 5 6 3
OpStore %x %182
%183 = OpLoad %bool %ok
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v4int %x
%187 = OpIEqual %v4bool %186 %109
%188 = OpAll %bool %187
OpBranch %185
%185 = OpLabel
%189 = OpPhi %bool %false %171 %188 %184
OpStore %ok %189
%191 = OpLoad %v4int %x
%192 = OpVectorShuffle %v2int %191 %191 2 3
%193 = OpCompositeConstruct %v2int %int_36 %int_36
%194 = OpSDiv %v2int %193 %192
%195 = OpLoad %v4int %x
%196 = OpVectorShuffle %v4int %195 %194 4 5 2 3
OpStore %x %196
%197 = OpLoad %bool %ok
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %v4int %x
%204 = OpIEqual %v4bool %200 %203
%205 = OpAll %bool %204
OpBranch %199
%199 = OpLabel
%206 = OpPhi %bool %false %185 %205 %198
OpStore %ok %206
%207 = OpLoad %v4int %x
%208 = OpCompositeConstruct %v4int %int_36 %int_36 %int_36 %int_36
%209 = OpSDiv %v4int %208 %207
%210 = OpVectorShuffle %v4int %209 %209 1 0 3 2
OpStore %x %210
%211 = OpLoad %bool %ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpLoad %v4int %x
%216 = OpIEqual %v4bool %214 %215
%217 = OpAll %bool %216
OpBranch %213
%213 = OpLabel
%218 = OpPhi %bool %false %199 %217 %212
OpStore %ok %218
%219 = OpLoad %v4int %x
%220 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%221 = OpIAdd %v4int %219 %220
OpStore %x %221
%222 = OpLoad %v4int %x
%223 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%224 = OpIMul %v4int %222 %223
OpStore %x %224
%225 = OpLoad %v4int %x
%226 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%227 = OpISub %v4int %225 %226
OpStore %x %227
%228 = OpLoad %v4int %x
%229 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%230 = OpSDiv %v4int %228 %229
OpStore %x %230
%231 = OpLoad %bool %ok
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %v4int %x
%235 = OpIEqual %v4bool %234 %215
%236 = OpAll %bool %235
OpBranch %233
%233 = OpLabel
%237 = OpPhi %bool %false %213 %236 %232
OpStore %ok %237
%238 = OpLoad %v4int %x
%239 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%240 = OpIAdd %v4int %238 %239
OpStore %x %240
%241 = OpLoad %v4int %x
%242 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%243 = OpIMul %v4int %241 %242
OpStore %x %243
%244 = OpLoad %v4int %x
%245 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%246 = OpISub %v4int %244 %245
OpStore %x %246
%247 = OpLoad %v4int %x
%248 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%249 = OpSDiv %v4int %247 %248
OpStore %x %249
%250 = OpLoad %bool %ok
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %v4int %x
%254 = OpIEqual %v4bool %253 %215
%255 = OpAll %bool %254
OpBranch %252
%252 = OpLabel
%256 = OpPhi %bool %false %233 %255 %251
OpStore %ok %256
%257 = OpLoad %bool %ok
OpReturnValue %257
OpFunctionEnd
%main = OpFunction %v4float None %258
%259 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%472 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%263 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%264 = OpLoad %v4float %263
OpStore %_1_inputRed %264
%266 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%267 = OpLoad %v4float %266
OpStore %_2_inputGreen %267
%269 = OpLoad %v4float %_1_inputRed
%271 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%272 = OpFAdd %v4float %269 %271
OpStore %_3_x %272
%273 = OpLoad %bool %_0_ok
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpLoad %v4float %_3_x
%279 = OpFOrdEqual %v4bool %276 %278
%280 = OpAll %bool %279
OpBranch %275
%275 = OpLabel
%281 = OpPhi %bool %false %259 %280 %274
OpStore %_0_ok %281
%282 = OpLoad %v4float %_2_inputGreen
%283 = OpVectorShuffle %v4float %282 %282 1 3 0 2
%284 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%285 = OpFSub %v4float %283 %284
OpStore %_3_x %285
%286 = OpLoad %bool %_0_ok
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%289 = OpLoad %v4float %_3_x
%293 = OpFOrdEqual %v4bool %289 %292
%294 = OpAll %bool %293
OpBranch %288
%288 = OpLabel
%295 = OpPhi %bool %false %275 %294 %287
OpStore %_0_ok %295
%296 = OpLoad %v4float %_1_inputRed
%297 = OpLoad %v4float %_2_inputGreen
%298 = OpCompositeExtract %float %297 1
%299 = OpCompositeConstruct %v4float %298 %298 %298 %298
%300 = OpFAdd %v4float %296 %299
OpStore %_3_x %300
%301 = OpLoad %bool %_0_ok
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpLoad %v4float %_3_x
%307 = OpFOrdEqual %v4bool %304 %306
%308 = OpAll %bool %307
OpBranch %303
%303 = OpLabel
%309 = OpPhi %bool %false %288 %308 %302
OpStore %_0_ok %309
%310 = OpLoad %v4float %_2_inputGreen
%311 = OpVectorShuffle %v3float %310 %310 3 1 3
%314 = OpVectorTimesScalar %v3float %311 %float_9
%315 = OpLoad %v4float %_3_x
%316 = OpVectorShuffle %v4float %315 %314 4 5 6 3
OpStore %_3_x %316
%317 = OpLoad %bool %_0_ok
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%320 = OpLoad %v4float %_3_x
%322 = OpFOrdEqual %v4bool %320 %321
%323 = OpAll %bool %322
OpBranch %319
%319 = OpLabel
%324 = OpPhi %bool %false %303 %323 %318
OpStore %_0_ok %324
%325 = OpLoad %v4float %_3_x
%326 = OpVectorShuffle %v2float %325 %325 2 3
%329 = OpFDiv %float %float_1 %float_0_5
%330 = OpVectorTimesScalar %v2float %326 %329
%331 = OpLoad %v4float %_3_x
%332 = OpVectorShuffle %v4float %331 %330 4 5 2 3
OpStore %_3_x %332
%333 = OpLoad %bool %_0_ok
OpSelectionMerge %335 None
OpBranchConditional %333 %334 %335
%334 = OpLabel
%336 = OpLoad %v4float %_3_x
%340 = OpFOrdEqual %v4bool %336 %339
%341 = OpAll %bool %340
OpBranch %335
%335 = OpLabel
%342 = OpPhi %bool %false %319 %341 %334
OpStore %_0_ok %342
%343 = OpLoad %v4float %_1_inputRed
%345 = OpVectorTimesScalar %v4float %343 %float_5
%346 = OpVectorShuffle %v4float %345 %345 1 0 3 2
OpStore %_3_x %346
%347 = OpLoad %bool %_0_ok
OpSelectionMerge %349 None
OpBranchConditional %347 %348 %349
%348 = OpLabel
%350 = OpLoad %v4float %_3_x
%353 = OpFOrdEqual %v4bool %350 %352
%354 = OpAll %bool %353
OpBranch %349
%349 = OpLabel
%355 = OpPhi %bool %false %335 %354 %348
OpStore %_0_ok %355
%356 = OpLoad %v4float %_1_inputRed
%357 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%358 = OpFAdd %v4float %357 %356
OpStore %_3_x %358
%359 = OpLoad %bool %_0_ok
OpSelectionMerge %361 None
OpBranchConditional %359 %360 %361
%360 = OpLabel
%362 = OpLoad %v4float %_3_x
%363 = OpFOrdEqual %v4bool %362 %278
%364 = OpAll %bool %363
OpBranch %361
%361 = OpLabel
%365 = OpPhi %bool %false %349 %364 %360
OpStore %_0_ok %365
%367 = OpLoad %v4float %_2_inputGreen
%368 = OpVectorShuffle %v4float %367 %367 1 3 0 2
%369 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%370 = OpFSub %v4float %369 %368
OpStore %_3_x %370
%371 = OpLoad %bool %_0_ok
OpSelectionMerge %373 None
OpBranchConditional %371 %372 %373
%372 = OpLabel
%374 = OpLoad %v4float %_3_x
%376 = OpFOrdEqual %v4bool %374 %375
%377 = OpAll %bool %376
OpBranch %373
%373 = OpLabel
%378 = OpPhi %bool %false %361 %377 %372
OpStore %_0_ok %378
%379 = OpLoad %v4float %_1_inputRed
%380 = OpCompositeExtract %float %379 0
%381 = OpLoad %v4float %_2_inputGreen
%382 = OpCompositeConstruct %v4float %380 %380 %380 %380
%383 = OpFAdd %v4float %382 %381
OpStore %_3_x %383
%384 = OpLoad %bool %_0_ok
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpLoad %v4float %_3_x
%389 = OpFOrdEqual %v4bool %387 %388
%390 = OpAll %bool %389
OpBranch %386
%386 = OpLabel
%391 = OpPhi %bool %false %373 %390 %385
OpStore %_0_ok %391
%392 = OpLoad %v4float %_2_inputGreen
%393 = OpVectorShuffle %v3float %392 %392 3 1 3
%394 = OpVectorTimesScalar %v3float %393 %float_9
%395 = OpLoad %v4float %_3_x
%396 = OpVectorShuffle %v4float %395 %394 4 5 6 3
OpStore %_3_x %396
%397 = OpLoad %bool %_0_ok
OpSelectionMerge %399 None
OpBranchConditional %397 %398 %399
%398 = OpLabel
%400 = OpLoad %v4float %_3_x
%401 = OpFOrdEqual %v4bool %400 %321
%402 = OpAll %bool %401
OpBranch %399
%399 = OpLabel
%403 = OpPhi %bool %false %386 %402 %398
OpStore %_0_ok %403
%405 = OpLoad %v4float %_3_x
%406 = OpVectorShuffle %v2float %405 %405 2 3
%407 = OpCompositeConstruct %v2float %float_36 %float_36
%408 = OpFDiv %v2float %407 %406
%409 = OpLoad %v4float %_3_x
%410 = OpVectorShuffle %v4float %409 %408 4 5 2 3
OpStore %_3_x %410
%411 = OpLoad %bool %_0_ok
OpSelectionMerge %413 None
OpBranchConditional %411 %412 %413
%412 = OpLabel
%414 = OpLoad %v4float %_3_x
%416 = OpFOrdEqual %v4bool %414 %415
%417 = OpAll %bool %416
OpBranch %413
%413 = OpLabel
%418 = OpPhi %bool %false %399 %417 %412
OpStore %_0_ok %418
%419 = OpLoad %v4float %_3_x
%420 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%421 = OpFDiv %v4float %420 %419
%422 = OpVectorShuffle %v4float %421 %421 1 0 3 2
OpStore %_3_x %422
%423 = OpLoad %bool %_0_ok
OpSelectionMerge %425 None
OpBranchConditional %423 %424 %425
%424 = OpLabel
%426 = OpLoad %v4float %_3_x
%428 = OpFOrdEqual %v4bool %426 %427
%429 = OpAll %bool %428
OpBranch %425
%425 = OpLabel
%430 = OpPhi %bool %false %413 %429 %424
OpStore %_0_ok %430
%431 = OpLoad %v4float %_3_x
%432 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%433 = OpFAdd %v4float %431 %432
OpStore %_3_x %433
%434 = OpLoad %v4float %_3_x
%435 = OpVectorTimesScalar %v4float %434 %float_2
OpStore %_3_x %435
%436 = OpLoad %v4float %_3_x
%437 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%438 = OpFSub %v4float %436 %437
OpStore %_3_x %438
%439 = OpLoad %v4float %_3_x
%440 = OpFDiv %float %float_1 %float_2
%441 = OpVectorTimesScalar %v4float %439 %440
OpStore %_3_x %441
%442 = OpLoad %bool %_0_ok
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpLoad %v4float %_3_x
%446 = OpFOrdEqual %v4bool %445 %427
%447 = OpAll %bool %446
OpBranch %444
%444 = OpLabel
%448 = OpPhi %bool %false %425 %447 %443
OpStore %_0_ok %448
%449 = OpLoad %v4float %_3_x
%450 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%451 = OpFAdd %v4float %449 %450
OpStore %_3_x %451
%452 = OpLoad %v4float %_3_x
%453 = OpVectorTimesScalar %v4float %452 %float_2
OpStore %_3_x %453
%454 = OpLoad %v4float %_3_x
%455 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%456 = OpFSub %v4float %454 %455
OpStore %_3_x %456
%457 = OpLoad %v4float %_3_x
%458 = OpFDiv %float %float_1 %float_2
%459 = OpVectorTimesScalar %v4float %457 %458
OpStore %_3_x %459
%460 = OpLoad %bool %_0_ok
OpSelectionMerge %462 None
OpBranchConditional %460 %461 %462
%461 = OpLabel
%463 = OpLoad %v4float %_3_x
%464 = OpFOrdEqual %v4bool %463 %427
%465 = OpAll %bool %464
OpBranch %462
%462 = OpLabel
%466 = OpPhi %bool %false %444 %465 %461
OpStore %_0_ok %466
%467 = OpLoad %bool %_0_ok
OpSelectionMerge %469 None
OpBranchConditional %467 %468 %469
%468 = OpLabel
%470 = OpFunctionCall %bool %test_int_b
OpBranch %469
%469 = OpLabel
%471 = OpPhi %bool %false %462 %470 %468
OpSelectionMerge %475 None
OpBranchConditional %471 %473 %474
%473 = OpLabel
%476 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%477 = OpLoad %v4float %476
OpStore %472 %477
OpBranch %475
%474 = OpLabel
%478 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%479 = OpLoad %v4float %478
OpStore %472 %479
OpBranch %475
%475 = OpLabel
%480 = OpLoad %v4float %472
OpReturnValue %480
OpFunctionEnd
