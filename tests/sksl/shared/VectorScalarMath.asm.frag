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
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
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
OpDecorate %443 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
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
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%337 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%350 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%373 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%386 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%413 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%425 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%float_0_5 = OpConstant %float 0.5
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
%470 = OpVariable %_ptr_Function_v4float Function
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
%328 = OpVectorTimesScalar %v2float %326 %float_2
%329 = OpLoad %v4float %_3_x
%330 = OpVectorShuffle %v4float %329 %328 4 5 2 3
OpStore %_3_x %330
%331 = OpLoad %bool %_0_ok
OpSelectionMerge %333 None
OpBranchConditional %331 %332 %333
%332 = OpLabel
%334 = OpLoad %v4float %_3_x
%338 = OpFOrdEqual %v4bool %334 %337
%339 = OpAll %bool %338
OpBranch %333
%333 = OpLabel
%340 = OpPhi %bool %false %319 %339 %332
OpStore %_0_ok %340
%341 = OpLoad %v4float %_1_inputRed
%343 = OpVectorTimesScalar %v4float %341 %float_5
%344 = OpVectorShuffle %v4float %343 %343 1 0 3 2
OpStore %_3_x %344
%345 = OpLoad %bool %_0_ok
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpLoad %v4float %_3_x
%351 = OpFOrdEqual %v4bool %348 %350
%352 = OpAll %bool %351
OpBranch %347
%347 = OpLabel
%353 = OpPhi %bool %false %333 %352 %346
OpStore %_0_ok %353
%354 = OpLoad %v4float %_1_inputRed
%355 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%356 = OpFAdd %v4float %355 %354
OpStore %_3_x %356
%357 = OpLoad %bool %_0_ok
OpSelectionMerge %359 None
OpBranchConditional %357 %358 %359
%358 = OpLabel
%360 = OpLoad %v4float %_3_x
%361 = OpFOrdEqual %v4bool %360 %278
%362 = OpAll %bool %361
OpBranch %359
%359 = OpLabel
%363 = OpPhi %bool %false %347 %362 %358
OpStore %_0_ok %363
%365 = OpLoad %v4float %_2_inputGreen
%366 = OpVectorShuffle %v4float %365 %365 1 3 0 2
%367 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%368 = OpFSub %v4float %367 %366
OpStore %_3_x %368
%369 = OpLoad %bool %_0_ok
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%372 = OpLoad %v4float %_3_x
%374 = OpFOrdEqual %v4bool %372 %373
%375 = OpAll %bool %374
OpBranch %371
%371 = OpLabel
%376 = OpPhi %bool %false %359 %375 %370
OpStore %_0_ok %376
%377 = OpLoad %v4float %_1_inputRed
%378 = OpCompositeExtract %float %377 0
%379 = OpLoad %v4float %_2_inputGreen
%380 = OpCompositeConstruct %v4float %378 %378 %378 %378
%381 = OpFAdd %v4float %380 %379
OpStore %_3_x %381
%382 = OpLoad %bool %_0_ok
OpSelectionMerge %384 None
OpBranchConditional %382 %383 %384
%383 = OpLabel
%385 = OpLoad %v4float %_3_x
%387 = OpFOrdEqual %v4bool %385 %386
%388 = OpAll %bool %387
OpBranch %384
%384 = OpLabel
%389 = OpPhi %bool %false %371 %388 %383
OpStore %_0_ok %389
%390 = OpLoad %v4float %_2_inputGreen
%391 = OpVectorShuffle %v3float %390 %390 3 1 3
%392 = OpVectorTimesScalar %v3float %391 %float_9
%393 = OpLoad %v4float %_3_x
%394 = OpVectorShuffle %v4float %393 %392 4 5 6 3
OpStore %_3_x %394
%395 = OpLoad %bool %_0_ok
OpSelectionMerge %397 None
OpBranchConditional %395 %396 %397
%396 = OpLabel
%398 = OpLoad %v4float %_3_x
%399 = OpFOrdEqual %v4bool %398 %321
%400 = OpAll %bool %399
OpBranch %397
%397 = OpLabel
%401 = OpPhi %bool %false %384 %400 %396
OpStore %_0_ok %401
%403 = OpLoad %v4float %_3_x
%404 = OpVectorShuffle %v2float %403 %403 2 3
%405 = OpCompositeConstruct %v2float %float_36 %float_36
%406 = OpFDiv %v2float %405 %404
%407 = OpLoad %v4float %_3_x
%408 = OpVectorShuffle %v4float %407 %406 4 5 2 3
OpStore %_3_x %408
%409 = OpLoad %bool %_0_ok
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%412 = OpLoad %v4float %_3_x
%414 = OpFOrdEqual %v4bool %412 %413
%415 = OpAll %bool %414
OpBranch %411
%411 = OpLabel
%416 = OpPhi %bool %false %397 %415 %410
OpStore %_0_ok %416
%417 = OpLoad %v4float %_3_x
%418 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%419 = OpFDiv %v4float %418 %417
%420 = OpVectorShuffle %v4float %419 %419 1 0 3 2
OpStore %_3_x %420
%421 = OpLoad %bool %_0_ok
OpSelectionMerge %423 None
OpBranchConditional %421 %422 %423
%422 = OpLabel
%424 = OpLoad %v4float %_3_x
%426 = OpFOrdEqual %v4bool %424 %425
%427 = OpAll %bool %426
OpBranch %423
%423 = OpLabel
%428 = OpPhi %bool %false %411 %427 %422
OpStore %_0_ok %428
%429 = OpLoad %v4float %_3_x
%430 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%431 = OpFAdd %v4float %429 %430
OpStore %_3_x %431
%432 = OpLoad %v4float %_3_x
%433 = OpVectorTimesScalar %v4float %432 %float_2
OpStore %_3_x %433
%434 = OpLoad %v4float %_3_x
%435 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%436 = OpFSub %v4float %434 %435
OpStore %_3_x %436
%437 = OpLoad %v4float %_3_x
%438 = OpFDiv %float %float_1 %float_2
%439 = OpVectorTimesScalar %v4float %437 %438
OpStore %_3_x %439
%440 = OpLoad %bool %_0_ok
OpSelectionMerge %442 None
OpBranchConditional %440 %441 %442
%441 = OpLabel
%443 = OpLoad %v4float %_3_x
%444 = OpFOrdEqual %v4bool %443 %425
%445 = OpAll %bool %444
OpBranch %442
%442 = OpLabel
%446 = OpPhi %bool %false %423 %445 %441
OpStore %_0_ok %446
%447 = OpLoad %v4float %_3_x
%448 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%449 = OpFAdd %v4float %447 %448
OpStore %_3_x %449
%450 = OpLoad %v4float %_3_x
%451 = OpVectorTimesScalar %v4float %450 %float_2
OpStore %_3_x %451
%452 = OpLoad %v4float %_3_x
%453 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%454 = OpFSub %v4float %452 %453
OpStore %_3_x %454
%455 = OpLoad %v4float %_3_x
%457 = OpVectorTimesScalar %v4float %455 %float_0_5
OpStore %_3_x %457
%458 = OpLoad %bool %_0_ok
OpSelectionMerge %460 None
OpBranchConditional %458 %459 %460
%459 = OpLabel
%461 = OpLoad %v4float %_3_x
%462 = OpFOrdEqual %v4bool %461 %425
%463 = OpAll %bool %462
OpBranch %460
%460 = OpLabel
%464 = OpPhi %bool %false %442 %463 %459
OpStore %_0_ok %464
%465 = OpLoad %bool %_0_ok
OpSelectionMerge %467 None
OpBranchConditional %465 %466 %467
%466 = OpLabel
%468 = OpFunctionCall %bool %test_int_b
OpBranch %467
%467 = OpLabel
%469 = OpPhi %bool %false %460 %468 %466
OpSelectionMerge %473 None
OpBranchConditional %469 %471 %472
%471 = OpLabel
%474 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%475 = OpLoad %v4float %474
OpStore %470 %475
OpBranch %473
%472 = OpLabel
%476 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%477 = OpLoad %v4float %476
OpStore %470 %477
OpBranch %473
%473 = OpLabel
%478 = OpLoad %v4float %470
OpReturnValue %478
OpFunctionEnd
