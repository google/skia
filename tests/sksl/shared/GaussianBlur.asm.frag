OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %vLocalCoord_Stage0
OpExecutionMode %main OriginUpperLeft
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "sk_RTAdjust"
OpMemberName %uniformBuffer 1 "uIncrement_Stage1_c0"
OpMemberName %uniformBuffer 2 "uKernel_Stage1_c0"
OpMemberName %uniformBuffer 3 "umatrix_Stage1_c0_c0"
OpMemberName %uniformBuffer 4 "uborder_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 5 "usubset_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 6 "unorm_Stage1_c0_c0_c0"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %uTextureSampler_0_Stage1 "uTextureSampler_0_Stage1"
OpName %vLocalCoord_Stage0 "vLocalCoord_Stage0"
OpName %TextureEffect_Stage1_c0_c0_c0 "TextureEffect_Stage1_c0_c0_c0"
OpName %_output "_output"
OpName %inCoord "inCoord"
OpName %subsetCoord "subsetCoord"
OpName %clampedCoord "clampedCoord"
OpName %textureColor "textureColor"
OpName %snappedX "snappedX"
OpName %MatrixEffect_Stage1_c0_c0 "MatrixEffect_Stage1_c0_c0"
OpName %_output_0 "_output"
OpName %GaussianConvolution_Stage1_c0 "GaussianConvolution_Stage1_c0"
OpName %_output_1 "_output"
OpName %coord "coord"
OpName %coordSampled "coordSampled"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpDecorate %_arr_v4float_int_7 ArrayStride 16
OpMemberDecorate %uniformBuffer 0 Offset 0
OpMemberDecorate %uniformBuffer 1 Offset 16
OpMemberDecorate %uniformBuffer 1 RelaxedPrecision
OpMemberDecorate %uniformBuffer 2 Offset 32
OpMemberDecorate %uniformBuffer 2 RelaxedPrecision
OpMemberDecorate %uniformBuffer 3 Offset 144
OpMemberDecorate %uniformBuffer 3 ColMajor
OpMemberDecorate %uniformBuffer 3 MatrixStride 16
OpMemberDecorate %uniformBuffer 4 Offset 192
OpMemberDecorate %uniformBuffer 4 RelaxedPrecision
OpMemberDecorate %uniformBuffer 5 Offset 208
OpMemberDecorate %uniformBuffer 6 Offset 224
OpDecorate %uniformBuffer Block
OpDecorate %6 Binding 0
OpDecorate %6 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %58 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %542 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_7 = OpConstant %int 7
%_arr_v4float_int_7 = OpTypeArray %v4float %int_7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%uniformBuffer = OpTypeStruct %v4float %v2float %_arr_v4float_int_7 %mat3v3float %v4float %v4float %v4float
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
%6 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%25 = OpTypeImage %float 2D 0 0 0 1 Unknown
%24 = OpTypeSampledImage %25
%_ptr_UniformConstant_24 = OpTypePointer UniformConstant %24
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_24 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0_5 = OpConstant %float 0.5
%true = OpConstantTrue %bool
%int_5 = OpConstant %int 5
%int_4 = OpConstant %int 4
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%113 = OpTypeFunction %v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%118 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%128 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%void = OpTypeVoid
%535 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%540 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%TextureEffect_Stage1_c0_c0_c0 = OpFunction %v4float None %28
%31 = OpFunctionParameter %_ptr_Function_v4float
%32 = OpFunctionParameter %_ptr_Function_v2float
%33 = OpLabel
%_output = OpVariable %_ptr_Function_v4float Function
%inCoord = OpVariable %_ptr_Function_v2float Function
%subsetCoord = OpVariable %_ptr_Function_v2float Function
%clampedCoord = OpVariable %_ptr_Function_v2float Function
%textureColor = OpVariable %_ptr_Function_v4float Function
%snappedX = OpVariable %_ptr_Function_float Function
%36 = OpLoad %v2float %32
OpStore %inCoord %36
%37 = OpLoad %v2float %inCoord
%39 = OpAccessChain %_ptr_Uniform_v4float %6 %int_6
%41 = OpLoad %v4float %39
%42 = OpVectorShuffle %v2float %41 %41 0 1
%43 = OpFMul %v2float %37 %42
OpStore %inCoord %43
%45 = OpLoad %v2float %inCoord
%46 = OpCompositeExtract %float %45 0
%47 = OpAccessChain %_ptr_Function_float %subsetCoord %int_0
OpStore %47 %46
%50 = OpLoad %v2float %inCoord
%51 = OpCompositeExtract %float %50 1
%52 = OpAccessChain %_ptr_Function_float %subsetCoord %int_1
OpStore %52 %51
%55 = OpLoad %v2float %subsetCoord
OpStore %clampedCoord %55
%58 = OpLoad %24 %uTextureSampler_0_Stage1
%59 = OpLoad %v2float %clampedCoord
%60 = OpAccessChain %_ptr_Uniform_v4float %6 %int_6
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v2float %61 %61 2 3
%63 = OpFMul %v2float %59 %62
%57 = OpImageSampleImplicitLod %v4float %58 %63
OpStore %textureColor %57
%66 = OpLoad %v2float %inCoord
%67 = OpCompositeExtract %float %66 0
%69 = OpFAdd %float %67 %float_0_00100000005
%65 = OpExtInst %float %1 Floor %69
%71 = OpFAdd %float %65 %float_0_5
OpStore %snappedX %71
%73 = OpLoad %float %snappedX
%75 = OpAccessChain %_ptr_Uniform_v4float %6 %int_5
%76 = OpLoad %v4float %75
%77 = OpCompositeExtract %float %76 0
%78 = OpFOrdLessThan %bool %73 %77
OpSelectionMerge %80 None
OpBranchConditional %78 %80 %79
%79 = OpLabel
%81 = OpLoad %float %snappedX
%82 = OpAccessChain %_ptr_Uniform_v4float %6 %int_5
%83 = OpLoad %v4float %82
%84 = OpCompositeExtract %float %83 2
%85 = OpFOrdGreaterThan %bool %81 %84
OpBranch %80
%80 = OpLabel
%86 = OpPhi %bool %true %33 %85 %79
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %6 %int_4
%91 = OpLoad %v4float %90
OpStore %textureColor %91
OpBranch %88
%88 = OpLabel
%92 = OpLoad %v4float %textureColor
OpReturnValue %92
OpFunctionEnd
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %28
%93 = OpFunctionParameter %_ptr_Function_v4float
%94 = OpFunctionParameter %_ptr_Function_v2float
%95 = OpLabel
%_output_0 = OpVariable %_ptr_Function_v4float Function
%98 = OpVariable %_ptr_Function_v4float Function
%111 = OpVariable %_ptr_Function_v2float Function
%97 = OpLoad %v4float %93
OpStore %98 %97
%100 = OpAccessChain %_ptr_Uniform_mat3v3float %6 %int_3
%102 = OpLoad %mat3v3float %100
%103 = OpLoad %v2float %94
%104 = OpVectorShuffle %v2float %103 %103 0 1
%105 = OpCompositeExtract %float %104 0
%106 = OpCompositeExtract %float %104 1
%107 = OpConvertSToF %float %int_1
%108 = OpCompositeConstruct %v3float %105 %106 %107
%109 = OpMatrixTimesVector %v3float %102 %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
OpStore %111 %110
%112 = OpFunctionCall %v4float %TextureEffect_Stage1_c0_c0_c0 %98 %111
OpReturnValue %112
OpFunctionEnd
%GaussianConvolution_Stage1_c0 = OpFunction %v4float None %113
%114 = OpFunctionParameter %_ptr_Function_v4float
%115 = OpLabel
%_output_1 = OpVariable %_ptr_Function_v4float Function
%coord = OpVariable %_ptr_Function_v2float Function
%coordSampled = OpVariable %_ptr_Function_v2float Function
%132 = OpVariable %_ptr_Function_v4float Function
%134 = OpVariable %_ptr_Function_v2float Function
%149 = OpVariable %_ptr_Function_v4float Function
%151 = OpVariable %_ptr_Function_v2float Function
%165 = OpVariable %_ptr_Function_v4float Function
%167 = OpVariable %_ptr_Function_v2float Function
%181 = OpVariable %_ptr_Function_v4float Function
%183 = OpVariable %_ptr_Function_v2float Function
%197 = OpVariable %_ptr_Function_v4float Function
%199 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v4float Function
%215 = OpVariable %_ptr_Function_v2float Function
%229 = OpVariable %_ptr_Function_v4float Function
%231 = OpVariable %_ptr_Function_v2float Function
%245 = OpVariable %_ptr_Function_v4float Function
%247 = OpVariable %_ptr_Function_v2float Function
%261 = OpVariable %_ptr_Function_v4float Function
%263 = OpVariable %_ptr_Function_v2float Function
%277 = OpVariable %_ptr_Function_v4float Function
%279 = OpVariable %_ptr_Function_v2float Function
%293 = OpVariable %_ptr_Function_v4float Function
%295 = OpVariable %_ptr_Function_v2float Function
%309 = OpVariable %_ptr_Function_v4float Function
%311 = OpVariable %_ptr_Function_v2float Function
%325 = OpVariable %_ptr_Function_v4float Function
%327 = OpVariable %_ptr_Function_v2float Function
%341 = OpVariable %_ptr_Function_v4float Function
%343 = OpVariable %_ptr_Function_v2float Function
%357 = OpVariable %_ptr_Function_v4float Function
%359 = OpVariable %_ptr_Function_v2float Function
%373 = OpVariable %_ptr_Function_v4float Function
%375 = OpVariable %_ptr_Function_v2float Function
%389 = OpVariable %_ptr_Function_v4float Function
%391 = OpVariable %_ptr_Function_v2float Function
%405 = OpVariable %_ptr_Function_v4float Function
%407 = OpVariable %_ptr_Function_v2float Function
%421 = OpVariable %_ptr_Function_v4float Function
%423 = OpVariable %_ptr_Function_v2float Function
%437 = OpVariable %_ptr_Function_v4float Function
%439 = OpVariable %_ptr_Function_v2float Function
%453 = OpVariable %_ptr_Function_v4float Function
%455 = OpVariable %_ptr_Function_v2float Function
%469 = OpVariable %_ptr_Function_v4float Function
%471 = OpVariable %_ptr_Function_v2float Function
%485 = OpVariable %_ptr_Function_v4float Function
%487 = OpVariable %_ptr_Function_v2float Function
%501 = OpVariable %_ptr_Function_v4float Function
%503 = OpVariable %_ptr_Function_v2float Function
%517 = OpVariable %_ptr_Function_v4float Function
%519 = OpVariable %_ptr_Function_v2float Function
OpStore %_output_1 %118
%120 = OpLoad %v2float %vLocalCoord_Stage0
%122 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%124 = OpLoad %v2float %122
%125 = OpVectorTimesScalar %v2float %124 %float_12
%126 = OpFSub %v2float %120 %125
OpStore %coord %126
OpStore %coordSampled %128
%129 = OpLoad %v2float %coord
OpStore %coordSampled %129
%130 = OpLoad %v4float %_output_1
%131 = OpLoad %v4float %114
OpStore %132 %131
%133 = OpLoad %v2float %coordSampled
OpStore %134 %133
%135 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %132 %134
%137 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 0
%140 = OpVectorTimesScalar %v4float %135 %139
%141 = OpFAdd %v4float %130 %140
OpStore %_output_1 %141
%142 = OpLoad %v2float %coord
%143 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%144 = OpLoad %v2float %143
%145 = OpFAdd %v2float %142 %144
OpStore %coord %145
%146 = OpLoad %v2float %coord
OpStore %coordSampled %146
%147 = OpLoad %v4float %_output_1
%148 = OpLoad %v4float %114
OpStore %149 %148
%150 = OpLoad %v2float %coordSampled
OpStore %151 %150
%152 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %149 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 1
%156 = OpVectorTimesScalar %v4float %152 %155
%157 = OpFAdd %v4float %147 %156
OpStore %_output_1 %157
%158 = OpLoad %v2float %coord
%159 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%160 = OpLoad %v2float %159
%161 = OpFAdd %v2float %158 %160
OpStore %coord %161
%162 = OpLoad %v2float %coord
OpStore %coordSampled %162
%163 = OpLoad %v4float %_output_1
%164 = OpLoad %v4float %114
OpStore %165 %164
%166 = OpLoad %v2float %coordSampled
OpStore %167 %166
%168 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %165 %167
%169 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 2
%172 = OpVectorTimesScalar %v4float %168 %171
%173 = OpFAdd %v4float %163 %172
OpStore %_output_1 %173
%174 = OpLoad %v2float %coord
%175 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%176 = OpLoad %v2float %175
%177 = OpFAdd %v2float %174 %176
OpStore %coord %177
%178 = OpLoad %v2float %coord
OpStore %coordSampled %178
%179 = OpLoad %v4float %_output_1
%180 = OpLoad %v4float %114
OpStore %181 %180
%182 = OpLoad %v2float %coordSampled
OpStore %183 %182
%184 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %181 %183
%185 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%186 = OpLoad %v4float %185
%187 = OpCompositeExtract %float %186 3
%188 = OpVectorTimesScalar %v4float %184 %187
%189 = OpFAdd %v4float %179 %188
OpStore %_output_1 %189
%190 = OpLoad %v2float %coord
%191 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%192 = OpLoad %v2float %191
%193 = OpFAdd %v2float %190 %192
OpStore %coord %193
%194 = OpLoad %v2float %coord
OpStore %coordSampled %194
%195 = OpLoad %v4float %_output_1
%196 = OpLoad %v4float %114
OpStore %197 %196
%198 = OpLoad %v2float %coordSampled
OpStore %199 %198
%200 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %197 %199
%201 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%202 = OpLoad %v4float %201
%203 = OpCompositeExtract %float %202 0
%204 = OpVectorTimesScalar %v4float %200 %203
%205 = OpFAdd %v4float %195 %204
OpStore %_output_1 %205
%206 = OpLoad %v2float %coord
%207 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%208 = OpLoad %v2float %207
%209 = OpFAdd %v2float %206 %208
OpStore %coord %209
%210 = OpLoad %v2float %coord
OpStore %coordSampled %210
%211 = OpLoad %v4float %_output_1
%212 = OpLoad %v4float %114
OpStore %213 %212
%214 = OpLoad %v2float %coordSampled
OpStore %215 %214
%216 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %213 %215
%217 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%218 = OpLoad %v4float %217
%219 = OpCompositeExtract %float %218 1
%220 = OpVectorTimesScalar %v4float %216 %219
%221 = OpFAdd %v4float %211 %220
OpStore %_output_1 %221
%222 = OpLoad %v2float %coord
%223 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%224 = OpLoad %v2float %223
%225 = OpFAdd %v2float %222 %224
OpStore %coord %225
%226 = OpLoad %v2float %coord
OpStore %coordSampled %226
%227 = OpLoad %v4float %_output_1
%228 = OpLoad %v4float %114
OpStore %229 %228
%230 = OpLoad %v2float %coordSampled
OpStore %231 %230
%232 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %229 %231
%233 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%234 = OpLoad %v4float %233
%235 = OpCompositeExtract %float %234 2
%236 = OpVectorTimesScalar %v4float %232 %235
%237 = OpFAdd %v4float %227 %236
OpStore %_output_1 %237
%238 = OpLoad %v2float %coord
%239 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%240 = OpLoad %v2float %239
%241 = OpFAdd %v2float %238 %240
OpStore %coord %241
%242 = OpLoad %v2float %coord
OpStore %coordSampled %242
%243 = OpLoad %v4float %_output_1
%244 = OpLoad %v4float %114
OpStore %245 %244
%246 = OpLoad %v2float %coordSampled
OpStore %247 %246
%248 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %245 %247
%249 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%250 = OpLoad %v4float %249
%251 = OpCompositeExtract %float %250 3
%252 = OpVectorTimesScalar %v4float %248 %251
%253 = OpFAdd %v4float %243 %252
OpStore %_output_1 %253
%254 = OpLoad %v2float %coord
%255 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%256 = OpLoad %v2float %255
%257 = OpFAdd %v2float %254 %256
OpStore %coord %257
%258 = OpLoad %v2float %coord
OpStore %coordSampled %258
%259 = OpLoad %v4float %_output_1
%260 = OpLoad %v4float %114
OpStore %261 %260
%262 = OpLoad %v2float %coordSampled
OpStore %263 %262
%264 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %261 %263
%265 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%266 = OpLoad %v4float %265
%267 = OpCompositeExtract %float %266 0
%268 = OpVectorTimesScalar %v4float %264 %267
%269 = OpFAdd %v4float %259 %268
OpStore %_output_1 %269
%270 = OpLoad %v2float %coord
%271 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%272 = OpLoad %v2float %271
%273 = OpFAdd %v2float %270 %272
OpStore %coord %273
%274 = OpLoad %v2float %coord
OpStore %coordSampled %274
%275 = OpLoad %v4float %_output_1
%276 = OpLoad %v4float %114
OpStore %277 %276
%278 = OpLoad %v2float %coordSampled
OpStore %279 %278
%280 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %277 %279
%281 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%282 = OpLoad %v4float %281
%283 = OpCompositeExtract %float %282 1
%284 = OpVectorTimesScalar %v4float %280 %283
%285 = OpFAdd %v4float %275 %284
OpStore %_output_1 %285
%286 = OpLoad %v2float %coord
%287 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%288 = OpLoad %v2float %287
%289 = OpFAdd %v2float %286 %288
OpStore %coord %289
%290 = OpLoad %v2float %coord
OpStore %coordSampled %290
%291 = OpLoad %v4float %_output_1
%292 = OpLoad %v4float %114
OpStore %293 %292
%294 = OpLoad %v2float %coordSampled
OpStore %295 %294
%296 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %293 %295
%297 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%298 = OpLoad %v4float %297
%299 = OpCompositeExtract %float %298 2
%300 = OpVectorTimesScalar %v4float %296 %299
%301 = OpFAdd %v4float %291 %300
OpStore %_output_1 %301
%302 = OpLoad %v2float %coord
%303 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%304 = OpLoad %v2float %303
%305 = OpFAdd %v2float %302 %304
OpStore %coord %305
%306 = OpLoad %v2float %coord
OpStore %coordSampled %306
%307 = OpLoad %v4float %_output_1
%308 = OpLoad %v4float %114
OpStore %309 %308
%310 = OpLoad %v2float %coordSampled
OpStore %311 %310
%312 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %309 %311
%313 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%314 = OpLoad %v4float %313
%315 = OpCompositeExtract %float %314 3
%316 = OpVectorTimesScalar %v4float %312 %315
%317 = OpFAdd %v4float %307 %316
OpStore %_output_1 %317
%318 = OpLoad %v2float %coord
%319 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%320 = OpLoad %v2float %319
%321 = OpFAdd %v2float %318 %320
OpStore %coord %321
%322 = OpLoad %v2float %coord
OpStore %coordSampled %322
%323 = OpLoad %v4float %_output_1
%324 = OpLoad %v4float %114
OpStore %325 %324
%326 = OpLoad %v2float %coordSampled
OpStore %327 %326
%328 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %325 %327
%329 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%330 = OpLoad %v4float %329
%331 = OpCompositeExtract %float %330 0
%332 = OpVectorTimesScalar %v4float %328 %331
%333 = OpFAdd %v4float %323 %332
OpStore %_output_1 %333
%334 = OpLoad %v2float %coord
%335 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%336 = OpLoad %v2float %335
%337 = OpFAdd %v2float %334 %336
OpStore %coord %337
%338 = OpLoad %v2float %coord
OpStore %coordSampled %338
%339 = OpLoad %v4float %_output_1
%340 = OpLoad %v4float %114
OpStore %341 %340
%342 = OpLoad %v2float %coordSampled
OpStore %343 %342
%344 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %341 %343
%345 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%346 = OpLoad %v4float %345
%347 = OpCompositeExtract %float %346 1
%348 = OpVectorTimesScalar %v4float %344 %347
%349 = OpFAdd %v4float %339 %348
OpStore %_output_1 %349
%350 = OpLoad %v2float %coord
%351 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%352 = OpLoad %v2float %351
%353 = OpFAdd %v2float %350 %352
OpStore %coord %353
%354 = OpLoad %v2float %coord
OpStore %coordSampled %354
%355 = OpLoad %v4float %_output_1
%356 = OpLoad %v4float %114
OpStore %357 %356
%358 = OpLoad %v2float %coordSampled
OpStore %359 %358
%360 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %357 %359
%361 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%362 = OpLoad %v4float %361
%363 = OpCompositeExtract %float %362 2
%364 = OpVectorTimesScalar %v4float %360 %363
%365 = OpFAdd %v4float %355 %364
OpStore %_output_1 %365
%366 = OpLoad %v2float %coord
%367 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%368 = OpLoad %v2float %367
%369 = OpFAdd %v2float %366 %368
OpStore %coord %369
%370 = OpLoad %v2float %coord
OpStore %coordSampled %370
%371 = OpLoad %v4float %_output_1
%372 = OpLoad %v4float %114
OpStore %373 %372
%374 = OpLoad %v2float %coordSampled
OpStore %375 %374
%376 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %373 %375
%377 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%378 = OpLoad %v4float %377
%379 = OpCompositeExtract %float %378 3
%380 = OpVectorTimesScalar %v4float %376 %379
%381 = OpFAdd %v4float %371 %380
OpStore %_output_1 %381
%382 = OpLoad %v2float %coord
%383 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%384 = OpLoad %v2float %383
%385 = OpFAdd %v2float %382 %384
OpStore %coord %385
%386 = OpLoad %v2float %coord
OpStore %coordSampled %386
%387 = OpLoad %v4float %_output_1
%388 = OpLoad %v4float %114
OpStore %389 %388
%390 = OpLoad %v2float %coordSampled
OpStore %391 %390
%392 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %389 %391
%393 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%394 = OpLoad %v4float %393
%395 = OpCompositeExtract %float %394 0
%396 = OpVectorTimesScalar %v4float %392 %395
%397 = OpFAdd %v4float %387 %396
OpStore %_output_1 %397
%398 = OpLoad %v2float %coord
%399 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%400 = OpLoad %v2float %399
%401 = OpFAdd %v2float %398 %400
OpStore %coord %401
%402 = OpLoad %v2float %coord
OpStore %coordSampled %402
%403 = OpLoad %v4float %_output_1
%404 = OpLoad %v4float %114
OpStore %405 %404
%406 = OpLoad %v2float %coordSampled
OpStore %407 %406
%408 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %405 %407
%409 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%410 = OpLoad %v4float %409
%411 = OpCompositeExtract %float %410 1
%412 = OpVectorTimesScalar %v4float %408 %411
%413 = OpFAdd %v4float %403 %412
OpStore %_output_1 %413
%414 = OpLoad %v2float %coord
%415 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%416 = OpLoad %v2float %415
%417 = OpFAdd %v2float %414 %416
OpStore %coord %417
%418 = OpLoad %v2float %coord
OpStore %coordSampled %418
%419 = OpLoad %v4float %_output_1
%420 = OpLoad %v4float %114
OpStore %421 %420
%422 = OpLoad %v2float %coordSampled
OpStore %423 %422
%424 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %421 %423
%425 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%426 = OpLoad %v4float %425
%427 = OpCompositeExtract %float %426 2
%428 = OpVectorTimesScalar %v4float %424 %427
%429 = OpFAdd %v4float %419 %428
OpStore %_output_1 %429
%430 = OpLoad %v2float %coord
%431 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%432 = OpLoad %v2float %431
%433 = OpFAdd %v2float %430 %432
OpStore %coord %433
%434 = OpLoad %v2float %coord
OpStore %coordSampled %434
%435 = OpLoad %v4float %_output_1
%436 = OpLoad %v4float %114
OpStore %437 %436
%438 = OpLoad %v2float %coordSampled
OpStore %439 %438
%440 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %437 %439
%441 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%442 = OpLoad %v4float %441
%443 = OpCompositeExtract %float %442 3
%444 = OpVectorTimesScalar %v4float %440 %443
%445 = OpFAdd %v4float %435 %444
OpStore %_output_1 %445
%446 = OpLoad %v2float %coord
%447 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%448 = OpLoad %v2float %447
%449 = OpFAdd %v2float %446 %448
OpStore %coord %449
%450 = OpLoad %v2float %coord
OpStore %coordSampled %450
%451 = OpLoad %v4float %_output_1
%452 = OpLoad %v4float %114
OpStore %453 %452
%454 = OpLoad %v2float %coordSampled
OpStore %455 %454
%456 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %453 %455
%457 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%458 = OpLoad %v4float %457
%459 = OpCompositeExtract %float %458 0
%460 = OpVectorTimesScalar %v4float %456 %459
%461 = OpFAdd %v4float %451 %460
OpStore %_output_1 %461
%462 = OpLoad %v2float %coord
%463 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%464 = OpLoad %v2float %463
%465 = OpFAdd %v2float %462 %464
OpStore %coord %465
%466 = OpLoad %v2float %coord
OpStore %coordSampled %466
%467 = OpLoad %v4float %_output_1
%468 = OpLoad %v4float %114
OpStore %469 %468
%470 = OpLoad %v2float %coordSampled
OpStore %471 %470
%472 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %469 %471
%473 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%474 = OpLoad %v4float %473
%475 = OpCompositeExtract %float %474 1
%476 = OpVectorTimesScalar %v4float %472 %475
%477 = OpFAdd %v4float %467 %476
OpStore %_output_1 %477
%478 = OpLoad %v2float %coord
%479 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%480 = OpLoad %v2float %479
%481 = OpFAdd %v2float %478 %480
OpStore %coord %481
%482 = OpLoad %v2float %coord
OpStore %coordSampled %482
%483 = OpLoad %v4float %_output_1
%484 = OpLoad %v4float %114
OpStore %485 %484
%486 = OpLoad %v2float %coordSampled
OpStore %487 %486
%488 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %485 %487
%489 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%490 = OpLoad %v4float %489
%491 = OpCompositeExtract %float %490 2
%492 = OpVectorTimesScalar %v4float %488 %491
%493 = OpFAdd %v4float %483 %492
OpStore %_output_1 %493
%494 = OpLoad %v2float %coord
%495 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%496 = OpLoad %v2float %495
%497 = OpFAdd %v2float %494 %496
OpStore %coord %497
%498 = OpLoad %v2float %coord
OpStore %coordSampled %498
%499 = OpLoad %v4float %_output_1
%500 = OpLoad %v4float %114
OpStore %501 %500
%502 = OpLoad %v2float %coordSampled
OpStore %503 %502
%504 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %501 %503
%505 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%506 = OpLoad %v4float %505
%507 = OpCompositeExtract %float %506 3
%508 = OpVectorTimesScalar %v4float %504 %507
%509 = OpFAdd %v4float %499 %508
OpStore %_output_1 %509
%510 = OpLoad %v2float %coord
%511 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%512 = OpLoad %v2float %511
%513 = OpFAdd %v2float %510 %512
OpStore %coord %513
%514 = OpLoad %v2float %coord
OpStore %coordSampled %514
%515 = OpLoad %v4float %_output_1
%516 = OpLoad %v4float %114
OpStore %517 %516
%518 = OpLoad %v2float %coordSampled
OpStore %519 %518
%520 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %517 %519
%521 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_6
%522 = OpLoad %v4float %521
%523 = OpCompositeExtract %float %522 0
%524 = OpVectorTimesScalar %v4float %520 %523
%525 = OpFAdd %v4float %515 %524
OpStore %_output_1 %525
%526 = OpLoad %v2float %coord
%527 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%528 = OpLoad %v2float %527
%529 = OpFAdd %v2float %526 %528
OpStore %coord %529
%530 = OpLoad %v4float %_output_1
%531 = OpLoad %v4float %114
%532 = OpFMul %v4float %530 %531
OpStore %_output_1 %532
%533 = OpLoad %v4float %_output_1
OpReturnValue %533
OpFunctionEnd
%main = OpFunction %void None %535
%536 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%543 = OpVariable %_ptr_Function_v4float Function
OpStore %outputColor_Stage0 %540
OpStore %outputCoverage_Stage0 %540
%542 = OpLoad %v4float %outputColor_Stage0
OpStore %543 %542
%544 = OpFunctionCall %v4float %GaussianConvolution_Stage1_c0 %543
OpStore %output_Stage1 %544
%545 = OpLoad %v4float %output_Stage1
%546 = OpLoad %v4float %outputCoverage_Stage0
%547 = OpFMul %v4float %545 %546
OpStore %sk_FragColor %547
OpReturn
OpFunctionEnd
