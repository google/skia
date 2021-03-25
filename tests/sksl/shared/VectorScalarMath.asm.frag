### Compilation failed:

error: SPIR-V validation error: Expected float vector type as Result Type: VectorTimesScalar
  %101 = OpVectorTimesScalar %v3int %98 %int_9

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
OpDecorate %104 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
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
%108 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%123 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%135 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%158 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%171 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%200 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%212 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%253 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%273 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%287 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%301 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%316 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%v2float = OpTypeVector %float 2
%float_0_5 = OpConstant %float 0.5
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%334 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%347 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%370 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%383 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%410 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%422 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
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
%101 = OpVectorTimesScalar %v3int %98 %int_9
%102 = OpLoad %v4int %x
%103 = OpVectorShuffle %v4int %102 %101 4 5 6 3
OpStore %x %103
%104 = OpLoad %bool %ok
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %v4int %x
%109 = OpIEqual %v4bool %107 %108
%110 = OpAll %bool %109
OpBranch %106
%106 = OpLabel
%111 = OpPhi %bool %false %91 %110 %105
OpStore %ok %111
%112 = OpLoad %v4int %x
%113 = OpVectorShuffle %v2int %112 %112 2 3
%115 = OpFDiv %int %int_1 %int_3
%116 = OpVectorTimesScalar %v2int %113 %115
%117 = OpLoad %v4int %x
%118 = OpVectorShuffle %v4int %117 %116 4 5 2 3
OpStore %x %118
%119 = OpLoad %bool %ok
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %v4int %x
%124 = OpIEqual %v4bool %122 %123
%125 = OpAll %bool %124
OpBranch %121
%121 = OpLabel
%126 = OpPhi %bool %false %106 %125 %120
OpStore %ok %126
%127 = OpLoad %v4int %inputRed
%129 = OpVectorTimesScalar %v4int %127 %int_5
%130 = OpVectorShuffle %v4int %129 %129 1 0 3 2
OpStore %x %130
%131 = OpLoad %bool %ok
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpLoad %v4int %x
%136 = OpIEqual %v4bool %134 %135
%137 = OpAll %bool %136
OpBranch %133
%133 = OpLabel
%138 = OpPhi %bool %false %121 %137 %132
OpStore %ok %138
%139 = OpLoad %v4int %inputRed
%140 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%141 = OpIAdd %v4int %140 %139
OpStore %x %141
%142 = OpLoad %bool %ok
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpLoad %v4int %x
%146 = OpIEqual %v4bool %145 %65
%147 = OpAll %bool %146
OpBranch %144
%144 = OpLabel
%148 = OpPhi %bool %false %133 %147 %143
OpStore %ok %148
%150 = OpLoad %v4int %inputGreen
%151 = OpVectorShuffle %v4int %150 %150 1 3 0 2
%152 = OpCompositeConstruct %v4int %int_10 %int_10 %int_10 %int_10
%153 = OpISub %v4int %152 %151
OpStore %x %153
%154 = OpLoad %bool %ok
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %v4int %x
%159 = OpIEqual %v4bool %157 %158
%160 = OpAll %bool %159
OpBranch %156
%156 = OpLabel
%161 = OpPhi %bool %false %144 %160 %155
OpStore %ok %161
%162 = OpLoad %v4int %inputRed
%163 = OpCompositeExtract %int %162 0
%164 = OpLoad %v4int %inputGreen
%165 = OpCompositeConstruct %v4int %163 %163 %163 %163
%166 = OpIAdd %v4int %165 %164
OpStore %x %166
%167 = OpLoad %bool %ok
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpLoad %v4int %x
%172 = OpIEqual %v4bool %170 %171
%173 = OpAll %bool %172
OpBranch %169
%169 = OpLabel
%174 = OpPhi %bool %false %156 %173 %168
OpStore %ok %174
%175 = OpLoad %v4int %inputGreen
%176 = OpVectorShuffle %v3int %175 %175 3 1 3
%177 = OpVectorTimesScalar %v3int %176 %int_9
%178 = OpLoad %v4int %x
%179 = OpVectorShuffle %v4int %178 %177 4 5 6 3
OpStore %x %179
%180 = OpLoad %bool %ok
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpLoad %v4int %x
%184 = OpIEqual %v4bool %183 %108
%185 = OpAll %bool %184
OpBranch %182
%182 = OpLabel
%186 = OpPhi %bool %false %169 %185 %181
OpStore %ok %186
%188 = OpLoad %v4int %x
%189 = OpVectorShuffle %v2int %188 %188 2 3
%190 = OpCompositeConstruct %v2int %int_36 %int_36
%191 = OpSDiv %v2int %190 %189
%192 = OpLoad %v4int %x
%193 = OpVectorShuffle %v4int %192 %191 4 5 2 3
OpStore %x %193
%194 = OpLoad %bool %ok
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %v4int %x
%201 = OpIEqual %v4bool %197 %200
%202 = OpAll %bool %201
OpBranch %196
%196 = OpLabel
%203 = OpPhi %bool %false %182 %202 %195
OpStore %ok %203
%204 = OpLoad %v4int %x
%205 = OpCompositeConstruct %v4int %int_36 %int_36 %int_36 %int_36
%206 = OpSDiv %v4int %205 %204
%207 = OpVectorShuffle %v4int %206 %206 1 0 3 2
OpStore %x %207
%208 = OpLoad %bool %ok
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpLoad %v4int %x
%213 = OpIEqual %v4bool %211 %212
%214 = OpAll %bool %213
OpBranch %210
%210 = OpLabel
%215 = OpPhi %bool %false %196 %214 %209
OpStore %ok %215
%216 = OpLoad %v4int %x
%217 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%218 = OpIAdd %v4int %216 %217
OpStore %x %218
%219 = OpLoad %v4int %x
%220 = OpVectorTimesScalar %v4int %219 %int_2
OpStore %x %220
%221 = OpLoad %v4int %x
%222 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%223 = OpISub %v4int %221 %222
OpStore %x %223
%224 = OpLoad %v4int %x
%225 = OpFDiv %int %int_1 %int_2
%226 = OpVectorTimesScalar %v4int %224 %225
OpStore %x %226
%227 = OpLoad %bool %ok
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpLoad %v4int %x
%231 = OpIEqual %v4bool %230 %212
%232 = OpAll %bool %231
OpBranch %229
%229 = OpLabel
%233 = OpPhi %bool %false %210 %232 %228
OpStore %ok %233
%234 = OpLoad %v4int %x
%235 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%236 = OpIAdd %v4int %234 %235
OpStore %x %236
%237 = OpLoad %v4int %x
%238 = OpVectorTimesScalar %v4int %237 %int_2
OpStore %x %238
%239 = OpLoad %v4int %x
%240 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%241 = OpISub %v4int %239 %240
OpStore %x %241
%242 = OpLoad %v4int %x
%243 = OpFDiv %int %int_1 %int_2
%244 = OpVectorTimesScalar %v4int %242 %243
OpStore %x %244
%245 = OpLoad %bool %ok
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %v4int %x
%249 = OpIEqual %v4bool %248 %212
%250 = OpAll %bool %249
OpBranch %247
%247 = OpLabel
%251 = OpPhi %bool %false %229 %250 %246
OpStore %ok %251
%252 = OpLoad %bool %ok
OpReturnValue %252
OpFunctionEnd
%main = OpFunction %v4float None %253
%254 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%467 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%258 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%259 = OpLoad %v4float %258
OpStore %_1_inputRed %259
%261 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%262 = OpLoad %v4float %261
OpStore %_2_inputGreen %262
%264 = OpLoad %v4float %_1_inputRed
%266 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%267 = OpFAdd %v4float %264 %266
OpStore %_3_x %267
%268 = OpLoad %bool %_0_ok
OpSelectionMerge %270 None
OpBranchConditional %268 %269 %270
%269 = OpLabel
%271 = OpLoad %v4float %_3_x
%274 = OpFOrdEqual %v4bool %271 %273
%275 = OpAll %bool %274
OpBranch %270
%270 = OpLabel
%276 = OpPhi %bool %false %254 %275 %269
OpStore %_0_ok %276
%277 = OpLoad %v4float %_2_inputGreen
%278 = OpVectorShuffle %v4float %277 %277 1 3 0 2
%279 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%280 = OpFSub %v4float %278 %279
OpStore %_3_x %280
%281 = OpLoad %bool %_0_ok
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpLoad %v4float %_3_x
%288 = OpFOrdEqual %v4bool %284 %287
%289 = OpAll %bool %288
OpBranch %283
%283 = OpLabel
%290 = OpPhi %bool %false %270 %289 %282
OpStore %_0_ok %290
%291 = OpLoad %v4float %_1_inputRed
%292 = OpLoad %v4float %_2_inputGreen
%293 = OpCompositeExtract %float %292 1
%294 = OpCompositeConstruct %v4float %293 %293 %293 %293
%295 = OpFAdd %v4float %291 %294
OpStore %_3_x %295
%296 = OpLoad %bool %_0_ok
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %v4float %_3_x
%302 = OpFOrdEqual %v4bool %299 %301
%303 = OpAll %bool %302
OpBranch %298
%298 = OpLabel
%304 = OpPhi %bool %false %283 %303 %297
OpStore %_0_ok %304
%305 = OpLoad %v4float %_2_inputGreen
%306 = OpVectorShuffle %v3float %305 %305 3 1 3
%309 = OpVectorTimesScalar %v3float %306 %float_9
%310 = OpLoad %v4float %_3_x
%311 = OpVectorShuffle %v4float %310 %309 4 5 6 3
OpStore %_3_x %311
%312 = OpLoad %bool %_0_ok
OpSelectionMerge %314 None
OpBranchConditional %312 %313 %314
%313 = OpLabel
%315 = OpLoad %v4float %_3_x
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
OpBranch %314
%314 = OpLabel
%319 = OpPhi %bool %false %298 %318 %313
OpStore %_0_ok %319
%320 = OpLoad %v4float %_3_x
%321 = OpVectorShuffle %v2float %320 %320 2 3
%324 = OpFDiv %float %float_1 %float_0_5
%325 = OpVectorTimesScalar %v2float %321 %324
%326 = OpLoad %v4float %_3_x
%327 = OpVectorShuffle %v4float %326 %325 4 5 2 3
OpStore %_3_x %327
%328 = OpLoad %bool %_0_ok
OpSelectionMerge %330 None
OpBranchConditional %328 %329 %330
%329 = OpLabel
%331 = OpLoad %v4float %_3_x
%335 = OpFOrdEqual %v4bool %331 %334
%336 = OpAll %bool %335
OpBranch %330
%330 = OpLabel
%337 = OpPhi %bool %false %314 %336 %329
OpStore %_0_ok %337
%338 = OpLoad %v4float %_1_inputRed
%340 = OpVectorTimesScalar %v4float %338 %float_5
%341 = OpVectorShuffle %v4float %340 %340 1 0 3 2
OpStore %_3_x %341
%342 = OpLoad %bool %_0_ok
OpSelectionMerge %344 None
OpBranchConditional %342 %343 %344
%343 = OpLabel
%345 = OpLoad %v4float %_3_x
%348 = OpFOrdEqual %v4bool %345 %347
%349 = OpAll %bool %348
OpBranch %344
%344 = OpLabel
%350 = OpPhi %bool %false %330 %349 %343
OpStore %_0_ok %350
%351 = OpLoad %v4float %_1_inputRed
%352 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%353 = OpFAdd %v4float %352 %351
OpStore %_3_x %353
%354 = OpLoad %bool %_0_ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%357 = OpLoad %v4float %_3_x
%358 = OpFOrdEqual %v4bool %357 %273
%359 = OpAll %bool %358
OpBranch %356
%356 = OpLabel
%360 = OpPhi %bool %false %344 %359 %355
OpStore %_0_ok %360
%362 = OpLoad %v4float %_2_inputGreen
%363 = OpVectorShuffle %v4float %362 %362 1 3 0 2
%364 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%365 = OpFSub %v4float %364 %363
OpStore %_3_x %365
%366 = OpLoad %bool %_0_ok
OpSelectionMerge %368 None
OpBranchConditional %366 %367 %368
%367 = OpLabel
%369 = OpLoad %v4float %_3_x
%371 = OpFOrdEqual %v4bool %369 %370
%372 = OpAll %bool %371
OpBranch %368
%368 = OpLabel
%373 = OpPhi %bool %false %356 %372 %367
OpStore %_0_ok %373
%374 = OpLoad %v4float %_1_inputRed
%375 = OpCompositeExtract %float %374 0
%376 = OpLoad %v4float %_2_inputGreen
%377 = OpCompositeConstruct %v4float %375 %375 %375 %375
%378 = OpFAdd %v4float %377 %376
OpStore %_3_x %378
%379 = OpLoad %bool %_0_ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %v4float %_3_x
%384 = OpFOrdEqual %v4bool %382 %383
%385 = OpAll %bool %384
OpBranch %381
%381 = OpLabel
%386 = OpPhi %bool %false %368 %385 %380
OpStore %_0_ok %386
%387 = OpLoad %v4float %_2_inputGreen
%388 = OpVectorShuffle %v3float %387 %387 3 1 3
%389 = OpVectorTimesScalar %v3float %388 %float_9
%390 = OpLoad %v4float %_3_x
%391 = OpVectorShuffle %v4float %390 %389 4 5 6 3
OpStore %_3_x %391
%392 = OpLoad %bool %_0_ok
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpLoad %v4float %_3_x
%396 = OpFOrdEqual %v4bool %395 %316
%397 = OpAll %bool %396
OpBranch %394
%394 = OpLabel
%398 = OpPhi %bool %false %381 %397 %393
OpStore %_0_ok %398
%400 = OpLoad %v4float %_3_x
%401 = OpVectorShuffle %v2float %400 %400 2 3
%402 = OpCompositeConstruct %v2float %float_36 %float_36
%403 = OpFDiv %v2float %402 %401
%404 = OpLoad %v4float %_3_x
%405 = OpVectorShuffle %v4float %404 %403 4 5 2 3
OpStore %_3_x %405
%406 = OpLoad %bool %_0_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpLoad %v4float %_3_x
%411 = OpFOrdEqual %v4bool %409 %410
%412 = OpAll %bool %411
OpBranch %408
%408 = OpLabel
%413 = OpPhi %bool %false %394 %412 %407
OpStore %_0_ok %413
%414 = OpLoad %v4float %_3_x
%415 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%416 = OpFDiv %v4float %415 %414
%417 = OpVectorShuffle %v4float %416 %416 1 0 3 2
OpStore %_3_x %417
%418 = OpLoad %bool %_0_ok
OpSelectionMerge %420 None
OpBranchConditional %418 %419 %420
%419 = OpLabel
%421 = OpLoad %v4float %_3_x
%423 = OpFOrdEqual %v4bool %421 %422
%424 = OpAll %bool %423
OpBranch %420
%420 = OpLabel
%425 = OpPhi %bool %false %408 %424 %419
OpStore %_0_ok %425
%426 = OpLoad %v4float %_3_x
%427 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%428 = OpFAdd %v4float %426 %427
OpStore %_3_x %428
%429 = OpLoad %v4float %_3_x
%430 = OpVectorTimesScalar %v4float %429 %float_2
OpStore %_3_x %430
%431 = OpLoad %v4float %_3_x
%432 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%433 = OpFSub %v4float %431 %432
OpStore %_3_x %433
%434 = OpLoad %v4float %_3_x
%435 = OpFDiv %float %float_1 %float_2
%436 = OpVectorTimesScalar %v4float %434 %435
OpStore %_3_x %436
%437 = OpLoad %bool %_0_ok
OpSelectionMerge %439 None
OpBranchConditional %437 %438 %439
%438 = OpLabel
%440 = OpLoad %v4float %_3_x
%441 = OpFOrdEqual %v4bool %440 %422
%442 = OpAll %bool %441
OpBranch %439
%439 = OpLabel
%443 = OpPhi %bool %false %420 %442 %438
OpStore %_0_ok %443
%444 = OpLoad %v4float %_3_x
%445 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%446 = OpFAdd %v4float %444 %445
OpStore %_3_x %446
%447 = OpLoad %v4float %_3_x
%448 = OpVectorTimesScalar %v4float %447 %float_2
OpStore %_3_x %448
%449 = OpLoad %v4float %_3_x
%450 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%451 = OpFSub %v4float %449 %450
OpStore %_3_x %451
%452 = OpLoad %v4float %_3_x
%453 = OpFDiv %float %float_1 %float_2
%454 = OpVectorTimesScalar %v4float %452 %453
OpStore %_3_x %454
%455 = OpLoad %bool %_0_ok
OpSelectionMerge %457 None
OpBranchConditional %455 %456 %457
%456 = OpLabel
%458 = OpLoad %v4float %_3_x
%459 = OpFOrdEqual %v4bool %458 %422
%460 = OpAll %bool %459
OpBranch %457
%457 = OpLabel
%461 = OpPhi %bool %false %439 %460 %456
OpStore %_0_ok %461
%462 = OpLoad %bool %_0_ok
OpSelectionMerge %464 None
OpBranchConditional %462 %463 %464
%463 = OpLabel
%465 = OpFunctionCall %bool %test_int_b
OpBranch %464
%464 = OpLabel
%466 = OpPhi %bool %false %457 %465 %463
OpSelectionMerge %470 None
OpBranchConditional %466 %468 %469
%468 = OpLabel
%471 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%472 = OpLoad %v4float %471
OpStore %467 %472
OpBranch %470
%469 = OpLabel
%473 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%474 = OpLoad %v4float %473
OpStore %467 %474
OpBranch %470
%470 = OpLabel
%475 = OpLoad %v4float %467
OpReturnValue %475
OpFunctionEnd

1 error
