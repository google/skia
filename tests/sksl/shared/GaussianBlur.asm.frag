OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %vLocalCoord_Stage0
OpExecutionMode %main OriginUpperLeft
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "sk_RTAdjust"
OpMemberName %uniformBuffer 1 "uIncrement_Stage1_c0"
OpMemberName %uniformBuffer 2 "uKernel_Stage1_c0"
OpMemberName %uniformBuffer 3 "umatrix_Stage1_c0_c0"
OpMemberName %uniformBuffer 4 "uborder_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 5 "usubset_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 6 "unorm_Stage1_c0_c0_c0"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %uTextureSampler_0_Stage1 "uTextureSampler_0_Stage1"
OpName %vLocalCoord_Stage0 "vLocalCoord_Stage0"
OpName %TextureEffect_Stage1_c0_c0_c0_h4h4f2 "TextureEffect_Stage1_c0_c0_c0_h4h4f2"
OpName %inCoord "inCoord"
OpName %subsetCoord "subsetCoord"
OpName %clampedCoord "clampedCoord"
OpName %textureColor "textureColor"
OpName %snappedX "snappedX"
OpName %MatrixEffect_Stage1_c0_c0_h4h4f2 "MatrixEffect_Stage1_c0_c0_h4h4f2"
OpName %GaussianConvolution_Stage1_c0_h4h4 "GaussianConvolution_Stage1_c0_h4h4"
OpName %_output "_output"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %textureColor RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %_output RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
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
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%23 = OpTypeImage %float 2D 0 0 0 1 Unknown
%24 = OpTypeSampledImage %23
%_ptr_UniformConstant_24 = OpTypePointer UniformConstant %24
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_24 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%30 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
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
%105 = OpTypeFunction %v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%110 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%120 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%void = OpTypeVoid
%425 = OpTypeFunction %void
%429 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%TextureEffect_Stage1_c0_c0_c0_h4h4f2 = OpFunction %v4float None %30
%31 = OpFunctionParameter %_ptr_Function_v4float
%32 = OpFunctionParameter %_ptr_Function_v2float
%33 = OpLabel
%inCoord = OpVariable %_ptr_Function_v2float Function
%subsetCoord = OpVariable %_ptr_Function_v2float Function
%clampedCoord = OpVariable %_ptr_Function_v2float Function
%textureColor = OpVariable %_ptr_Function_v4float Function
%snappedX = OpVariable %_ptr_Function_float Function
%35 = OpLoad %v2float %32
OpStore %inCoord %35
%37 = OpAccessChain %_ptr_Uniform_v4float %6 %int_6
%39 = OpLoad %v4float %37
%40 = OpVectorShuffle %v2float %39 %39 0 1
%41 = OpFMul %v2float %35 %40
OpStore %inCoord %41
%43 = OpCompositeExtract %float %41 0
%44 = OpAccessChain %_ptr_Function_float %subsetCoord %int_0
OpStore %44 %43
%47 = OpLoad %v2float %inCoord
%48 = OpCompositeExtract %float %47 1
%49 = OpAccessChain %_ptr_Function_float %subsetCoord %int_1
OpStore %49 %48
%52 = OpLoad %v2float %subsetCoord
OpStore %clampedCoord %52
%55 = OpLoad %24 %uTextureSampler_0_Stage1
%56 = OpAccessChain %_ptr_Uniform_v4float %6 %int_6
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v2float %57 %57 2 3
%59 = OpFMul %v2float %52 %58
%54 = OpImageSampleImplicitLod %v4float %55 %59
OpStore %textureColor %54
%62 = OpLoad %v2float %inCoord
%63 = OpCompositeExtract %float %62 0
%65 = OpFAdd %float %63 %float_0_00100000005
%61 = OpExtInst %float %1 Floor %65
%67 = OpFAdd %float %61 %float_0_5
OpStore %snappedX %67
%70 = OpAccessChain %_ptr_Uniform_v4float %6 %int_5
%71 = OpLoad %v4float %70
%72 = OpCompositeExtract %float %71 0
%73 = OpFOrdLessThan %bool %67 %72
OpSelectionMerge %75 None
OpBranchConditional %73 %75 %74
%74 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %6 %int_5
%77 = OpLoad %v4float %76
%78 = OpCompositeExtract %float %77 2
%79 = OpFOrdGreaterThan %bool %67 %78
OpBranch %75
%75 = OpLabel
%80 = OpPhi %bool %true %33 %79 %74
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %6 %int_4
%85 = OpLoad %v4float %84
OpStore %textureColor %85
OpBranch %82
%82 = OpLabel
%86 = OpLoad %v4float %textureColor
OpReturnValue %86
OpFunctionEnd
%MatrixEffect_Stage1_c0_c0_h4h4f2 = OpFunction %v4float None %30
%87 = OpFunctionParameter %_ptr_Function_v4float
%88 = OpFunctionParameter %_ptr_Function_v2float
%89 = OpLabel
%91 = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v2float Function
%90 = OpLoad %v4float %87
OpStore %91 %90
%93 = OpAccessChain %_ptr_Uniform_mat3v3float %6 %int_3
%95 = OpLoad %mat3v3float %93
%96 = OpLoad %v2float %88
%97 = OpCompositeExtract %float %96 0
%98 = OpCompositeExtract %float %96 1
%100 = OpCompositeConstruct %v3float %97 %98 %float_1
%101 = OpMatrixTimesVector %v3float %95 %100
%102 = OpVectorShuffle %v2float %101 %101 0 1
OpStore %103 %102
%104 = OpFunctionCall %v4float %TextureEffect_Stage1_c0_c0_c0_h4h4f2 %91 %103
OpReturnValue %104
OpFunctionEnd
%GaussianConvolution_Stage1_c0_h4h4 = OpFunction %v4float None %105
%106 = OpFunctionParameter %_ptr_Function_v4float
%107 = OpLabel
%_output = OpVariable %_ptr_Function_v4float Function
%coord = OpVariable %_ptr_Function_v2float Function
%coordSampled = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v4float Function
%123 = OpVariable %_ptr_Function_v2float Function
%135 = OpVariable %_ptr_Function_v4float Function
%136 = OpVariable %_ptr_Function_v2float Function
%147 = OpVariable %_ptr_Function_v4float Function
%148 = OpVariable %_ptr_Function_v2float Function
%159 = OpVariable %_ptr_Function_v4float Function
%160 = OpVariable %_ptr_Function_v2float Function
%171 = OpVariable %_ptr_Function_v4float Function
%172 = OpVariable %_ptr_Function_v2float Function
%183 = OpVariable %_ptr_Function_v4float Function
%184 = OpVariable %_ptr_Function_v2float Function
%195 = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v2float Function
%207 = OpVariable %_ptr_Function_v4float Function
%208 = OpVariable %_ptr_Function_v2float Function
%219 = OpVariable %_ptr_Function_v4float Function
%220 = OpVariable %_ptr_Function_v2float Function
%231 = OpVariable %_ptr_Function_v4float Function
%232 = OpVariable %_ptr_Function_v2float Function
%243 = OpVariable %_ptr_Function_v4float Function
%244 = OpVariable %_ptr_Function_v2float Function
%255 = OpVariable %_ptr_Function_v4float Function
%256 = OpVariable %_ptr_Function_v2float Function
%267 = OpVariable %_ptr_Function_v4float Function
%268 = OpVariable %_ptr_Function_v2float Function
%279 = OpVariable %_ptr_Function_v4float Function
%280 = OpVariable %_ptr_Function_v2float Function
%291 = OpVariable %_ptr_Function_v4float Function
%292 = OpVariable %_ptr_Function_v2float Function
%303 = OpVariable %_ptr_Function_v4float Function
%304 = OpVariable %_ptr_Function_v2float Function
%315 = OpVariable %_ptr_Function_v4float Function
%316 = OpVariable %_ptr_Function_v2float Function
%327 = OpVariable %_ptr_Function_v4float Function
%328 = OpVariable %_ptr_Function_v2float Function
%339 = OpVariable %_ptr_Function_v4float Function
%340 = OpVariable %_ptr_Function_v2float Function
%351 = OpVariable %_ptr_Function_v4float Function
%352 = OpVariable %_ptr_Function_v2float Function
%363 = OpVariable %_ptr_Function_v4float Function
%364 = OpVariable %_ptr_Function_v2float Function
%375 = OpVariable %_ptr_Function_v4float Function
%376 = OpVariable %_ptr_Function_v2float Function
%387 = OpVariable %_ptr_Function_v4float Function
%388 = OpVariable %_ptr_Function_v2float Function
%399 = OpVariable %_ptr_Function_v4float Function
%400 = OpVariable %_ptr_Function_v2float Function
%411 = OpVariable %_ptr_Function_v4float Function
%412 = OpVariable %_ptr_Function_v2float Function
OpStore %_output %110
%112 = OpLoad %v2float %vLocalCoord_Stage0
%114 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%116 = OpLoad %v2float %114
%117 = OpVectorTimesScalar %v2float %116 %float_12
%118 = OpFSub %v2float %112 %117
OpStore %coord %118
OpStore %coordSampled %120
OpStore %coordSampled %118
%121 = OpLoad %v4float %106
OpStore %122 %121
OpStore %123 %118
%124 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %122 %123
%126 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 0
%129 = OpVectorTimesScalar %v4float %124 %128
%130 = OpFAdd %v4float %110 %129
OpStore %_output %130
%131 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%132 = OpLoad %v2float %131
%133 = OpFAdd %v2float %118 %132
OpStore %coord %133
OpStore %coordSampled %133
%134 = OpLoad %v4float %106
OpStore %135 %134
OpStore %136 %133
%137 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %135 %136
%138 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 1
%141 = OpVectorTimesScalar %v4float %137 %140
%142 = OpFAdd %v4float %130 %141
OpStore %_output %142
%143 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%144 = OpLoad %v2float %143
%145 = OpFAdd %v2float %133 %144
OpStore %coord %145
OpStore %coordSampled %145
%146 = OpLoad %v4float %106
OpStore %147 %146
OpStore %148 %145
%149 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %147 %148
%150 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 2
%153 = OpVectorTimesScalar %v4float %149 %152
%154 = OpFAdd %v4float %142 %153
OpStore %_output %154
%155 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%156 = OpLoad %v2float %155
%157 = OpFAdd %v2float %145 %156
OpStore %coord %157
OpStore %coordSampled %157
%158 = OpLoad %v4float %106
OpStore %159 %158
OpStore %160 %157
%161 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %159 %160
%162 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_0
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 3
%165 = OpVectorTimesScalar %v4float %161 %164
%166 = OpFAdd %v4float %154 %165
OpStore %_output %166
%167 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%168 = OpLoad %v2float %167
%169 = OpFAdd %v2float %157 %168
OpStore %coord %169
OpStore %coordSampled %169
%170 = OpLoad %v4float %106
OpStore %171 %170
OpStore %172 %169
%173 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %171 %172
%174 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 0
%177 = OpVectorTimesScalar %v4float %173 %176
%178 = OpFAdd %v4float %166 %177
OpStore %_output %178
%179 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%180 = OpLoad %v2float %179
%181 = OpFAdd %v2float %169 %180
OpStore %coord %181
OpStore %coordSampled %181
%182 = OpLoad %v4float %106
OpStore %183 %182
OpStore %184 %181
%185 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %183 %184
%186 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 1
%189 = OpVectorTimesScalar %v4float %185 %188
%190 = OpFAdd %v4float %178 %189
OpStore %_output %190
%191 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%192 = OpLoad %v2float %191
%193 = OpFAdd %v2float %181 %192
OpStore %coord %193
OpStore %coordSampled %193
%194 = OpLoad %v4float %106
OpStore %195 %194
OpStore %196 %193
%197 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %195 %196
%198 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%199 = OpLoad %v4float %198
%200 = OpCompositeExtract %float %199 2
%201 = OpVectorTimesScalar %v4float %197 %200
%202 = OpFAdd %v4float %190 %201
OpStore %_output %202
%203 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%204 = OpLoad %v2float %203
%205 = OpFAdd %v2float %193 %204
OpStore %coord %205
OpStore %coordSampled %205
%206 = OpLoad %v4float %106
OpStore %207 %206
OpStore %208 %205
%209 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %207 %208
%210 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_1
%211 = OpLoad %v4float %210
%212 = OpCompositeExtract %float %211 3
%213 = OpVectorTimesScalar %v4float %209 %212
%214 = OpFAdd %v4float %202 %213
OpStore %_output %214
%215 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%216 = OpLoad %v2float %215
%217 = OpFAdd %v2float %205 %216
OpStore %coord %217
OpStore %coordSampled %217
%218 = OpLoad %v4float %106
OpStore %219 %218
OpStore %220 %217
%221 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %219 %220
%222 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%223 = OpLoad %v4float %222
%224 = OpCompositeExtract %float %223 0
%225 = OpVectorTimesScalar %v4float %221 %224
%226 = OpFAdd %v4float %214 %225
OpStore %_output %226
%227 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%228 = OpLoad %v2float %227
%229 = OpFAdd %v2float %217 %228
OpStore %coord %229
OpStore %coordSampled %229
%230 = OpLoad %v4float %106
OpStore %231 %230
OpStore %232 %229
%233 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %231 %232
%234 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%235 = OpLoad %v4float %234
%236 = OpCompositeExtract %float %235 1
%237 = OpVectorTimesScalar %v4float %233 %236
%238 = OpFAdd %v4float %226 %237
OpStore %_output %238
%239 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%240 = OpLoad %v2float %239
%241 = OpFAdd %v2float %229 %240
OpStore %coord %241
OpStore %coordSampled %241
%242 = OpLoad %v4float %106
OpStore %243 %242
OpStore %244 %241
%245 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %243 %244
%246 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 2
%249 = OpVectorTimesScalar %v4float %245 %248
%250 = OpFAdd %v4float %238 %249
OpStore %_output %250
%251 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%252 = OpLoad %v2float %251
%253 = OpFAdd %v2float %241 %252
OpStore %coord %253
OpStore %coordSampled %253
%254 = OpLoad %v4float %106
OpStore %255 %254
OpStore %256 %253
%257 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %255 %256
%258 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_2
%259 = OpLoad %v4float %258
%260 = OpCompositeExtract %float %259 3
%261 = OpVectorTimesScalar %v4float %257 %260
%262 = OpFAdd %v4float %250 %261
OpStore %_output %262
%263 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%264 = OpLoad %v2float %263
%265 = OpFAdd %v2float %253 %264
OpStore %coord %265
OpStore %coordSampled %265
%266 = OpLoad %v4float %106
OpStore %267 %266
OpStore %268 %265
%269 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %267 %268
%270 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%271 = OpLoad %v4float %270
%272 = OpCompositeExtract %float %271 0
%273 = OpVectorTimesScalar %v4float %269 %272
%274 = OpFAdd %v4float %262 %273
OpStore %_output %274
%275 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%276 = OpLoad %v2float %275
%277 = OpFAdd %v2float %265 %276
OpStore %coord %277
OpStore %coordSampled %277
%278 = OpLoad %v4float %106
OpStore %279 %278
OpStore %280 %277
%281 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %279 %280
%282 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%283 = OpLoad %v4float %282
%284 = OpCompositeExtract %float %283 1
%285 = OpVectorTimesScalar %v4float %281 %284
%286 = OpFAdd %v4float %274 %285
OpStore %_output %286
%287 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%288 = OpLoad %v2float %287
%289 = OpFAdd %v2float %277 %288
OpStore %coord %289
OpStore %coordSampled %289
%290 = OpLoad %v4float %106
OpStore %291 %290
OpStore %292 %289
%293 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %291 %292
%294 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%295 = OpLoad %v4float %294
%296 = OpCompositeExtract %float %295 2
%297 = OpVectorTimesScalar %v4float %293 %296
%298 = OpFAdd %v4float %286 %297
OpStore %_output %298
%299 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%300 = OpLoad %v2float %299
%301 = OpFAdd %v2float %289 %300
OpStore %coord %301
OpStore %coordSampled %301
%302 = OpLoad %v4float %106
OpStore %303 %302
OpStore %304 %301
%305 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %303 %304
%306 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_3
%307 = OpLoad %v4float %306
%308 = OpCompositeExtract %float %307 3
%309 = OpVectorTimesScalar %v4float %305 %308
%310 = OpFAdd %v4float %298 %309
OpStore %_output %310
%311 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%312 = OpLoad %v2float %311
%313 = OpFAdd %v2float %301 %312
OpStore %coord %313
OpStore %coordSampled %313
%314 = OpLoad %v4float %106
OpStore %315 %314
OpStore %316 %313
%317 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %315 %316
%318 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%319 = OpLoad %v4float %318
%320 = OpCompositeExtract %float %319 0
%321 = OpVectorTimesScalar %v4float %317 %320
%322 = OpFAdd %v4float %310 %321
OpStore %_output %322
%323 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%324 = OpLoad %v2float %323
%325 = OpFAdd %v2float %313 %324
OpStore %coord %325
OpStore %coordSampled %325
%326 = OpLoad %v4float %106
OpStore %327 %326
OpStore %328 %325
%329 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %327 %328
%330 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%331 = OpLoad %v4float %330
%332 = OpCompositeExtract %float %331 1
%333 = OpVectorTimesScalar %v4float %329 %332
%334 = OpFAdd %v4float %322 %333
OpStore %_output %334
%335 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%336 = OpLoad %v2float %335
%337 = OpFAdd %v2float %325 %336
OpStore %coord %337
OpStore %coordSampled %337
%338 = OpLoad %v4float %106
OpStore %339 %338
OpStore %340 %337
%341 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %339 %340
%342 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%343 = OpLoad %v4float %342
%344 = OpCompositeExtract %float %343 2
%345 = OpVectorTimesScalar %v4float %341 %344
%346 = OpFAdd %v4float %334 %345
OpStore %_output %346
%347 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%348 = OpLoad %v2float %347
%349 = OpFAdd %v2float %337 %348
OpStore %coord %349
OpStore %coordSampled %349
%350 = OpLoad %v4float %106
OpStore %351 %350
OpStore %352 %349
%353 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %351 %352
%354 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_4
%355 = OpLoad %v4float %354
%356 = OpCompositeExtract %float %355 3
%357 = OpVectorTimesScalar %v4float %353 %356
%358 = OpFAdd %v4float %346 %357
OpStore %_output %358
%359 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%360 = OpLoad %v2float %359
%361 = OpFAdd %v2float %349 %360
OpStore %coord %361
OpStore %coordSampled %361
%362 = OpLoad %v4float %106
OpStore %363 %362
OpStore %364 %361
%365 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %363 %364
%366 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%367 = OpLoad %v4float %366
%368 = OpCompositeExtract %float %367 0
%369 = OpVectorTimesScalar %v4float %365 %368
%370 = OpFAdd %v4float %358 %369
OpStore %_output %370
%371 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%372 = OpLoad %v2float %371
%373 = OpFAdd %v2float %361 %372
OpStore %coord %373
OpStore %coordSampled %373
%374 = OpLoad %v4float %106
OpStore %375 %374
OpStore %376 %373
%377 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %375 %376
%378 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%379 = OpLoad %v4float %378
%380 = OpCompositeExtract %float %379 1
%381 = OpVectorTimesScalar %v4float %377 %380
%382 = OpFAdd %v4float %370 %381
OpStore %_output %382
%383 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%384 = OpLoad %v2float %383
%385 = OpFAdd %v2float %373 %384
OpStore %coord %385
OpStore %coordSampled %385
%386 = OpLoad %v4float %106
OpStore %387 %386
OpStore %388 %385
%389 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %387 %388
%390 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%391 = OpLoad %v4float %390
%392 = OpCompositeExtract %float %391 2
%393 = OpVectorTimesScalar %v4float %389 %392
%394 = OpFAdd %v4float %382 %393
OpStore %_output %394
%395 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%396 = OpLoad %v2float %395
%397 = OpFAdd %v2float %385 %396
OpStore %coord %397
OpStore %coordSampled %397
%398 = OpLoad %v4float %106
OpStore %399 %398
OpStore %400 %397
%401 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %399 %400
%402 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_5
%403 = OpLoad %v4float %402
%404 = OpCompositeExtract %float %403 3
%405 = OpVectorTimesScalar %v4float %401 %404
%406 = OpFAdd %v4float %394 %405
OpStore %_output %406
%407 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%408 = OpLoad %v2float %407
%409 = OpFAdd %v2float %397 %408
OpStore %coord %409
OpStore %coordSampled %409
%410 = OpLoad %v4float %106
OpStore %411 %410
OpStore %412 %409
%413 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %411 %412
%414 = OpAccessChain %_ptr_Uniform_v4float %6 %int_2 %int_6
%415 = OpLoad %v4float %414
%416 = OpCompositeExtract %float %415 0
%417 = OpVectorTimesScalar %v4float %413 %416
%418 = OpFAdd %v4float %406 %417
OpStore %_output %418
%419 = OpAccessChain %_ptr_Uniform_v2float %6 %int_1
%420 = OpLoad %v2float %419
%421 = OpFAdd %v2float %409 %420
OpStore %coord %421
%422 = OpLoad %v4float %106
%423 = OpFMul %v4float %418 %422
OpStore %_output %423
OpReturnValue %423
OpFunctionEnd
%main = OpFunction %void None %425
%426 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%431 = OpVariable %_ptr_Function_v4float Function
OpStore %outputColor_Stage0 %429
OpStore %outputCoverage_Stage0 %429
OpStore %431 %429
%432 = OpFunctionCall %v4float %GaussianConvolution_Stage1_c0_h4h4 %431
OpStore %output_Stage1 %432
%433 = OpFMul %v4float %432 %429
OpStore %sk_FragColor %433
OpReturn
OpFunctionEnd
