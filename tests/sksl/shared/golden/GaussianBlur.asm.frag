### Compilation failed:

error: SPIR-V validation error: UniformConstant id '20' is missing DescriptorSet decoration.
From Vulkan spec, section 14.5.2:
These variables must have DescriptorSet and Binding decorations specified
  %uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_22 UniformConstant

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
OpName %MatrixEffect_Stage1_c0_c0 "MatrixEffect_Stage1_c0_c0"
OpName %_0_TextureEffect_Stage1_c0_c0_c0 "_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_1_coords "_1_coords"
OpName %_2_inCoord "_2_inCoord"
OpName %_3_subsetCoord "_3_subsetCoord"
OpName %_4_clampedCoord "_4_clampedCoord"
OpName %_5_textureColor "_5_textureColor"
OpName %_6_snappedX "_6_snappedX"
OpName %main "main"
OpName %output_Stage1 "output_Stage1"
OpName %_7_GaussianConvolution_Stage1_c0 "_7_GaussianConvolution_Stage1_c0"
OpName %_8_output "_8_output"
OpName %_9_coord "_9_coord"
OpName %_10_coordSampled "_10_coordSampled"
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
OpDecorate %4 Binding 0
OpDecorate %4 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %68 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
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
%4 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%23 = OpTypeImage %float 2D 0 0 0 1 Unknown
%22 = OpTypeSampledImage %23
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_22 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%float_1 = OpConstant %float 1
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
%void = OpTypeVoid
%105 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%110 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%121 = OpConstantComposite %v2float %float_0 %float_0
%float_1_0 = OpConstant %float 1
%124 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%int_2 = OpConstant %int 2
%142 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%158 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%174 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%190 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%206 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%222 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%238 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%254 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%270 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%286 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%302 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%318 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%334 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%350 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%366 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%382 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%398 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%414 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%430 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%446 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%462 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%478 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%494 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%510 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_1_coords = OpVariable %_ptr_Function_v2float Function
%_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_6_snappedX = OpVariable %_ptr_Function_float Function
%35 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%37 = OpLoad %mat3v3float %35
%38 = OpLoad %v2float %30
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%42 = OpCompositeConstruct %v3float %39 %40 %float_1
%43 = OpMatrixTimesVector %v3float %37 %42
%44 = OpVectorShuffle %v2float %43 %43 0 1
OpStore %_1_coords %44
%46 = OpLoad %v2float %_1_coords
OpStore %_2_inCoord %46
%47 = OpLoad %v2float %_2_inCoord
%49 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%51 = OpLoad %v4float %49
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpFMul %v2float %47 %52
OpStore %_2_inCoord %53
%55 = OpLoad %v2float %_2_inCoord
%56 = OpCompositeExtract %float %55 0
%57 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_0
OpStore %57 %56
%60 = OpLoad %v2float %_2_inCoord
%61 = OpCompositeExtract %float %60 1
%62 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_1
OpStore %62 %61
%65 = OpLoad %v2float %_3_subsetCoord
OpStore %_4_clampedCoord %65
%68 = OpLoad %22 %uTextureSampler_0_Stage1
%69 = OpLoad %v2float %_4_clampedCoord
%70 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%71 = OpLoad %v4float %70
%72 = OpVectorShuffle %v2float %71 %71 2 3
%73 = OpFMul %v2float %69 %72
%67 = OpImageSampleImplicitLod %v4float %68 %73
OpStore %_5_textureColor %67
%76 = OpLoad %v2float %_2_inCoord
%77 = OpCompositeExtract %float %76 0
%79 = OpFAdd %float %77 %float_0_00100000005
%75 = OpExtInst %float %1 Floor %79
%81 = OpFAdd %float %75 %float_0_5
OpStore %_6_snappedX %81
%83 = OpLoad %float %_6_snappedX
%85 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdLessThan %bool %83 %87
OpSelectionMerge %90 None
OpBranchConditional %88 %90 %89
%89 = OpLabel
%91 = OpLoad %float %_6_snappedX
%92 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%93 = OpLoad %v4float %92
%94 = OpCompositeExtract %float %93 2
%95 = OpFOrdGreaterThan %bool %91 %94
OpBranch %90
%90 = OpLabel
%96 = OpPhi %bool %true %31 %95 %89
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%101 = OpLoad %v4float %100
OpStore %_5_textureColor %101
OpBranch %98
%98 = OpLabel
%102 = OpLoad %v4float %_5_textureColor
OpStore %_0_TextureEffect_Stage1_c0_c0_c0 %102
%103 = OpLoad %v4float %_0_TextureEffect_Stage1_c0_c0_c0
OpReturnValue %103
OpFunctionEnd
%main = OpFunction %void None %105
%106 = OpLabel
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_7_GaussianConvolution_Stage1_c0 = OpVariable %_ptr_Function_v4float Function
%_8_output = OpVariable %_ptr_Function_v4float Function
%_9_coord = OpVariable %_ptr_Function_v2float Function
%_10_coordSampled = OpVariable %_ptr_Function_v2float Function
%126 = OpVariable %_ptr_Function_v4float Function
%128 = OpVariable %_ptr_Function_v2float Function
%143 = OpVariable %_ptr_Function_v4float Function
%145 = OpVariable %_ptr_Function_v2float Function
%159 = OpVariable %_ptr_Function_v4float Function
%161 = OpVariable %_ptr_Function_v2float Function
%175 = OpVariable %_ptr_Function_v4float Function
%177 = OpVariable %_ptr_Function_v2float Function
%191 = OpVariable %_ptr_Function_v4float Function
%193 = OpVariable %_ptr_Function_v2float Function
%207 = OpVariable %_ptr_Function_v4float Function
%209 = OpVariable %_ptr_Function_v2float Function
%223 = OpVariable %_ptr_Function_v4float Function
%225 = OpVariable %_ptr_Function_v2float Function
%239 = OpVariable %_ptr_Function_v4float Function
%241 = OpVariable %_ptr_Function_v2float Function
%255 = OpVariable %_ptr_Function_v4float Function
%257 = OpVariable %_ptr_Function_v2float Function
%271 = OpVariable %_ptr_Function_v4float Function
%273 = OpVariable %_ptr_Function_v2float Function
%287 = OpVariable %_ptr_Function_v4float Function
%289 = OpVariable %_ptr_Function_v2float Function
%303 = OpVariable %_ptr_Function_v4float Function
%305 = OpVariable %_ptr_Function_v2float Function
%319 = OpVariable %_ptr_Function_v4float Function
%321 = OpVariable %_ptr_Function_v2float Function
%335 = OpVariable %_ptr_Function_v4float Function
%337 = OpVariable %_ptr_Function_v2float Function
%351 = OpVariable %_ptr_Function_v4float Function
%353 = OpVariable %_ptr_Function_v2float Function
%367 = OpVariable %_ptr_Function_v4float Function
%369 = OpVariable %_ptr_Function_v2float Function
%383 = OpVariable %_ptr_Function_v4float Function
%385 = OpVariable %_ptr_Function_v2float Function
%399 = OpVariable %_ptr_Function_v4float Function
%401 = OpVariable %_ptr_Function_v2float Function
%415 = OpVariable %_ptr_Function_v4float Function
%417 = OpVariable %_ptr_Function_v2float Function
%431 = OpVariable %_ptr_Function_v4float Function
%433 = OpVariable %_ptr_Function_v2float Function
%447 = OpVariable %_ptr_Function_v4float Function
%449 = OpVariable %_ptr_Function_v2float Function
%463 = OpVariable %_ptr_Function_v4float Function
%465 = OpVariable %_ptr_Function_v2float Function
%479 = OpVariable %_ptr_Function_v4float Function
%481 = OpVariable %_ptr_Function_v2float Function
%495 = OpVariable %_ptr_Function_v4float Function
%497 = OpVariable %_ptr_Function_v2float Function
%511 = OpVariable %_ptr_Function_v4float Function
%513 = OpVariable %_ptr_Function_v2float Function
OpStore %_8_output %110
%113 = OpLoad %v2float %vLocalCoord_Stage0
%115 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%117 = OpLoad %v2float %115
%118 = OpVectorTimesScalar %v2float %117 %float_12
%119 = OpFSub %v2float %113 %118
OpStore %_9_coord %119
OpStore %_10_coordSampled %121
%122 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %122
%123 = OpLoad %v4float %_8_output
OpStore %126 %124
%127 = OpLoad %v2float %_10_coordSampled
OpStore %128 %127
%129 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %126 %128
%131 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 0
%134 = OpVectorTimesScalar %v4float %129 %133
%135 = OpFAdd %v4float %123 %134
OpStore %_8_output %135
%136 = OpLoad %v2float %_9_coord
%137 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%138 = OpLoad %v2float %137
%139 = OpFAdd %v2float %136 %138
OpStore %_9_coord %139
%140 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %140
%141 = OpLoad %v4float %_8_output
OpStore %143 %142
%144 = OpLoad %v2float %_10_coordSampled
OpStore %145 %144
%146 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %143 %145
%147 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%148 = OpLoad %v4float %147
%149 = OpCompositeExtract %float %148 1
%150 = OpVectorTimesScalar %v4float %146 %149
%151 = OpFAdd %v4float %141 %150
OpStore %_8_output %151
%152 = OpLoad %v2float %_9_coord
%153 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%154 = OpLoad %v2float %153
%155 = OpFAdd %v2float %152 %154
OpStore %_9_coord %155
%156 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %156
%157 = OpLoad %v4float %_8_output
OpStore %159 %158
%160 = OpLoad %v2float %_10_coordSampled
OpStore %161 %160
%162 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %159 %161
%163 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 2
%166 = OpVectorTimesScalar %v4float %162 %165
%167 = OpFAdd %v4float %157 %166
OpStore %_8_output %167
%168 = OpLoad %v2float %_9_coord
%169 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%170 = OpLoad %v2float %169
%171 = OpFAdd %v2float %168 %170
OpStore %_9_coord %171
%172 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %172
%173 = OpLoad %v4float %_8_output
OpStore %175 %174
%176 = OpLoad %v2float %_10_coordSampled
OpStore %177 %176
%178 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %175 %177
%179 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%180 = OpLoad %v4float %179
%181 = OpCompositeExtract %float %180 3
%182 = OpVectorTimesScalar %v4float %178 %181
%183 = OpFAdd %v4float %173 %182
OpStore %_8_output %183
%184 = OpLoad %v2float %_9_coord
%185 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%186 = OpLoad %v2float %185
%187 = OpFAdd %v2float %184 %186
OpStore %_9_coord %187
%188 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %188
%189 = OpLoad %v4float %_8_output
OpStore %191 %190
%192 = OpLoad %v2float %_10_coordSampled
OpStore %193 %192
%194 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %191 %193
%195 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 0
%198 = OpVectorTimesScalar %v4float %194 %197
%199 = OpFAdd %v4float %189 %198
OpStore %_8_output %199
%200 = OpLoad %v2float %_9_coord
%201 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%202 = OpLoad %v2float %201
%203 = OpFAdd %v2float %200 %202
OpStore %_9_coord %203
%204 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %204
%205 = OpLoad %v4float %_8_output
OpStore %207 %206
%208 = OpLoad %v2float %_10_coordSampled
OpStore %209 %208
%210 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %207 %209
%211 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%212 = OpLoad %v4float %211
%213 = OpCompositeExtract %float %212 1
%214 = OpVectorTimesScalar %v4float %210 %213
%215 = OpFAdd %v4float %205 %214
OpStore %_8_output %215
%216 = OpLoad %v2float %_9_coord
%217 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%218 = OpLoad %v2float %217
%219 = OpFAdd %v2float %216 %218
OpStore %_9_coord %219
%220 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %220
%221 = OpLoad %v4float %_8_output
OpStore %223 %222
%224 = OpLoad %v2float %_10_coordSampled
OpStore %225 %224
%226 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %223 %225
%227 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%228 = OpLoad %v4float %227
%229 = OpCompositeExtract %float %228 2
%230 = OpVectorTimesScalar %v4float %226 %229
%231 = OpFAdd %v4float %221 %230
OpStore %_8_output %231
%232 = OpLoad %v2float %_9_coord
%233 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%234 = OpLoad %v2float %233
%235 = OpFAdd %v2float %232 %234
OpStore %_9_coord %235
%236 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %236
%237 = OpLoad %v4float %_8_output
OpStore %239 %238
%240 = OpLoad %v2float %_10_coordSampled
OpStore %241 %240
%242 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %239 %241
%243 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%244 = OpLoad %v4float %243
%245 = OpCompositeExtract %float %244 3
%246 = OpVectorTimesScalar %v4float %242 %245
%247 = OpFAdd %v4float %237 %246
OpStore %_8_output %247
%248 = OpLoad %v2float %_9_coord
%249 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%250 = OpLoad %v2float %249
%251 = OpFAdd %v2float %248 %250
OpStore %_9_coord %251
%252 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %252
%253 = OpLoad %v4float %_8_output
OpStore %255 %254
%256 = OpLoad %v2float %_10_coordSampled
OpStore %257 %256
%258 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %255 %257
%259 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%260 = OpLoad %v4float %259
%261 = OpCompositeExtract %float %260 0
%262 = OpVectorTimesScalar %v4float %258 %261
%263 = OpFAdd %v4float %253 %262
OpStore %_8_output %263
%264 = OpLoad %v2float %_9_coord
%265 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%266 = OpLoad %v2float %265
%267 = OpFAdd %v2float %264 %266
OpStore %_9_coord %267
%268 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %268
%269 = OpLoad %v4float %_8_output
OpStore %271 %270
%272 = OpLoad %v2float %_10_coordSampled
OpStore %273 %272
%274 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %271 %273
%275 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%276 = OpLoad %v4float %275
%277 = OpCompositeExtract %float %276 1
%278 = OpVectorTimesScalar %v4float %274 %277
%279 = OpFAdd %v4float %269 %278
OpStore %_8_output %279
%280 = OpLoad %v2float %_9_coord
%281 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%282 = OpLoad %v2float %281
%283 = OpFAdd %v2float %280 %282
OpStore %_9_coord %283
%284 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %284
%285 = OpLoad %v4float %_8_output
OpStore %287 %286
%288 = OpLoad %v2float %_10_coordSampled
OpStore %289 %288
%290 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %287 %289
%291 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%292 = OpLoad %v4float %291
%293 = OpCompositeExtract %float %292 2
%294 = OpVectorTimesScalar %v4float %290 %293
%295 = OpFAdd %v4float %285 %294
OpStore %_8_output %295
%296 = OpLoad %v2float %_9_coord
%297 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%298 = OpLoad %v2float %297
%299 = OpFAdd %v2float %296 %298
OpStore %_9_coord %299
%300 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %300
%301 = OpLoad %v4float %_8_output
OpStore %303 %302
%304 = OpLoad %v2float %_10_coordSampled
OpStore %305 %304
%306 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %303 %305
%307 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%308 = OpLoad %v4float %307
%309 = OpCompositeExtract %float %308 3
%310 = OpVectorTimesScalar %v4float %306 %309
%311 = OpFAdd %v4float %301 %310
OpStore %_8_output %311
%312 = OpLoad %v2float %_9_coord
%313 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%314 = OpLoad %v2float %313
%315 = OpFAdd %v2float %312 %314
OpStore %_9_coord %315
%316 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %316
%317 = OpLoad %v4float %_8_output
OpStore %319 %318
%320 = OpLoad %v2float %_10_coordSampled
OpStore %321 %320
%322 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %319 %321
%323 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%324 = OpLoad %v4float %323
%325 = OpCompositeExtract %float %324 0
%326 = OpVectorTimesScalar %v4float %322 %325
%327 = OpFAdd %v4float %317 %326
OpStore %_8_output %327
%328 = OpLoad %v2float %_9_coord
%329 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%330 = OpLoad %v2float %329
%331 = OpFAdd %v2float %328 %330
OpStore %_9_coord %331
%332 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %332
%333 = OpLoad %v4float %_8_output
OpStore %335 %334
%336 = OpLoad %v2float %_10_coordSampled
OpStore %337 %336
%338 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %335 %337
%339 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%340 = OpLoad %v4float %339
%341 = OpCompositeExtract %float %340 1
%342 = OpVectorTimesScalar %v4float %338 %341
%343 = OpFAdd %v4float %333 %342
OpStore %_8_output %343
%344 = OpLoad %v2float %_9_coord
%345 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%346 = OpLoad %v2float %345
%347 = OpFAdd %v2float %344 %346
OpStore %_9_coord %347
%348 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %348
%349 = OpLoad %v4float %_8_output
OpStore %351 %350
%352 = OpLoad %v2float %_10_coordSampled
OpStore %353 %352
%354 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %351 %353
%355 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%356 = OpLoad %v4float %355
%357 = OpCompositeExtract %float %356 2
%358 = OpVectorTimesScalar %v4float %354 %357
%359 = OpFAdd %v4float %349 %358
OpStore %_8_output %359
%360 = OpLoad %v2float %_9_coord
%361 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%362 = OpLoad %v2float %361
%363 = OpFAdd %v2float %360 %362
OpStore %_9_coord %363
%364 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %364
%365 = OpLoad %v4float %_8_output
OpStore %367 %366
%368 = OpLoad %v2float %_10_coordSampled
OpStore %369 %368
%370 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %367 %369
%371 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%372 = OpLoad %v4float %371
%373 = OpCompositeExtract %float %372 3
%374 = OpVectorTimesScalar %v4float %370 %373
%375 = OpFAdd %v4float %365 %374
OpStore %_8_output %375
%376 = OpLoad %v2float %_9_coord
%377 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%378 = OpLoad %v2float %377
%379 = OpFAdd %v2float %376 %378
OpStore %_9_coord %379
%380 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %380
%381 = OpLoad %v4float %_8_output
OpStore %383 %382
%384 = OpLoad %v2float %_10_coordSampled
OpStore %385 %384
%386 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %383 %385
%387 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%388 = OpLoad %v4float %387
%389 = OpCompositeExtract %float %388 0
%390 = OpVectorTimesScalar %v4float %386 %389
%391 = OpFAdd %v4float %381 %390
OpStore %_8_output %391
%392 = OpLoad %v2float %_9_coord
%393 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%394 = OpLoad %v2float %393
%395 = OpFAdd %v2float %392 %394
OpStore %_9_coord %395
%396 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %396
%397 = OpLoad %v4float %_8_output
OpStore %399 %398
%400 = OpLoad %v2float %_10_coordSampled
OpStore %401 %400
%402 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %399 %401
%403 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%404 = OpLoad %v4float %403
%405 = OpCompositeExtract %float %404 1
%406 = OpVectorTimesScalar %v4float %402 %405
%407 = OpFAdd %v4float %397 %406
OpStore %_8_output %407
%408 = OpLoad %v2float %_9_coord
%409 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%410 = OpLoad %v2float %409
%411 = OpFAdd %v2float %408 %410
OpStore %_9_coord %411
%412 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %412
%413 = OpLoad %v4float %_8_output
OpStore %415 %414
%416 = OpLoad %v2float %_10_coordSampled
OpStore %417 %416
%418 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %415 %417
%419 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%420 = OpLoad %v4float %419
%421 = OpCompositeExtract %float %420 2
%422 = OpVectorTimesScalar %v4float %418 %421
%423 = OpFAdd %v4float %413 %422
OpStore %_8_output %423
%424 = OpLoad %v2float %_9_coord
%425 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%426 = OpLoad %v2float %425
%427 = OpFAdd %v2float %424 %426
OpStore %_9_coord %427
%428 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %428
%429 = OpLoad %v4float %_8_output
OpStore %431 %430
%432 = OpLoad %v2float %_10_coordSampled
OpStore %433 %432
%434 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %431 %433
%435 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%436 = OpLoad %v4float %435
%437 = OpCompositeExtract %float %436 3
%438 = OpVectorTimesScalar %v4float %434 %437
%439 = OpFAdd %v4float %429 %438
OpStore %_8_output %439
%440 = OpLoad %v2float %_9_coord
%441 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%442 = OpLoad %v2float %441
%443 = OpFAdd %v2float %440 %442
OpStore %_9_coord %443
%444 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %444
%445 = OpLoad %v4float %_8_output
OpStore %447 %446
%448 = OpLoad %v2float %_10_coordSampled
OpStore %449 %448
%450 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %447 %449
%451 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%452 = OpLoad %v4float %451
%453 = OpCompositeExtract %float %452 0
%454 = OpVectorTimesScalar %v4float %450 %453
%455 = OpFAdd %v4float %445 %454
OpStore %_8_output %455
%456 = OpLoad %v2float %_9_coord
%457 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%458 = OpLoad %v2float %457
%459 = OpFAdd %v2float %456 %458
OpStore %_9_coord %459
%460 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %460
%461 = OpLoad %v4float %_8_output
OpStore %463 %462
%464 = OpLoad %v2float %_10_coordSampled
OpStore %465 %464
%466 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %463 %465
%467 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%468 = OpLoad %v4float %467
%469 = OpCompositeExtract %float %468 1
%470 = OpVectorTimesScalar %v4float %466 %469
%471 = OpFAdd %v4float %461 %470
OpStore %_8_output %471
%472 = OpLoad %v2float %_9_coord
%473 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%474 = OpLoad %v2float %473
%475 = OpFAdd %v2float %472 %474
OpStore %_9_coord %475
%476 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %476
%477 = OpLoad %v4float %_8_output
OpStore %479 %478
%480 = OpLoad %v2float %_10_coordSampled
OpStore %481 %480
%482 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %479 %481
%483 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%484 = OpLoad %v4float %483
%485 = OpCompositeExtract %float %484 2
%486 = OpVectorTimesScalar %v4float %482 %485
%487 = OpFAdd %v4float %477 %486
OpStore %_8_output %487
%488 = OpLoad %v2float %_9_coord
%489 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%490 = OpLoad %v2float %489
%491 = OpFAdd %v2float %488 %490
OpStore %_9_coord %491
%492 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %492
%493 = OpLoad %v4float %_8_output
OpStore %495 %494
%496 = OpLoad %v2float %_10_coordSampled
OpStore %497 %496
%498 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %495 %497
%499 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%500 = OpLoad %v4float %499
%501 = OpCompositeExtract %float %500 3
%502 = OpVectorTimesScalar %v4float %498 %501
%503 = OpFAdd %v4float %493 %502
OpStore %_8_output %503
%504 = OpLoad %v2float %_9_coord
%505 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%506 = OpLoad %v2float %505
%507 = OpFAdd %v2float %504 %506
OpStore %_9_coord %507
%508 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %508
%509 = OpLoad %v4float %_8_output
OpStore %511 %510
%512 = OpLoad %v2float %_10_coordSampled
OpStore %513 %512
%514 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %511 %513
%515 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%516 = OpLoad %v4float %515
%517 = OpCompositeExtract %float %516 0
%518 = OpVectorTimesScalar %v4float %514 %517
%519 = OpFAdd %v4float %509 %518
OpStore %_8_output %519
%520 = OpLoad %v2float %_9_coord
%521 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%522 = OpLoad %v2float %521
%523 = OpFAdd %v2float %520 %522
OpStore %_9_coord %523
%524 = OpLoad %v4float %_8_output
OpStore %_7_GaussianConvolution_Stage1_c0 %524
%525 = OpLoad %v4float %_7_GaussianConvolution_Stage1_c0
OpStore %output_Stage1 %525
%526 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %526
OpReturn
OpFunctionEnd

1 error
