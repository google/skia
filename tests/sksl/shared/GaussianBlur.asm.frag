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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %58 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %511 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %540 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%112 = OpTypeFunction %v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%117 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%127 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%void = OpTypeVoid
%534 = OpTypeFunction %void
%538 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%110 = OpVariable %_ptr_Function_v2float Function
%97 = OpLoad %v4float %93
OpStore %98 %97
%100 = OpAccessChain %_ptr_Uniform_mat3v3float %6 %int_3
%102 = OpLoad %mat3v3float %100
%103 = OpLoad %v2float %94
%104 = OpCompositeExtract %float %103 0
%105 = OpCompositeExtract %float %103 1
%107 = OpCompositeConstruct %v3float %104 %105 %float_1
%108 = OpMatrixTimesVector %v3float %102 %107
%109 = OpVectorShuffle %v2float %108 %108 0 1
OpStore %110 %109
%111 = OpFunctionCall %v4float %TextureEffect_Stage1_c0_c0_c0 %98 %110
OpReturnValue %111
OpFunctionEnd
%GaussianConvolution_Stage1_c0 = OpFunction %v4float None %112
%113 = OpFunctionParameter %_ptr_Function_v4float
%114 = OpLabel
%_output_1 = OpVariable %_ptr_Function_v4float Function
%coord = OpVariable %_ptr_Function_v2float Function
%coordSampled = OpVariable %_ptr_Function_v2float Function
%131 = OpVariable %_ptr_Function_v4float Function
%133 = OpVariable %_ptr_Function_v2float Function
%148 = OpVariable %_ptr_Function_v4float Function
%150 = OpVariable %_ptr_Function_v2float Function
%164 = OpVariable %_ptr_Function_v4float Function
%166 = OpVariable %_ptr_Function_v2float Function
%180 = OpVariable %_ptr_Function_v4float Function
%182 = OpVariable %_ptr_Function_v2float Function
%196 = OpVariable %_ptr_Function_v4float Function
%198 = OpVariable %_ptr_Function_v2float Function
%212 = OpVariable %_ptr_Function_v4float Function
%214 = OpVariable %_ptr_Function_v2float Function
%228 = OpVariable %_ptr_Function_v4float Function
%230 = OpVariable %_ptr_Function_v2float Function
%244 = OpVariable %_ptr_Function_v4float Function
%246 = OpVariable %_ptr_Function_v2float Function
%260 = OpVariable %_ptr_Function_v4float Function
%262 = OpVariable %_ptr_Function_v2float Function
%276 = OpVariable %_ptr_Function_v4float Function
%278 = OpVariable %_ptr_Function_v2float Function
%292 = OpVariable %_ptr_Function_v4float Function
%294 = OpVariable %_ptr_Function_v2float Function
%308 = OpVariable %_ptr_Function_v4float Function
%310 = OpVariable %_ptr_Function_v2float Function
%324 = OpVariable %_ptr_Function_v4float Function
%326 = OpVariable %_ptr_Function_v2float Function
%340 = OpVariable %_ptr_Function_v4float Function
%342 = OpVariable %_ptr_Function_v2float Function
%356 = OpVariable %_ptr_Function_v4float Function
%358 = OpVariable %_ptr_Function_v2float Function
%372 = OpVariable %_ptr_Function_v4float Function
%374 = OpVariable %_ptr_Function_v2float Function
%388 = OpVariable %_ptr_Function_v4float Function
%390 = OpVariable %_ptr_Function_v2float Function
%404 = OpVariable %_ptr_Function_v4float Function
%406 = OpVariable %_ptr_Function_v2float Function
%420 = OpVariable %_ptr_Function_v4float Function
%422 = OpVariable %_ptr_Function_v2float Function
%436 = OpVariable %_ptr_Function_v4float Function
%438 = OpVariable %_ptr_Function_v2float Function
%452 = OpVariable %_ptr_Function_v4float Function
%454 = OpVariable %_ptr_Function_v2float Function
%468 = OpVariable %_ptr_Function_v4float Function
%470 = OpVariable %_ptr_Function_v2float Function
%484 = OpVariable %_ptr_Function_v4float Function
%486 = OpVariable %_ptr_Function_v2float Function
%500 = OpVariable %_ptr_Function_v4float Function
%502 = OpVariable %_ptr_Function_v2float Function
%516 = OpVariable %_ptr_Function_v4float Function
%518 = OpVariable %_ptr_Function_v2float Function
OpStore %_output_1 %117
%119 = OpLoad %v2float %vLocalCoord_Stage0
%121 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%123 = OpLoad %v2float %121
%124 = OpVectorTimesScalar %v2float %123 %float_12
%125 = OpFSub %v2float %119 %124
OpStore %coord %125
OpStore %coordSampled %127
%128 = OpLoad %v2float %coord
OpStore %coordSampled %128
%129 = OpLoad %v4float %_output_1
%130 = OpLoad %v4float %113
OpStore %131 %130
%132 = OpLoad %v2float %coordSampled
OpStore %133 %132
%134 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %131 %133
%136 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 0
%139 = OpVectorTimesScalar %v4float %134 %138
%140 = OpFAdd %v4float %129 %139
OpStore %_output_1 %140
%141 = OpLoad %v2float %coord
%142 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%143 = OpLoad %v2float %142
%144 = OpFAdd %v2float %141 %143
OpStore %coord %144
%145 = OpLoad %v2float %coord
OpStore %coordSampled %145
%146 = OpLoad %v4float %_output_1
%147 = OpLoad %v4float %113
OpStore %148 %147
%149 = OpLoad %v2float %coordSampled
OpStore %150 %149
%151 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %148 %150
%152 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 1
%155 = OpVectorTimesScalar %v4float %151 %154
%156 = OpFAdd %v4float %146 %155
OpStore %_output_1 %156
%157 = OpLoad %v2float %coord
%158 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%159 = OpLoad %v2float %158
%160 = OpFAdd %v2float %157 %159
OpStore %coord %160
%161 = OpLoad %v2float %coord
OpStore %coordSampled %161
%162 = OpLoad %v4float %_output_1
%163 = OpLoad %v4float %113
OpStore %164 %163
%165 = OpLoad %v2float %coordSampled
OpStore %166 %165
%167 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %164 %166
%168 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%169 = OpLoad %v4float %168
%170 = OpCompositeExtract %float %169 2
%171 = OpVectorTimesScalar %v4float %167 %170
%172 = OpFAdd %v4float %162 %171
OpStore %_output_1 %172
%173 = OpLoad %v2float %coord
%174 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%175 = OpLoad %v2float %174
%176 = OpFAdd %v2float %173 %175
OpStore %coord %176
%177 = OpLoad %v2float %coord
OpStore %coordSampled %177
%178 = OpLoad %v4float %_output_1
%179 = OpLoad %v4float %113
OpStore %180 %179
%181 = OpLoad %v2float %coordSampled
OpStore %182 %181
%183 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %180 %182
%184 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%185 = OpLoad %v4float %184
%186 = OpCompositeExtract %float %185 3
%187 = OpVectorTimesScalar %v4float %183 %186
%188 = OpFAdd %v4float %178 %187
OpStore %_output_1 %188
%189 = OpLoad %v2float %coord
%190 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%191 = OpLoad %v2float %190
%192 = OpFAdd %v2float %189 %191
OpStore %coord %192
%193 = OpLoad %v2float %coord
OpStore %coordSampled %193
%194 = OpLoad %v4float %_output_1
%195 = OpLoad %v4float %113
OpStore %196 %195
%197 = OpLoad %v2float %coordSampled
OpStore %198 %197
%199 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %196 %198
%200 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%201 = OpLoad %v4float %200
%202 = OpCompositeExtract %float %201 0
%203 = OpVectorTimesScalar %v4float %199 %202
%204 = OpFAdd %v4float %194 %203
OpStore %_output_1 %204
%205 = OpLoad %v2float %coord
%206 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%207 = OpLoad %v2float %206
%208 = OpFAdd %v2float %205 %207
OpStore %coord %208
%209 = OpLoad %v2float %coord
OpStore %coordSampled %209
%210 = OpLoad %v4float %_output_1
%211 = OpLoad %v4float %113
OpStore %212 %211
%213 = OpLoad %v2float %coordSampled
OpStore %214 %213
%215 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %212 %214
%216 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%217 = OpLoad %v4float %216
%218 = OpCompositeExtract %float %217 1
%219 = OpVectorTimesScalar %v4float %215 %218
%220 = OpFAdd %v4float %210 %219
OpStore %_output_1 %220
%221 = OpLoad %v2float %coord
%222 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%223 = OpLoad %v2float %222
%224 = OpFAdd %v2float %221 %223
OpStore %coord %224
%225 = OpLoad %v2float %coord
OpStore %coordSampled %225
%226 = OpLoad %v4float %_output_1
%227 = OpLoad %v4float %113
OpStore %228 %227
%229 = OpLoad %v2float %coordSampled
OpStore %230 %229
%231 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %228 %230
%232 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%233 = OpLoad %v4float %232
%234 = OpCompositeExtract %float %233 2
%235 = OpVectorTimesScalar %v4float %231 %234
%236 = OpFAdd %v4float %226 %235
OpStore %_output_1 %236
%237 = OpLoad %v2float %coord
%238 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%239 = OpLoad %v2float %238
%240 = OpFAdd %v2float %237 %239
OpStore %coord %240
%241 = OpLoad %v2float %coord
OpStore %coordSampled %241
%242 = OpLoad %v4float %_output_1
%243 = OpLoad %v4float %113
OpStore %244 %243
%245 = OpLoad %v2float %coordSampled
OpStore %246 %245
%247 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %244 %246
%248 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%249 = OpLoad %v4float %248
%250 = OpCompositeExtract %float %249 3
%251 = OpVectorTimesScalar %v4float %247 %250
%252 = OpFAdd %v4float %242 %251
OpStore %_output_1 %252
%253 = OpLoad %v2float %coord
%254 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%255 = OpLoad %v2float %254
%256 = OpFAdd %v2float %253 %255
OpStore %coord %256
%257 = OpLoad %v2float %coord
OpStore %coordSampled %257
%258 = OpLoad %v4float %_output_1
%259 = OpLoad %v4float %113
OpStore %260 %259
%261 = OpLoad %v2float %coordSampled
OpStore %262 %261
%263 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %260 %262
%264 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%265 = OpLoad %v4float %264
%266 = OpCompositeExtract %float %265 0
%267 = OpVectorTimesScalar %v4float %263 %266
%268 = OpFAdd %v4float %258 %267
OpStore %_output_1 %268
%269 = OpLoad %v2float %coord
%270 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%271 = OpLoad %v2float %270
%272 = OpFAdd %v2float %269 %271
OpStore %coord %272
%273 = OpLoad %v2float %coord
OpStore %coordSampled %273
%274 = OpLoad %v4float %_output_1
%275 = OpLoad %v4float %113
OpStore %276 %275
%277 = OpLoad %v2float %coordSampled
OpStore %278 %277
%279 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %276 %278
%280 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%281 = OpLoad %v4float %280
%282 = OpCompositeExtract %float %281 1
%283 = OpVectorTimesScalar %v4float %279 %282
%284 = OpFAdd %v4float %274 %283
OpStore %_output_1 %284
%285 = OpLoad %v2float %coord
%286 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%287 = OpLoad %v2float %286
%288 = OpFAdd %v2float %285 %287
OpStore %coord %288
%289 = OpLoad %v2float %coord
OpStore %coordSampled %289
%290 = OpLoad %v4float %_output_1
%291 = OpLoad %v4float %113
OpStore %292 %291
%293 = OpLoad %v2float %coordSampled
OpStore %294 %293
%295 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %292 %294
%296 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%297 = OpLoad %v4float %296
%298 = OpCompositeExtract %float %297 2
%299 = OpVectorTimesScalar %v4float %295 %298
%300 = OpFAdd %v4float %290 %299
OpStore %_output_1 %300
%301 = OpLoad %v2float %coord
%302 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%303 = OpLoad %v2float %302
%304 = OpFAdd %v2float %301 %303
OpStore %coord %304
%305 = OpLoad %v2float %coord
OpStore %coordSampled %305
%306 = OpLoad %v4float %_output_1
%307 = OpLoad %v4float %113
OpStore %308 %307
%309 = OpLoad %v2float %coordSampled
OpStore %310 %309
%311 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %308 %310
%312 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%313 = OpLoad %v4float %312
%314 = OpCompositeExtract %float %313 3
%315 = OpVectorTimesScalar %v4float %311 %314
%316 = OpFAdd %v4float %306 %315
OpStore %_output_1 %316
%317 = OpLoad %v2float %coord
%318 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%319 = OpLoad %v2float %318
%320 = OpFAdd %v2float %317 %319
OpStore %coord %320
%321 = OpLoad %v2float %coord
OpStore %coordSampled %321
%322 = OpLoad %v4float %_output_1
%323 = OpLoad %v4float %113
OpStore %324 %323
%325 = OpLoad %v2float %coordSampled
OpStore %326 %325
%327 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %324 %326
%328 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%329 = OpLoad %v4float %328
%330 = OpCompositeExtract %float %329 0
%331 = OpVectorTimesScalar %v4float %327 %330
%332 = OpFAdd %v4float %322 %331
OpStore %_output_1 %332
%333 = OpLoad %v2float %coord
%334 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%335 = OpLoad %v2float %334
%336 = OpFAdd %v2float %333 %335
OpStore %coord %336
%337 = OpLoad %v2float %coord
OpStore %coordSampled %337
%338 = OpLoad %v4float %_output_1
%339 = OpLoad %v4float %113
OpStore %340 %339
%341 = OpLoad %v2float %coordSampled
OpStore %342 %341
%343 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %340 %342
%344 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%345 = OpLoad %v4float %344
%346 = OpCompositeExtract %float %345 1
%347 = OpVectorTimesScalar %v4float %343 %346
%348 = OpFAdd %v4float %338 %347
OpStore %_output_1 %348
%349 = OpLoad %v2float %coord
%350 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%351 = OpLoad %v2float %350
%352 = OpFAdd %v2float %349 %351
OpStore %coord %352
%353 = OpLoad %v2float %coord
OpStore %coordSampled %353
%354 = OpLoad %v4float %_output_1
%355 = OpLoad %v4float %113
OpStore %356 %355
%357 = OpLoad %v2float %coordSampled
OpStore %358 %357
%359 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %356 %358
%360 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%361 = OpLoad %v4float %360
%362 = OpCompositeExtract %float %361 2
%363 = OpVectorTimesScalar %v4float %359 %362
%364 = OpFAdd %v4float %354 %363
OpStore %_output_1 %364
%365 = OpLoad %v2float %coord
%366 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%367 = OpLoad %v2float %366
%368 = OpFAdd %v2float %365 %367
OpStore %coord %368
%369 = OpLoad %v2float %coord
OpStore %coordSampled %369
%370 = OpLoad %v4float %_output_1
%371 = OpLoad %v4float %113
OpStore %372 %371
%373 = OpLoad %v2float %coordSampled
OpStore %374 %373
%375 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %372 %374
%376 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%377 = OpLoad %v4float %376
%378 = OpCompositeExtract %float %377 3
%379 = OpVectorTimesScalar %v4float %375 %378
%380 = OpFAdd %v4float %370 %379
OpStore %_output_1 %380
%381 = OpLoad %v2float %coord
%382 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%383 = OpLoad %v2float %382
%384 = OpFAdd %v2float %381 %383
OpStore %coord %384
%385 = OpLoad %v2float %coord
OpStore %coordSampled %385
%386 = OpLoad %v4float %_output_1
%387 = OpLoad %v4float %113
OpStore %388 %387
%389 = OpLoad %v2float %coordSampled
OpStore %390 %389
%391 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %388 %390
%392 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%393 = OpLoad %v4float %392
%394 = OpCompositeExtract %float %393 0
%395 = OpVectorTimesScalar %v4float %391 %394
%396 = OpFAdd %v4float %386 %395
OpStore %_output_1 %396
%397 = OpLoad %v2float %coord
%398 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%399 = OpLoad %v2float %398
%400 = OpFAdd %v2float %397 %399
OpStore %coord %400
%401 = OpLoad %v2float %coord
OpStore %coordSampled %401
%402 = OpLoad %v4float %_output_1
%403 = OpLoad %v4float %113
OpStore %404 %403
%405 = OpLoad %v2float %coordSampled
OpStore %406 %405
%407 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %404 %406
%408 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%409 = OpLoad %v4float %408
%410 = OpCompositeExtract %float %409 1
%411 = OpVectorTimesScalar %v4float %407 %410
%412 = OpFAdd %v4float %402 %411
OpStore %_output_1 %412
%413 = OpLoad %v2float %coord
%414 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%415 = OpLoad %v2float %414
%416 = OpFAdd %v2float %413 %415
OpStore %coord %416
%417 = OpLoad %v2float %coord
OpStore %coordSampled %417
%418 = OpLoad %v4float %_output_1
%419 = OpLoad %v4float %113
OpStore %420 %419
%421 = OpLoad %v2float %coordSampled
OpStore %422 %421
%423 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %420 %422
%424 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%425 = OpLoad %v4float %424
%426 = OpCompositeExtract %float %425 2
%427 = OpVectorTimesScalar %v4float %423 %426
%428 = OpFAdd %v4float %418 %427
OpStore %_output_1 %428
%429 = OpLoad %v2float %coord
%430 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%431 = OpLoad %v2float %430
%432 = OpFAdd %v2float %429 %431
OpStore %coord %432
%433 = OpLoad %v2float %coord
OpStore %coordSampled %433
%434 = OpLoad %v4float %_output_1
%435 = OpLoad %v4float %113
OpStore %436 %435
%437 = OpLoad %v2float %coordSampled
OpStore %438 %437
%439 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %436 %438
%440 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%441 = OpLoad %v4float %440
%442 = OpCompositeExtract %float %441 3
%443 = OpVectorTimesScalar %v4float %439 %442
%444 = OpFAdd %v4float %434 %443
OpStore %_output_1 %444
%445 = OpLoad %v2float %coord
%446 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%447 = OpLoad %v2float %446
%448 = OpFAdd %v2float %445 %447
OpStore %coord %448
%449 = OpLoad %v2float %coord
OpStore %coordSampled %449
%450 = OpLoad %v4float %_output_1
%451 = OpLoad %v4float %113
OpStore %452 %451
%453 = OpLoad %v2float %coordSampled
OpStore %454 %453
%455 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %452 %454
%456 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%457 = OpLoad %v4float %456
%458 = OpCompositeExtract %float %457 0
%459 = OpVectorTimesScalar %v4float %455 %458
%460 = OpFAdd %v4float %450 %459
OpStore %_output_1 %460
%461 = OpLoad %v2float %coord
%462 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%463 = OpLoad %v2float %462
%464 = OpFAdd %v2float %461 %463
OpStore %coord %464
%465 = OpLoad %v2float %coord
OpStore %coordSampled %465
%466 = OpLoad %v4float %_output_1
%467 = OpLoad %v4float %113
OpStore %468 %467
%469 = OpLoad %v2float %coordSampled
OpStore %470 %469
%471 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %468 %470
%472 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%473 = OpLoad %v4float %472
%474 = OpCompositeExtract %float %473 1
%475 = OpVectorTimesScalar %v4float %471 %474
%476 = OpFAdd %v4float %466 %475
OpStore %_output_1 %476
%477 = OpLoad %v2float %coord
%478 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%479 = OpLoad %v2float %478
%480 = OpFAdd %v2float %477 %479
OpStore %coord %480
%481 = OpLoad %v2float %coord
OpStore %coordSampled %481
%482 = OpLoad %v4float %_output_1
%483 = OpLoad %v4float %113
OpStore %484 %483
%485 = OpLoad %v2float %coordSampled
OpStore %486 %485
%487 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %484 %486
%488 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%489 = OpLoad %v4float %488
%490 = OpCompositeExtract %float %489 2
%491 = OpVectorTimesScalar %v4float %487 %490
%492 = OpFAdd %v4float %482 %491
OpStore %_output_1 %492
%493 = OpLoad %v2float %coord
%494 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%495 = OpLoad %v2float %494
%496 = OpFAdd %v2float %493 %495
OpStore %coord %496
%497 = OpLoad %v2float %coord
OpStore %coordSampled %497
%498 = OpLoad %v4float %_output_1
%499 = OpLoad %v4float %113
OpStore %500 %499
%501 = OpLoad %v2float %coordSampled
OpStore %502 %501
%503 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %500 %502
%504 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%505 = OpLoad %v4float %504
%506 = OpCompositeExtract %float %505 3
%507 = OpVectorTimesScalar %v4float %503 %506
%508 = OpFAdd %v4float %498 %507
OpStore %_output_1 %508
%509 = OpLoad %v2float %coord
%510 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%511 = OpLoad %v2float %510
%512 = OpFAdd %v2float %509 %511
OpStore %coord %512
%513 = OpLoad %v2float %coord
OpStore %coordSampled %513
%514 = OpLoad %v4float %_output_1
%515 = OpLoad %v4float %113
OpStore %516 %515
%517 = OpLoad %v2float %coordSampled
OpStore %518 %517
%519 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %516 %518
%520 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_6
%521 = OpLoad %v4float %520
%522 = OpCompositeExtract %float %521 0
%523 = OpVectorTimesScalar %v4float %519 %522
%524 = OpFAdd %v4float %514 %523
OpStore %_output_1 %524
%525 = OpLoad %v2float %coord
%526 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%527 = OpLoad %v2float %526
%528 = OpFAdd %v2float %525 %527
OpStore %coord %528
%529 = OpLoad %v4float %_output_1
%530 = OpLoad %v4float %113
%531 = OpFMul %v4float %529 %530
OpStore %_output_1 %531
%532 = OpLoad %v4float %_output_1
OpReturnValue %532
OpFunctionEnd
%main = OpFunction %void None %534
%535 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%541 = OpVariable %_ptr_Function_v4float Function
OpStore %outputColor_Stage0 %538
OpStore %outputCoverage_Stage0 %538
%540 = OpLoad %v4float %outputColor_Stage0
OpStore %541 %540
%542 = OpFunctionCall %v4float %GaussianConvolution_Stage1_c0 %541
OpStore %output_Stage1 %542
%543 = OpLoad %v4float %output_Stage1
%544 = OpLoad %v4float %outputCoverage_Stage0
%545 = OpFMul %v4float %543 %544
OpStore %sk_FragColor %545
OpReturn
OpFunctionEnd
