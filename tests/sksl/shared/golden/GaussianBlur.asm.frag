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
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
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
%121 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%138 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%154 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%170 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%186 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%202 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%218 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%234 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%250 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%266 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%282 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%298 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%314 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%330 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%346 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%362 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%378 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%394 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%410 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%426 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%442 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%458 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%474 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%490 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%506 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%122 = OpVariable %_ptr_Function_v4float Function
%124 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v4float Function
%141 = OpVariable %_ptr_Function_v2float Function
%155 = OpVariable %_ptr_Function_v4float Function
%157 = OpVariable %_ptr_Function_v2float Function
%171 = OpVariable %_ptr_Function_v4float Function
%173 = OpVariable %_ptr_Function_v2float Function
%187 = OpVariable %_ptr_Function_v4float Function
%189 = OpVariable %_ptr_Function_v2float Function
%203 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_v2float Function
%219 = OpVariable %_ptr_Function_v4float Function
%221 = OpVariable %_ptr_Function_v2float Function
%235 = OpVariable %_ptr_Function_v4float Function
%237 = OpVariable %_ptr_Function_v2float Function
%251 = OpVariable %_ptr_Function_v4float Function
%253 = OpVariable %_ptr_Function_v2float Function
%267 = OpVariable %_ptr_Function_v4float Function
%269 = OpVariable %_ptr_Function_v2float Function
%283 = OpVariable %_ptr_Function_v4float Function
%285 = OpVariable %_ptr_Function_v2float Function
%299 = OpVariable %_ptr_Function_v4float Function
%301 = OpVariable %_ptr_Function_v2float Function
%315 = OpVariable %_ptr_Function_v4float Function
%317 = OpVariable %_ptr_Function_v2float Function
%331 = OpVariable %_ptr_Function_v4float Function
%333 = OpVariable %_ptr_Function_v2float Function
%347 = OpVariable %_ptr_Function_v4float Function
%349 = OpVariable %_ptr_Function_v2float Function
%363 = OpVariable %_ptr_Function_v4float Function
%365 = OpVariable %_ptr_Function_v2float Function
%379 = OpVariable %_ptr_Function_v4float Function
%381 = OpVariable %_ptr_Function_v2float Function
%395 = OpVariable %_ptr_Function_v4float Function
%397 = OpVariable %_ptr_Function_v2float Function
%411 = OpVariable %_ptr_Function_v4float Function
%413 = OpVariable %_ptr_Function_v2float Function
%427 = OpVariable %_ptr_Function_v4float Function
%429 = OpVariable %_ptr_Function_v2float Function
%443 = OpVariable %_ptr_Function_v4float Function
%445 = OpVariable %_ptr_Function_v2float Function
%459 = OpVariable %_ptr_Function_v4float Function
%461 = OpVariable %_ptr_Function_v2float Function
%475 = OpVariable %_ptr_Function_v4float Function
%477 = OpVariable %_ptr_Function_v2float Function
%491 = OpVariable %_ptr_Function_v4float Function
%493 = OpVariable %_ptr_Function_v2float Function
%507 = OpVariable %_ptr_Function_v4float Function
%509 = OpVariable %_ptr_Function_v2float Function
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
OpStore %122 %121
%123 = OpLoad %v2float %_10_coordSampled
OpStore %124 %123
%125 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %122 %124
%127 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 0
%130 = OpVectorTimesScalar %v4float %125 %129
%131 = OpFAdd %v4float %120 %130
OpStore %_8_output %131
%132 = OpLoad %v2float %_9_coord
%133 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%134 = OpLoad %v2float %133
%135 = OpFAdd %v2float %132 %134
OpStore %_9_coord %135
%136 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %136
%137 = OpLoad %v4float %_8_output
OpStore %139 %138
%140 = OpLoad %v2float %_10_coordSampled
OpStore %141 %140
%142 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %139 %141
%143 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%144 = OpLoad %v4float %143
%145 = OpCompositeExtract %float %144 1
%146 = OpVectorTimesScalar %v4float %142 %145
%147 = OpFAdd %v4float %137 %146
OpStore %_8_output %147
%148 = OpLoad %v2float %_9_coord
%149 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%150 = OpLoad %v2float %149
%151 = OpFAdd %v2float %148 %150
OpStore %_9_coord %151
%152 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %152
%153 = OpLoad %v4float %_8_output
OpStore %155 %154
%156 = OpLoad %v2float %_10_coordSampled
OpStore %157 %156
%158 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %155 %157
%159 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%160 = OpLoad %v4float %159
%161 = OpCompositeExtract %float %160 2
%162 = OpVectorTimesScalar %v4float %158 %161
%163 = OpFAdd %v4float %153 %162
OpStore %_8_output %163
%164 = OpLoad %v2float %_9_coord
%165 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%166 = OpLoad %v2float %165
%167 = OpFAdd %v2float %164 %166
OpStore %_9_coord %167
%168 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %168
%169 = OpLoad %v4float %_8_output
OpStore %171 %170
%172 = OpLoad %v2float %_10_coordSampled
OpStore %173 %172
%174 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %171 %173
%175 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%176 = OpLoad %v4float %175
%177 = OpCompositeExtract %float %176 3
%178 = OpVectorTimesScalar %v4float %174 %177
%179 = OpFAdd %v4float %169 %178
OpStore %_8_output %179
%180 = OpLoad %v2float %_9_coord
%181 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%182 = OpLoad %v2float %181
%183 = OpFAdd %v2float %180 %182
OpStore %_9_coord %183
%184 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %184
%185 = OpLoad %v4float %_8_output
OpStore %187 %186
%188 = OpLoad %v2float %_10_coordSampled
OpStore %189 %188
%190 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %187 %189
%191 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%192 = OpLoad %v4float %191
%193 = OpCompositeExtract %float %192 0
%194 = OpVectorTimesScalar %v4float %190 %193
%195 = OpFAdd %v4float %185 %194
OpStore %_8_output %195
%196 = OpLoad %v2float %_9_coord
%197 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%198 = OpLoad %v2float %197
%199 = OpFAdd %v2float %196 %198
OpStore %_9_coord %199
%200 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %200
%201 = OpLoad %v4float %_8_output
OpStore %203 %202
%204 = OpLoad %v2float %_10_coordSampled
OpStore %205 %204
%206 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %203 %205
%207 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 1
%210 = OpVectorTimesScalar %v4float %206 %209
%211 = OpFAdd %v4float %201 %210
OpStore %_8_output %211
%212 = OpLoad %v2float %_9_coord
%213 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%214 = OpLoad %v2float %213
%215 = OpFAdd %v2float %212 %214
OpStore %_9_coord %215
%216 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %216
%217 = OpLoad %v4float %_8_output
OpStore %219 %218
%220 = OpLoad %v2float %_10_coordSampled
OpStore %221 %220
%222 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %219 %221
%223 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%224 = OpLoad %v4float %223
%225 = OpCompositeExtract %float %224 2
%226 = OpVectorTimesScalar %v4float %222 %225
%227 = OpFAdd %v4float %217 %226
OpStore %_8_output %227
%228 = OpLoad %v2float %_9_coord
%229 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%230 = OpLoad %v2float %229
%231 = OpFAdd %v2float %228 %230
OpStore %_9_coord %231
%232 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %232
%233 = OpLoad %v4float %_8_output
OpStore %235 %234
%236 = OpLoad %v2float %_10_coordSampled
OpStore %237 %236
%238 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %235 %237
%239 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%240 = OpLoad %v4float %239
%241 = OpCompositeExtract %float %240 3
%242 = OpVectorTimesScalar %v4float %238 %241
%243 = OpFAdd %v4float %233 %242
OpStore %_8_output %243
%244 = OpLoad %v2float %_9_coord
%245 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%246 = OpLoad %v2float %245
%247 = OpFAdd %v2float %244 %246
OpStore %_9_coord %247
%248 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %248
%249 = OpLoad %v4float %_8_output
OpStore %251 %250
%252 = OpLoad %v2float %_10_coordSampled
OpStore %253 %252
%254 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %251 %253
%255 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%256 = OpLoad %v4float %255
%257 = OpCompositeExtract %float %256 0
%258 = OpVectorTimesScalar %v4float %254 %257
%259 = OpFAdd %v4float %249 %258
OpStore %_8_output %259
%260 = OpLoad %v2float %_9_coord
%261 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%262 = OpLoad %v2float %261
%263 = OpFAdd %v2float %260 %262
OpStore %_9_coord %263
%264 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %264
%265 = OpLoad %v4float %_8_output
OpStore %267 %266
%268 = OpLoad %v2float %_10_coordSampled
OpStore %269 %268
%270 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %267 %269
%271 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%272 = OpLoad %v4float %271
%273 = OpCompositeExtract %float %272 1
%274 = OpVectorTimesScalar %v4float %270 %273
%275 = OpFAdd %v4float %265 %274
OpStore %_8_output %275
%276 = OpLoad %v2float %_9_coord
%277 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%278 = OpLoad %v2float %277
%279 = OpFAdd %v2float %276 %278
OpStore %_9_coord %279
%280 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %280
%281 = OpLoad %v4float %_8_output
OpStore %283 %282
%284 = OpLoad %v2float %_10_coordSampled
OpStore %285 %284
%286 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %283 %285
%287 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%288 = OpLoad %v4float %287
%289 = OpCompositeExtract %float %288 2
%290 = OpVectorTimesScalar %v4float %286 %289
%291 = OpFAdd %v4float %281 %290
OpStore %_8_output %291
%292 = OpLoad %v2float %_9_coord
%293 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%294 = OpLoad %v2float %293
%295 = OpFAdd %v2float %292 %294
OpStore %_9_coord %295
%296 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %296
%297 = OpLoad %v4float %_8_output
OpStore %299 %298
%300 = OpLoad %v2float %_10_coordSampled
OpStore %301 %300
%302 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %299 %301
%303 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%304 = OpLoad %v4float %303
%305 = OpCompositeExtract %float %304 3
%306 = OpVectorTimesScalar %v4float %302 %305
%307 = OpFAdd %v4float %297 %306
OpStore %_8_output %307
%308 = OpLoad %v2float %_9_coord
%309 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%310 = OpLoad %v2float %309
%311 = OpFAdd %v2float %308 %310
OpStore %_9_coord %311
%312 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %312
%313 = OpLoad %v4float %_8_output
OpStore %315 %314
%316 = OpLoad %v2float %_10_coordSampled
OpStore %317 %316
%318 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %315 %317
%319 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%320 = OpLoad %v4float %319
%321 = OpCompositeExtract %float %320 0
%322 = OpVectorTimesScalar %v4float %318 %321
%323 = OpFAdd %v4float %313 %322
OpStore %_8_output %323
%324 = OpLoad %v2float %_9_coord
%325 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%326 = OpLoad %v2float %325
%327 = OpFAdd %v2float %324 %326
OpStore %_9_coord %327
%328 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %328
%329 = OpLoad %v4float %_8_output
OpStore %331 %330
%332 = OpLoad %v2float %_10_coordSampled
OpStore %333 %332
%334 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %331 %333
%335 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%336 = OpLoad %v4float %335
%337 = OpCompositeExtract %float %336 1
%338 = OpVectorTimesScalar %v4float %334 %337
%339 = OpFAdd %v4float %329 %338
OpStore %_8_output %339
%340 = OpLoad %v2float %_9_coord
%341 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%342 = OpLoad %v2float %341
%343 = OpFAdd %v2float %340 %342
OpStore %_9_coord %343
%344 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %344
%345 = OpLoad %v4float %_8_output
OpStore %347 %346
%348 = OpLoad %v2float %_10_coordSampled
OpStore %349 %348
%350 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %347 %349
%351 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%352 = OpLoad %v4float %351
%353 = OpCompositeExtract %float %352 2
%354 = OpVectorTimesScalar %v4float %350 %353
%355 = OpFAdd %v4float %345 %354
OpStore %_8_output %355
%356 = OpLoad %v2float %_9_coord
%357 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%358 = OpLoad %v2float %357
%359 = OpFAdd %v2float %356 %358
OpStore %_9_coord %359
%360 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %360
%361 = OpLoad %v4float %_8_output
OpStore %363 %362
%364 = OpLoad %v2float %_10_coordSampled
OpStore %365 %364
%366 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %363 %365
%367 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%368 = OpLoad %v4float %367
%369 = OpCompositeExtract %float %368 3
%370 = OpVectorTimesScalar %v4float %366 %369
%371 = OpFAdd %v4float %361 %370
OpStore %_8_output %371
%372 = OpLoad %v2float %_9_coord
%373 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%374 = OpLoad %v2float %373
%375 = OpFAdd %v2float %372 %374
OpStore %_9_coord %375
%376 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %376
%377 = OpLoad %v4float %_8_output
OpStore %379 %378
%380 = OpLoad %v2float %_10_coordSampled
OpStore %381 %380
%382 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %379 %381
%383 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%384 = OpLoad %v4float %383
%385 = OpCompositeExtract %float %384 0
%386 = OpVectorTimesScalar %v4float %382 %385
%387 = OpFAdd %v4float %377 %386
OpStore %_8_output %387
%388 = OpLoad %v2float %_9_coord
%389 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%390 = OpLoad %v2float %389
%391 = OpFAdd %v2float %388 %390
OpStore %_9_coord %391
%392 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %392
%393 = OpLoad %v4float %_8_output
OpStore %395 %394
%396 = OpLoad %v2float %_10_coordSampled
OpStore %397 %396
%398 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %395 %397
%399 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%400 = OpLoad %v4float %399
%401 = OpCompositeExtract %float %400 1
%402 = OpVectorTimesScalar %v4float %398 %401
%403 = OpFAdd %v4float %393 %402
OpStore %_8_output %403
%404 = OpLoad %v2float %_9_coord
%405 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%406 = OpLoad %v2float %405
%407 = OpFAdd %v2float %404 %406
OpStore %_9_coord %407
%408 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %408
%409 = OpLoad %v4float %_8_output
OpStore %411 %410
%412 = OpLoad %v2float %_10_coordSampled
OpStore %413 %412
%414 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %411 %413
%415 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%416 = OpLoad %v4float %415
%417 = OpCompositeExtract %float %416 2
%418 = OpVectorTimesScalar %v4float %414 %417
%419 = OpFAdd %v4float %409 %418
OpStore %_8_output %419
%420 = OpLoad %v2float %_9_coord
%421 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%422 = OpLoad %v2float %421
%423 = OpFAdd %v2float %420 %422
OpStore %_9_coord %423
%424 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %424
%425 = OpLoad %v4float %_8_output
OpStore %427 %426
%428 = OpLoad %v2float %_10_coordSampled
OpStore %429 %428
%430 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %427 %429
%431 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%432 = OpLoad %v4float %431
%433 = OpCompositeExtract %float %432 3
%434 = OpVectorTimesScalar %v4float %430 %433
%435 = OpFAdd %v4float %425 %434
OpStore %_8_output %435
%436 = OpLoad %v2float %_9_coord
%437 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%438 = OpLoad %v2float %437
%439 = OpFAdd %v2float %436 %438
OpStore %_9_coord %439
%440 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %440
%441 = OpLoad %v4float %_8_output
OpStore %443 %442
%444 = OpLoad %v2float %_10_coordSampled
OpStore %445 %444
%446 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %443 %445
%447 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%448 = OpLoad %v4float %447
%449 = OpCompositeExtract %float %448 0
%450 = OpVectorTimesScalar %v4float %446 %449
%451 = OpFAdd %v4float %441 %450
OpStore %_8_output %451
%452 = OpLoad %v2float %_9_coord
%453 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%454 = OpLoad %v2float %453
%455 = OpFAdd %v2float %452 %454
OpStore %_9_coord %455
%456 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %456
%457 = OpLoad %v4float %_8_output
OpStore %459 %458
%460 = OpLoad %v2float %_10_coordSampled
OpStore %461 %460
%462 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %459 %461
%463 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%464 = OpLoad %v4float %463
%465 = OpCompositeExtract %float %464 1
%466 = OpVectorTimesScalar %v4float %462 %465
%467 = OpFAdd %v4float %457 %466
OpStore %_8_output %467
%468 = OpLoad %v2float %_9_coord
%469 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%470 = OpLoad %v2float %469
%471 = OpFAdd %v2float %468 %470
OpStore %_9_coord %471
%472 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %472
%473 = OpLoad %v4float %_8_output
OpStore %475 %474
%476 = OpLoad %v2float %_10_coordSampled
OpStore %477 %476
%478 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %475 %477
%479 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%480 = OpLoad %v4float %479
%481 = OpCompositeExtract %float %480 2
%482 = OpVectorTimesScalar %v4float %478 %481
%483 = OpFAdd %v4float %473 %482
OpStore %_8_output %483
%484 = OpLoad %v2float %_9_coord
%485 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%486 = OpLoad %v2float %485
%487 = OpFAdd %v2float %484 %486
OpStore %_9_coord %487
%488 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %488
%489 = OpLoad %v4float %_8_output
OpStore %491 %490
%492 = OpLoad %v2float %_10_coordSampled
OpStore %493 %492
%494 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %491 %493
%495 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%496 = OpLoad %v4float %495
%497 = OpCompositeExtract %float %496 3
%498 = OpVectorTimesScalar %v4float %494 %497
%499 = OpFAdd %v4float %489 %498
OpStore %_8_output %499
%500 = OpLoad %v2float %_9_coord
%501 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%502 = OpLoad %v2float %501
%503 = OpFAdd %v2float %500 %502
OpStore %_9_coord %503
%504 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %504
%505 = OpLoad %v4float %_8_output
OpStore %507 %506
%508 = OpLoad %v2float %_10_coordSampled
OpStore %509 %508
%510 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %507 %509
%511 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%512 = OpLoad %v4float %511
%513 = OpCompositeExtract %float %512 0
%514 = OpVectorTimesScalar %v4float %510 %513
%515 = OpFAdd %v4float %505 %514
OpStore %_8_output %515
%516 = OpLoad %v2float %_9_coord
%517 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%518 = OpLoad %v2float %517
%519 = OpFAdd %v2float %516 %518
OpStore %_9_coord %519
%520 = OpLoad %v4float %_8_output
OpStore %output_Stage1 %520
%521 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %521
OpReturn
OpFunctionEnd

1 error
