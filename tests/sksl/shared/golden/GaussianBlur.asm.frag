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
OpName %_1_coords "_1_coords"
OpName %_2_inCoord "_2_inCoord"
OpName %_3_subsetCoord "_3_subsetCoord"
OpName %_4_clampedCoord "_4_clampedCoord"
OpName %_5_textureColor "_5_textureColor"
OpName %_6_snappedX "_6_snappedX"
OpName %main "main"
OpName %output_Stage1 "output_Stage1"
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
OpDecorate %67 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
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
%103 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%107 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%118 = OpConstantComposite %v2float %float_0 %float_0
%float_1_0 = OpConstant %float 1
%121 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%int_2 = OpConstant %int 2
%139 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%155 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%171 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%187 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%203 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%219 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%235 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%251 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%267 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%283 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%299 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%315 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%331 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%347 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%363 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%379 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%395 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%411 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%427 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%443 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%459 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%475 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%491 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%507 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_1_coords = OpVariable %_ptr_Function_v2float Function
%_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_6_snappedX = OpVariable %_ptr_Function_float Function
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%36 = OpLoad %mat3v3float %34
%37 = OpLoad %v2float %30
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpCompositeConstruct %v3float %38 %39 %float_1
%42 = OpMatrixTimesVector %v3float %36 %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
OpStore %_1_coords %43
%45 = OpLoad %v2float %_1_coords
OpStore %_2_inCoord %45
%46 = OpLoad %v2float %_2_inCoord
%48 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%50 = OpLoad %v4float %48
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFMul %v2float %46 %51
OpStore %_2_inCoord %52
%54 = OpLoad %v2float %_2_inCoord
%55 = OpCompositeExtract %float %54 0
%56 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_0
OpStore %56 %55
%59 = OpLoad %v2float %_2_inCoord
%60 = OpCompositeExtract %float %59 1
%61 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_1
OpStore %61 %60
%64 = OpLoad %v2float %_3_subsetCoord
OpStore %_4_clampedCoord %64
%67 = OpLoad %22 %uTextureSampler_0_Stage1
%68 = OpLoad %v2float %_4_clampedCoord
%69 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 2 3
%72 = OpFMul %v2float %68 %71
%66 = OpImageSampleImplicitLod %v4float %67 %72
OpStore %_5_textureColor %66
%75 = OpLoad %v2float %_2_inCoord
%76 = OpCompositeExtract %float %75 0
%78 = OpFAdd %float %76 %float_0_00100000005
%74 = OpExtInst %float %1 Floor %78
%80 = OpFAdd %float %74 %float_0_5
OpStore %_6_snappedX %80
%82 = OpLoad %float %_6_snappedX
%84 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdLessThan %bool %82 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %89 %88
%88 = OpLabel
%90 = OpLoad %float %_6_snappedX
%91 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%92 = OpLoad %v4float %91
%93 = OpCompositeExtract %float %92 2
%94 = OpFOrdGreaterThan %bool %90 %93
OpBranch %89
%89 = OpLabel
%95 = OpPhi %bool %true %31 %94 %88
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%100 = OpLoad %v4float %99
OpStore %_5_textureColor %100
OpBranch %97
%97 = OpLabel
%101 = OpLoad %v4float %_5_textureColor
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %void None %103
%104 = OpLabel
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_8_output = OpVariable %_ptr_Function_v4float Function
%_9_coord = OpVariable %_ptr_Function_v2float Function
%_10_coordSampled = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v4float Function
%125 = OpVariable %_ptr_Function_v2float Function
%140 = OpVariable %_ptr_Function_v4float Function
%142 = OpVariable %_ptr_Function_v2float Function
%156 = OpVariable %_ptr_Function_v4float Function
%158 = OpVariable %_ptr_Function_v2float Function
%172 = OpVariable %_ptr_Function_v4float Function
%174 = OpVariable %_ptr_Function_v2float Function
%188 = OpVariable %_ptr_Function_v4float Function
%190 = OpVariable %_ptr_Function_v2float Function
%204 = OpVariable %_ptr_Function_v4float Function
%206 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v4float Function
%222 = OpVariable %_ptr_Function_v2float Function
%236 = OpVariable %_ptr_Function_v4float Function
%238 = OpVariable %_ptr_Function_v2float Function
%252 = OpVariable %_ptr_Function_v4float Function
%254 = OpVariable %_ptr_Function_v2float Function
%268 = OpVariable %_ptr_Function_v4float Function
%270 = OpVariable %_ptr_Function_v2float Function
%284 = OpVariable %_ptr_Function_v4float Function
%286 = OpVariable %_ptr_Function_v2float Function
%300 = OpVariable %_ptr_Function_v4float Function
%302 = OpVariable %_ptr_Function_v2float Function
%316 = OpVariable %_ptr_Function_v4float Function
%318 = OpVariable %_ptr_Function_v2float Function
%332 = OpVariable %_ptr_Function_v4float Function
%334 = OpVariable %_ptr_Function_v2float Function
%348 = OpVariable %_ptr_Function_v4float Function
%350 = OpVariable %_ptr_Function_v2float Function
%364 = OpVariable %_ptr_Function_v4float Function
%366 = OpVariable %_ptr_Function_v2float Function
%380 = OpVariable %_ptr_Function_v4float Function
%382 = OpVariable %_ptr_Function_v2float Function
%396 = OpVariable %_ptr_Function_v4float Function
%398 = OpVariable %_ptr_Function_v2float Function
%412 = OpVariable %_ptr_Function_v4float Function
%414 = OpVariable %_ptr_Function_v2float Function
%428 = OpVariable %_ptr_Function_v4float Function
%430 = OpVariable %_ptr_Function_v2float Function
%444 = OpVariable %_ptr_Function_v4float Function
%446 = OpVariable %_ptr_Function_v2float Function
%460 = OpVariable %_ptr_Function_v4float Function
%462 = OpVariable %_ptr_Function_v2float Function
%476 = OpVariable %_ptr_Function_v4float Function
%478 = OpVariable %_ptr_Function_v2float Function
%492 = OpVariable %_ptr_Function_v4float Function
%494 = OpVariable %_ptr_Function_v2float Function
%508 = OpVariable %_ptr_Function_v4float Function
%510 = OpVariable %_ptr_Function_v2float Function
OpStore %_8_output %107
%110 = OpLoad %v2float %vLocalCoord_Stage0
%112 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%114 = OpLoad %v2float %112
%115 = OpVectorTimesScalar %v2float %114 %float_12
%116 = OpFSub %v2float %110 %115
OpStore %_9_coord %116
OpStore %_10_coordSampled %118
%119 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %119
%120 = OpLoad %v4float %_8_output
OpStore %123 %121
%124 = OpLoad %v2float %_10_coordSampled
OpStore %125 %124
%126 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %123 %125
%128 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 0
%131 = OpVectorTimesScalar %v4float %126 %130
%132 = OpFAdd %v4float %120 %131
OpStore %_8_output %132
%133 = OpLoad %v2float %_9_coord
%134 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%135 = OpLoad %v2float %134
%136 = OpFAdd %v2float %133 %135
OpStore %_9_coord %136
%137 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %137
%138 = OpLoad %v4float %_8_output
OpStore %140 %139
%141 = OpLoad %v2float %_10_coordSampled
OpStore %142 %141
%143 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %140 %142
%144 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 1
%147 = OpVectorTimesScalar %v4float %143 %146
%148 = OpFAdd %v4float %138 %147
OpStore %_8_output %148
%149 = OpLoad %v2float %_9_coord
%150 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%151 = OpLoad %v2float %150
%152 = OpFAdd %v2float %149 %151
OpStore %_9_coord %152
%153 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %153
%154 = OpLoad %v4float %_8_output
OpStore %156 %155
%157 = OpLoad %v2float %_10_coordSampled
OpStore %158 %157
%159 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %156 %158
%160 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 2
%163 = OpVectorTimesScalar %v4float %159 %162
%164 = OpFAdd %v4float %154 %163
OpStore %_8_output %164
%165 = OpLoad %v2float %_9_coord
%166 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%167 = OpLoad %v2float %166
%168 = OpFAdd %v2float %165 %167
OpStore %_9_coord %168
%169 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %169
%170 = OpLoad %v4float %_8_output
OpStore %172 %171
%173 = OpLoad %v2float %_10_coordSampled
OpStore %174 %173
%175 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %172 %174
%176 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 3
%179 = OpVectorTimesScalar %v4float %175 %178
%180 = OpFAdd %v4float %170 %179
OpStore %_8_output %180
%181 = OpLoad %v2float %_9_coord
%182 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%183 = OpLoad %v2float %182
%184 = OpFAdd %v2float %181 %183
OpStore %_9_coord %184
%185 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %185
%186 = OpLoad %v4float %_8_output
OpStore %188 %187
%189 = OpLoad %v2float %_10_coordSampled
OpStore %190 %189
%191 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %188 %190
%192 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
%195 = OpVectorTimesScalar %v4float %191 %194
%196 = OpFAdd %v4float %186 %195
OpStore %_8_output %196
%197 = OpLoad %v2float %_9_coord
%198 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%199 = OpLoad %v2float %198
%200 = OpFAdd %v2float %197 %199
OpStore %_9_coord %200
%201 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %201
%202 = OpLoad %v4float %_8_output
OpStore %204 %203
%205 = OpLoad %v2float %_10_coordSampled
OpStore %206 %205
%207 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %204 %206
%208 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%209 = OpLoad %v4float %208
%210 = OpCompositeExtract %float %209 1
%211 = OpVectorTimesScalar %v4float %207 %210
%212 = OpFAdd %v4float %202 %211
OpStore %_8_output %212
%213 = OpLoad %v2float %_9_coord
%214 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%215 = OpLoad %v2float %214
%216 = OpFAdd %v2float %213 %215
OpStore %_9_coord %216
%217 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %217
%218 = OpLoad %v4float %_8_output
OpStore %220 %219
%221 = OpLoad %v2float %_10_coordSampled
OpStore %222 %221
%223 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %220 %222
%224 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%225 = OpLoad %v4float %224
%226 = OpCompositeExtract %float %225 2
%227 = OpVectorTimesScalar %v4float %223 %226
%228 = OpFAdd %v4float %218 %227
OpStore %_8_output %228
%229 = OpLoad %v2float %_9_coord
%230 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%231 = OpLoad %v2float %230
%232 = OpFAdd %v2float %229 %231
OpStore %_9_coord %232
%233 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %233
%234 = OpLoad %v4float %_8_output
OpStore %236 %235
%237 = OpLoad %v2float %_10_coordSampled
OpStore %238 %237
%239 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %236 %238
%240 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%241 = OpLoad %v4float %240
%242 = OpCompositeExtract %float %241 3
%243 = OpVectorTimesScalar %v4float %239 %242
%244 = OpFAdd %v4float %234 %243
OpStore %_8_output %244
%245 = OpLoad %v2float %_9_coord
%246 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%247 = OpLoad %v2float %246
%248 = OpFAdd %v2float %245 %247
OpStore %_9_coord %248
%249 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %249
%250 = OpLoad %v4float %_8_output
OpStore %252 %251
%253 = OpLoad %v2float %_10_coordSampled
OpStore %254 %253
%255 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %252 %254
%256 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%257 = OpLoad %v4float %256
%258 = OpCompositeExtract %float %257 0
%259 = OpVectorTimesScalar %v4float %255 %258
%260 = OpFAdd %v4float %250 %259
OpStore %_8_output %260
%261 = OpLoad %v2float %_9_coord
%262 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%263 = OpLoad %v2float %262
%264 = OpFAdd %v2float %261 %263
OpStore %_9_coord %264
%265 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %265
%266 = OpLoad %v4float %_8_output
OpStore %268 %267
%269 = OpLoad %v2float %_10_coordSampled
OpStore %270 %269
%271 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %268 %270
%272 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%273 = OpLoad %v4float %272
%274 = OpCompositeExtract %float %273 1
%275 = OpVectorTimesScalar %v4float %271 %274
%276 = OpFAdd %v4float %266 %275
OpStore %_8_output %276
%277 = OpLoad %v2float %_9_coord
%278 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%279 = OpLoad %v2float %278
%280 = OpFAdd %v2float %277 %279
OpStore %_9_coord %280
%281 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %281
%282 = OpLoad %v4float %_8_output
OpStore %284 %283
%285 = OpLoad %v2float %_10_coordSampled
OpStore %286 %285
%287 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %284 %286
%288 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%289 = OpLoad %v4float %288
%290 = OpCompositeExtract %float %289 2
%291 = OpVectorTimesScalar %v4float %287 %290
%292 = OpFAdd %v4float %282 %291
OpStore %_8_output %292
%293 = OpLoad %v2float %_9_coord
%294 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%295 = OpLoad %v2float %294
%296 = OpFAdd %v2float %293 %295
OpStore %_9_coord %296
%297 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %297
%298 = OpLoad %v4float %_8_output
OpStore %300 %299
%301 = OpLoad %v2float %_10_coordSampled
OpStore %302 %301
%303 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %300 %302
%304 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%305 = OpLoad %v4float %304
%306 = OpCompositeExtract %float %305 3
%307 = OpVectorTimesScalar %v4float %303 %306
%308 = OpFAdd %v4float %298 %307
OpStore %_8_output %308
%309 = OpLoad %v2float %_9_coord
%310 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%311 = OpLoad %v2float %310
%312 = OpFAdd %v2float %309 %311
OpStore %_9_coord %312
%313 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %313
%314 = OpLoad %v4float %_8_output
OpStore %316 %315
%317 = OpLoad %v2float %_10_coordSampled
OpStore %318 %317
%319 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %316 %318
%320 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%321 = OpLoad %v4float %320
%322 = OpCompositeExtract %float %321 0
%323 = OpVectorTimesScalar %v4float %319 %322
%324 = OpFAdd %v4float %314 %323
OpStore %_8_output %324
%325 = OpLoad %v2float %_9_coord
%326 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%327 = OpLoad %v2float %326
%328 = OpFAdd %v2float %325 %327
OpStore %_9_coord %328
%329 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %329
%330 = OpLoad %v4float %_8_output
OpStore %332 %331
%333 = OpLoad %v2float %_10_coordSampled
OpStore %334 %333
%335 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %332 %334
%336 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%337 = OpLoad %v4float %336
%338 = OpCompositeExtract %float %337 1
%339 = OpVectorTimesScalar %v4float %335 %338
%340 = OpFAdd %v4float %330 %339
OpStore %_8_output %340
%341 = OpLoad %v2float %_9_coord
%342 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%343 = OpLoad %v2float %342
%344 = OpFAdd %v2float %341 %343
OpStore %_9_coord %344
%345 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %345
%346 = OpLoad %v4float %_8_output
OpStore %348 %347
%349 = OpLoad %v2float %_10_coordSampled
OpStore %350 %349
%351 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %348 %350
%352 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%353 = OpLoad %v4float %352
%354 = OpCompositeExtract %float %353 2
%355 = OpVectorTimesScalar %v4float %351 %354
%356 = OpFAdd %v4float %346 %355
OpStore %_8_output %356
%357 = OpLoad %v2float %_9_coord
%358 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%359 = OpLoad %v2float %358
%360 = OpFAdd %v2float %357 %359
OpStore %_9_coord %360
%361 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %361
%362 = OpLoad %v4float %_8_output
OpStore %364 %363
%365 = OpLoad %v2float %_10_coordSampled
OpStore %366 %365
%367 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %364 %366
%368 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%369 = OpLoad %v4float %368
%370 = OpCompositeExtract %float %369 3
%371 = OpVectorTimesScalar %v4float %367 %370
%372 = OpFAdd %v4float %362 %371
OpStore %_8_output %372
%373 = OpLoad %v2float %_9_coord
%374 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%375 = OpLoad %v2float %374
%376 = OpFAdd %v2float %373 %375
OpStore %_9_coord %376
%377 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %377
%378 = OpLoad %v4float %_8_output
OpStore %380 %379
%381 = OpLoad %v2float %_10_coordSampled
OpStore %382 %381
%383 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %380 %382
%384 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%385 = OpLoad %v4float %384
%386 = OpCompositeExtract %float %385 0
%387 = OpVectorTimesScalar %v4float %383 %386
%388 = OpFAdd %v4float %378 %387
OpStore %_8_output %388
%389 = OpLoad %v2float %_9_coord
%390 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%391 = OpLoad %v2float %390
%392 = OpFAdd %v2float %389 %391
OpStore %_9_coord %392
%393 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %393
%394 = OpLoad %v4float %_8_output
OpStore %396 %395
%397 = OpLoad %v2float %_10_coordSampled
OpStore %398 %397
%399 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %396 %398
%400 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%401 = OpLoad %v4float %400
%402 = OpCompositeExtract %float %401 1
%403 = OpVectorTimesScalar %v4float %399 %402
%404 = OpFAdd %v4float %394 %403
OpStore %_8_output %404
%405 = OpLoad %v2float %_9_coord
%406 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%407 = OpLoad %v2float %406
%408 = OpFAdd %v2float %405 %407
OpStore %_9_coord %408
%409 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %409
%410 = OpLoad %v4float %_8_output
OpStore %412 %411
%413 = OpLoad %v2float %_10_coordSampled
OpStore %414 %413
%415 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %412 %414
%416 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%417 = OpLoad %v4float %416
%418 = OpCompositeExtract %float %417 2
%419 = OpVectorTimesScalar %v4float %415 %418
%420 = OpFAdd %v4float %410 %419
OpStore %_8_output %420
%421 = OpLoad %v2float %_9_coord
%422 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%423 = OpLoad %v2float %422
%424 = OpFAdd %v2float %421 %423
OpStore %_9_coord %424
%425 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %425
%426 = OpLoad %v4float %_8_output
OpStore %428 %427
%429 = OpLoad %v2float %_10_coordSampled
OpStore %430 %429
%431 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %428 %430
%432 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%433 = OpLoad %v4float %432
%434 = OpCompositeExtract %float %433 3
%435 = OpVectorTimesScalar %v4float %431 %434
%436 = OpFAdd %v4float %426 %435
OpStore %_8_output %436
%437 = OpLoad %v2float %_9_coord
%438 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%439 = OpLoad %v2float %438
%440 = OpFAdd %v2float %437 %439
OpStore %_9_coord %440
%441 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %441
%442 = OpLoad %v4float %_8_output
OpStore %444 %443
%445 = OpLoad %v2float %_10_coordSampled
OpStore %446 %445
%447 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %444 %446
%448 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%449 = OpLoad %v4float %448
%450 = OpCompositeExtract %float %449 0
%451 = OpVectorTimesScalar %v4float %447 %450
%452 = OpFAdd %v4float %442 %451
OpStore %_8_output %452
%453 = OpLoad %v2float %_9_coord
%454 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%455 = OpLoad %v2float %454
%456 = OpFAdd %v2float %453 %455
OpStore %_9_coord %456
%457 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %457
%458 = OpLoad %v4float %_8_output
OpStore %460 %459
%461 = OpLoad %v2float %_10_coordSampled
OpStore %462 %461
%463 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %460 %462
%464 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%465 = OpLoad %v4float %464
%466 = OpCompositeExtract %float %465 1
%467 = OpVectorTimesScalar %v4float %463 %466
%468 = OpFAdd %v4float %458 %467
OpStore %_8_output %468
%469 = OpLoad %v2float %_9_coord
%470 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%471 = OpLoad %v2float %470
%472 = OpFAdd %v2float %469 %471
OpStore %_9_coord %472
%473 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %473
%474 = OpLoad %v4float %_8_output
OpStore %476 %475
%477 = OpLoad %v2float %_10_coordSampled
OpStore %478 %477
%479 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %476 %478
%480 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%481 = OpLoad %v4float %480
%482 = OpCompositeExtract %float %481 2
%483 = OpVectorTimesScalar %v4float %479 %482
%484 = OpFAdd %v4float %474 %483
OpStore %_8_output %484
%485 = OpLoad %v2float %_9_coord
%486 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%487 = OpLoad %v2float %486
%488 = OpFAdd %v2float %485 %487
OpStore %_9_coord %488
%489 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %489
%490 = OpLoad %v4float %_8_output
OpStore %492 %491
%493 = OpLoad %v2float %_10_coordSampled
OpStore %494 %493
%495 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %492 %494
%496 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%497 = OpLoad %v4float %496
%498 = OpCompositeExtract %float %497 3
%499 = OpVectorTimesScalar %v4float %495 %498
%500 = OpFAdd %v4float %490 %499
OpStore %_8_output %500
%501 = OpLoad %v2float %_9_coord
%502 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%503 = OpLoad %v2float %502
%504 = OpFAdd %v2float %501 %503
OpStore %_9_coord %504
%505 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %505
%506 = OpLoad %v4float %_8_output
OpStore %508 %507
%509 = OpLoad %v2float %_10_coordSampled
OpStore %510 %509
%511 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %508 %510
%512 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%513 = OpLoad %v4float %512
%514 = OpCompositeExtract %float %513 0
%515 = OpVectorTimesScalar %v4float %511 %514
%516 = OpFAdd %v4float %506 %515
OpStore %_8_output %516
%517 = OpLoad %v2float %_9_coord
%518 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%519 = OpLoad %v2float %518
%520 = OpFAdd %v2float %517 %519
OpStore %_9_coord %520
%521 = OpLoad %v4float %_8_output
OpStore %output_Stage1 %521
%522 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %522
OpReturn
OpFunctionEnd

1 error
