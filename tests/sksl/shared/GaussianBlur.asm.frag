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
OpName %_0_coords "_0_coords"
OpName %_1_inCoord "_1_inCoord"
OpName %_2_subsetCoord "_2_subsetCoord"
OpName %_3_clampedCoord "_3_clampedCoord"
OpName %_4_textureColor "_4_textureColor"
OpName %_5_snappedX "_5_snappedX"
OpName %main "main"
OpName %output_Stage1 "output_Stage1"
OpName %_6_output "_6_output"
OpName %_7_coord "_7_coord"
OpName %_8_coordSampled "_8_coordSampled"
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
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
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
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
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
%108 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%118 = OpConstantComposite %v2float %float_0 %float_0
%121 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_0_coords = OpVariable %_ptr_Function_v2float Function
%_1_inCoord = OpVariable %_ptr_Function_v2float Function
%_2_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_3_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_4_textureColor = OpVariable %_ptr_Function_v4float Function
%_5_snappedX = OpVariable %_ptr_Function_float Function
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%36 = OpLoad %mat3v3float %34
%37 = OpLoad %v2float %30
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpCompositeConstruct %v3float %38 %39 %float_1
%42 = OpMatrixTimesVector %v3float %36 %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
OpStore %_0_coords %43
%45 = OpLoad %v2float %_0_coords
OpStore %_1_inCoord %45
%46 = OpLoad %v2float %_1_inCoord
%48 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%50 = OpLoad %v4float %48
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFMul %v2float %46 %51
OpStore %_1_inCoord %52
%54 = OpLoad %v2float %_1_inCoord
%55 = OpCompositeExtract %float %54 0
%56 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_0
OpStore %56 %55
%59 = OpLoad %v2float %_1_inCoord
%60 = OpCompositeExtract %float %59 1
%61 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_1
OpStore %61 %60
%64 = OpLoad %v2float %_2_subsetCoord
OpStore %_3_clampedCoord %64
%67 = OpLoad %22 %uTextureSampler_0_Stage1
%68 = OpLoad %v2float %_3_clampedCoord
%69 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 2 3
%72 = OpFMul %v2float %68 %71
%66 = OpImageSampleImplicitLod %v4float %67 %72
OpStore %_4_textureColor %66
%75 = OpLoad %v2float %_1_inCoord
%76 = OpCompositeExtract %float %75 0
%78 = OpFAdd %float %76 %float_0_00100000005
%74 = OpExtInst %float %1 Floor %78
%80 = OpFAdd %float %74 %float_0_5
OpStore %_5_snappedX %80
%82 = OpLoad %float %_5_snappedX
%84 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdLessThan %bool %82 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %89 %88
%88 = OpLabel
%90 = OpLoad %float %_5_snappedX
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
OpStore %_4_textureColor %100
OpBranch %97
%97 = OpLabel
%101 = OpLoad %v4float %_4_textureColor
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %void None %103
%104 = OpLabel
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_6_output = OpVariable %_ptr_Function_v4float Function
%_7_coord = OpVariable %_ptr_Function_v2float Function
%_8_coordSampled = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v4float Function
%124 = OpVariable %_ptr_Function_v2float Function
%138 = OpVariable %_ptr_Function_v4float Function
%140 = OpVariable %_ptr_Function_v2float Function
%153 = OpVariable %_ptr_Function_v4float Function
%155 = OpVariable %_ptr_Function_v2float Function
%168 = OpVariable %_ptr_Function_v4float Function
%170 = OpVariable %_ptr_Function_v2float Function
%183 = OpVariable %_ptr_Function_v4float Function
%185 = OpVariable %_ptr_Function_v2float Function
%198 = OpVariable %_ptr_Function_v4float Function
%200 = OpVariable %_ptr_Function_v2float Function
%213 = OpVariable %_ptr_Function_v4float Function
%215 = OpVariable %_ptr_Function_v2float Function
%228 = OpVariable %_ptr_Function_v4float Function
%230 = OpVariable %_ptr_Function_v2float Function
%243 = OpVariable %_ptr_Function_v4float Function
%245 = OpVariable %_ptr_Function_v2float Function
%258 = OpVariable %_ptr_Function_v4float Function
%260 = OpVariable %_ptr_Function_v2float Function
%273 = OpVariable %_ptr_Function_v4float Function
%275 = OpVariable %_ptr_Function_v2float Function
%288 = OpVariable %_ptr_Function_v4float Function
%290 = OpVariable %_ptr_Function_v2float Function
%303 = OpVariable %_ptr_Function_v4float Function
%305 = OpVariable %_ptr_Function_v2float Function
%318 = OpVariable %_ptr_Function_v4float Function
%320 = OpVariable %_ptr_Function_v2float Function
%333 = OpVariable %_ptr_Function_v4float Function
%335 = OpVariable %_ptr_Function_v2float Function
%348 = OpVariable %_ptr_Function_v4float Function
%350 = OpVariable %_ptr_Function_v2float Function
%363 = OpVariable %_ptr_Function_v4float Function
%365 = OpVariable %_ptr_Function_v2float Function
%378 = OpVariable %_ptr_Function_v4float Function
%380 = OpVariable %_ptr_Function_v2float Function
%393 = OpVariable %_ptr_Function_v4float Function
%395 = OpVariable %_ptr_Function_v2float Function
%408 = OpVariable %_ptr_Function_v4float Function
%410 = OpVariable %_ptr_Function_v2float Function
%423 = OpVariable %_ptr_Function_v4float Function
%425 = OpVariable %_ptr_Function_v2float Function
%438 = OpVariable %_ptr_Function_v4float Function
%440 = OpVariable %_ptr_Function_v2float Function
%453 = OpVariable %_ptr_Function_v4float Function
%455 = OpVariable %_ptr_Function_v2float Function
%468 = OpVariable %_ptr_Function_v4float Function
%470 = OpVariable %_ptr_Function_v2float Function
%483 = OpVariable %_ptr_Function_v4float Function
%485 = OpVariable %_ptr_Function_v2float Function
OpStore %_6_output %108
%110 = OpLoad %v2float %vLocalCoord_Stage0
%112 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%114 = OpLoad %v2float %112
%115 = OpVectorTimesScalar %v2float %114 %float_12
%116 = OpFSub %v2float %110 %115
OpStore %_7_coord %116
OpStore %_8_coordSampled %118
%119 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %119
%120 = OpLoad %v4float %_6_output
OpStore %122 %121
%123 = OpLoad %v2float %_8_coordSampled
OpStore %124 %123
%125 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %122 %124
%127 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%128 = OpLoad %v4float %127
%129 = OpCompositeExtract %float %128 0
%130 = OpVectorTimesScalar %v4float %125 %129
%131 = OpFAdd %v4float %120 %130
OpStore %_6_output %131
%132 = OpLoad %v2float %_7_coord
%133 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%134 = OpLoad %v2float %133
%135 = OpFAdd %v2float %132 %134
OpStore %_7_coord %135
%136 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %136
%137 = OpLoad %v4float %_6_output
OpStore %138 %121
%139 = OpLoad %v2float %_8_coordSampled
OpStore %140 %139
%141 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %138 %140
%142 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%143 = OpLoad %v4float %142
%144 = OpCompositeExtract %float %143 1
%145 = OpVectorTimesScalar %v4float %141 %144
%146 = OpFAdd %v4float %137 %145
OpStore %_6_output %146
%147 = OpLoad %v2float %_7_coord
%148 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%149 = OpLoad %v2float %148
%150 = OpFAdd %v2float %147 %149
OpStore %_7_coord %150
%151 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %151
%152 = OpLoad %v4float %_6_output
OpStore %153 %121
%154 = OpLoad %v2float %_8_coordSampled
OpStore %155 %154
%156 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %153 %155
%157 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 2
%160 = OpVectorTimesScalar %v4float %156 %159
%161 = OpFAdd %v4float %152 %160
OpStore %_6_output %161
%162 = OpLoad %v2float %_7_coord
%163 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%164 = OpLoad %v2float %163
%165 = OpFAdd %v2float %162 %164
OpStore %_7_coord %165
%166 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %166
%167 = OpLoad %v4float %_6_output
OpStore %168 %121
%169 = OpLoad %v2float %_8_coordSampled
OpStore %170 %169
%171 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %168 %170
%172 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 3
%175 = OpVectorTimesScalar %v4float %171 %174
%176 = OpFAdd %v4float %167 %175
OpStore %_6_output %176
%177 = OpLoad %v2float %_7_coord
%178 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%179 = OpLoad %v2float %178
%180 = OpFAdd %v2float %177 %179
OpStore %_7_coord %180
%181 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %181
%182 = OpLoad %v4float %_6_output
OpStore %183 %121
%184 = OpLoad %v2float %_8_coordSampled
OpStore %185 %184
%186 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %183 %185
%187 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%188 = OpLoad %v4float %187
%189 = OpCompositeExtract %float %188 0
%190 = OpVectorTimesScalar %v4float %186 %189
%191 = OpFAdd %v4float %182 %190
OpStore %_6_output %191
%192 = OpLoad %v2float %_7_coord
%193 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%194 = OpLoad %v2float %193
%195 = OpFAdd %v2float %192 %194
OpStore %_7_coord %195
%196 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %196
%197 = OpLoad %v4float %_6_output
OpStore %198 %121
%199 = OpLoad %v2float %_8_coordSampled
OpStore %200 %199
%201 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %198 %200
%202 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%203 = OpLoad %v4float %202
%204 = OpCompositeExtract %float %203 1
%205 = OpVectorTimesScalar %v4float %201 %204
%206 = OpFAdd %v4float %197 %205
OpStore %_6_output %206
%207 = OpLoad %v2float %_7_coord
%208 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%209 = OpLoad %v2float %208
%210 = OpFAdd %v2float %207 %209
OpStore %_7_coord %210
%211 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %211
%212 = OpLoad %v4float %_6_output
OpStore %213 %121
%214 = OpLoad %v2float %_8_coordSampled
OpStore %215 %214
%216 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %213 %215
%217 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%218 = OpLoad %v4float %217
%219 = OpCompositeExtract %float %218 2
%220 = OpVectorTimesScalar %v4float %216 %219
%221 = OpFAdd %v4float %212 %220
OpStore %_6_output %221
%222 = OpLoad %v2float %_7_coord
%223 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%224 = OpLoad %v2float %223
%225 = OpFAdd %v2float %222 %224
OpStore %_7_coord %225
%226 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %226
%227 = OpLoad %v4float %_6_output
OpStore %228 %121
%229 = OpLoad %v2float %_8_coordSampled
OpStore %230 %229
%231 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %228 %230
%232 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%233 = OpLoad %v4float %232
%234 = OpCompositeExtract %float %233 3
%235 = OpVectorTimesScalar %v4float %231 %234
%236 = OpFAdd %v4float %227 %235
OpStore %_6_output %236
%237 = OpLoad %v2float %_7_coord
%238 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%239 = OpLoad %v2float %238
%240 = OpFAdd %v2float %237 %239
OpStore %_7_coord %240
%241 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %241
%242 = OpLoad %v4float %_6_output
OpStore %243 %121
%244 = OpLoad %v2float %_8_coordSampled
OpStore %245 %244
%246 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %243 %245
%247 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%248 = OpLoad %v4float %247
%249 = OpCompositeExtract %float %248 0
%250 = OpVectorTimesScalar %v4float %246 %249
%251 = OpFAdd %v4float %242 %250
OpStore %_6_output %251
%252 = OpLoad %v2float %_7_coord
%253 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%254 = OpLoad %v2float %253
%255 = OpFAdd %v2float %252 %254
OpStore %_7_coord %255
%256 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %256
%257 = OpLoad %v4float %_6_output
OpStore %258 %121
%259 = OpLoad %v2float %_8_coordSampled
OpStore %260 %259
%261 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %258 %260
%262 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%263 = OpLoad %v4float %262
%264 = OpCompositeExtract %float %263 1
%265 = OpVectorTimesScalar %v4float %261 %264
%266 = OpFAdd %v4float %257 %265
OpStore %_6_output %266
%267 = OpLoad %v2float %_7_coord
%268 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%269 = OpLoad %v2float %268
%270 = OpFAdd %v2float %267 %269
OpStore %_7_coord %270
%271 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %271
%272 = OpLoad %v4float %_6_output
OpStore %273 %121
%274 = OpLoad %v2float %_8_coordSampled
OpStore %275 %274
%276 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %273 %275
%277 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%278 = OpLoad %v4float %277
%279 = OpCompositeExtract %float %278 2
%280 = OpVectorTimesScalar %v4float %276 %279
%281 = OpFAdd %v4float %272 %280
OpStore %_6_output %281
%282 = OpLoad %v2float %_7_coord
%283 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%284 = OpLoad %v2float %283
%285 = OpFAdd %v2float %282 %284
OpStore %_7_coord %285
%286 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %286
%287 = OpLoad %v4float %_6_output
OpStore %288 %121
%289 = OpLoad %v2float %_8_coordSampled
OpStore %290 %289
%291 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %288 %290
%292 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%293 = OpLoad %v4float %292
%294 = OpCompositeExtract %float %293 3
%295 = OpVectorTimesScalar %v4float %291 %294
%296 = OpFAdd %v4float %287 %295
OpStore %_6_output %296
%297 = OpLoad %v2float %_7_coord
%298 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%299 = OpLoad %v2float %298
%300 = OpFAdd %v2float %297 %299
OpStore %_7_coord %300
%301 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %301
%302 = OpLoad %v4float %_6_output
OpStore %303 %121
%304 = OpLoad %v2float %_8_coordSampled
OpStore %305 %304
%306 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %303 %305
%307 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%308 = OpLoad %v4float %307
%309 = OpCompositeExtract %float %308 0
%310 = OpVectorTimesScalar %v4float %306 %309
%311 = OpFAdd %v4float %302 %310
OpStore %_6_output %311
%312 = OpLoad %v2float %_7_coord
%313 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%314 = OpLoad %v2float %313
%315 = OpFAdd %v2float %312 %314
OpStore %_7_coord %315
%316 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %316
%317 = OpLoad %v4float %_6_output
OpStore %318 %121
%319 = OpLoad %v2float %_8_coordSampled
OpStore %320 %319
%321 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %318 %320
%322 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%323 = OpLoad %v4float %322
%324 = OpCompositeExtract %float %323 1
%325 = OpVectorTimesScalar %v4float %321 %324
%326 = OpFAdd %v4float %317 %325
OpStore %_6_output %326
%327 = OpLoad %v2float %_7_coord
%328 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%329 = OpLoad %v2float %328
%330 = OpFAdd %v2float %327 %329
OpStore %_7_coord %330
%331 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %331
%332 = OpLoad %v4float %_6_output
OpStore %333 %121
%334 = OpLoad %v2float %_8_coordSampled
OpStore %335 %334
%336 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %333 %335
%337 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%338 = OpLoad %v4float %337
%339 = OpCompositeExtract %float %338 2
%340 = OpVectorTimesScalar %v4float %336 %339
%341 = OpFAdd %v4float %332 %340
OpStore %_6_output %341
%342 = OpLoad %v2float %_7_coord
%343 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%344 = OpLoad %v2float %343
%345 = OpFAdd %v2float %342 %344
OpStore %_7_coord %345
%346 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %346
%347 = OpLoad %v4float %_6_output
OpStore %348 %121
%349 = OpLoad %v2float %_8_coordSampled
OpStore %350 %349
%351 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %348 %350
%352 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%353 = OpLoad %v4float %352
%354 = OpCompositeExtract %float %353 3
%355 = OpVectorTimesScalar %v4float %351 %354
%356 = OpFAdd %v4float %347 %355
OpStore %_6_output %356
%357 = OpLoad %v2float %_7_coord
%358 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%359 = OpLoad %v2float %358
%360 = OpFAdd %v2float %357 %359
OpStore %_7_coord %360
%361 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %361
%362 = OpLoad %v4float %_6_output
OpStore %363 %121
%364 = OpLoad %v2float %_8_coordSampled
OpStore %365 %364
%366 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %363 %365
%367 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%368 = OpLoad %v4float %367
%369 = OpCompositeExtract %float %368 0
%370 = OpVectorTimesScalar %v4float %366 %369
%371 = OpFAdd %v4float %362 %370
OpStore %_6_output %371
%372 = OpLoad %v2float %_7_coord
%373 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%374 = OpLoad %v2float %373
%375 = OpFAdd %v2float %372 %374
OpStore %_7_coord %375
%376 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %376
%377 = OpLoad %v4float %_6_output
OpStore %378 %121
%379 = OpLoad %v2float %_8_coordSampled
OpStore %380 %379
%381 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %378 %380
%382 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%383 = OpLoad %v4float %382
%384 = OpCompositeExtract %float %383 1
%385 = OpVectorTimesScalar %v4float %381 %384
%386 = OpFAdd %v4float %377 %385
OpStore %_6_output %386
%387 = OpLoad %v2float %_7_coord
%388 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%389 = OpLoad %v2float %388
%390 = OpFAdd %v2float %387 %389
OpStore %_7_coord %390
%391 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %391
%392 = OpLoad %v4float %_6_output
OpStore %393 %121
%394 = OpLoad %v2float %_8_coordSampled
OpStore %395 %394
%396 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %393 %395
%397 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%398 = OpLoad %v4float %397
%399 = OpCompositeExtract %float %398 2
%400 = OpVectorTimesScalar %v4float %396 %399
%401 = OpFAdd %v4float %392 %400
OpStore %_6_output %401
%402 = OpLoad %v2float %_7_coord
%403 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%404 = OpLoad %v2float %403
%405 = OpFAdd %v2float %402 %404
OpStore %_7_coord %405
%406 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %406
%407 = OpLoad %v4float %_6_output
OpStore %408 %121
%409 = OpLoad %v2float %_8_coordSampled
OpStore %410 %409
%411 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %408 %410
%412 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%413 = OpLoad %v4float %412
%414 = OpCompositeExtract %float %413 3
%415 = OpVectorTimesScalar %v4float %411 %414
%416 = OpFAdd %v4float %407 %415
OpStore %_6_output %416
%417 = OpLoad %v2float %_7_coord
%418 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%419 = OpLoad %v2float %418
%420 = OpFAdd %v2float %417 %419
OpStore %_7_coord %420
%421 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %421
%422 = OpLoad %v4float %_6_output
OpStore %423 %121
%424 = OpLoad %v2float %_8_coordSampled
OpStore %425 %424
%426 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %423 %425
%427 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%428 = OpLoad %v4float %427
%429 = OpCompositeExtract %float %428 0
%430 = OpVectorTimesScalar %v4float %426 %429
%431 = OpFAdd %v4float %422 %430
OpStore %_6_output %431
%432 = OpLoad %v2float %_7_coord
%433 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%434 = OpLoad %v2float %433
%435 = OpFAdd %v2float %432 %434
OpStore %_7_coord %435
%436 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %436
%437 = OpLoad %v4float %_6_output
OpStore %438 %121
%439 = OpLoad %v2float %_8_coordSampled
OpStore %440 %439
%441 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %438 %440
%442 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%443 = OpLoad %v4float %442
%444 = OpCompositeExtract %float %443 1
%445 = OpVectorTimesScalar %v4float %441 %444
%446 = OpFAdd %v4float %437 %445
OpStore %_6_output %446
%447 = OpLoad %v2float %_7_coord
%448 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%449 = OpLoad %v2float %448
%450 = OpFAdd %v2float %447 %449
OpStore %_7_coord %450
%451 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %451
%452 = OpLoad %v4float %_6_output
OpStore %453 %121
%454 = OpLoad %v2float %_8_coordSampled
OpStore %455 %454
%456 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %453 %455
%457 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%458 = OpLoad %v4float %457
%459 = OpCompositeExtract %float %458 2
%460 = OpVectorTimesScalar %v4float %456 %459
%461 = OpFAdd %v4float %452 %460
OpStore %_6_output %461
%462 = OpLoad %v2float %_7_coord
%463 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%464 = OpLoad %v2float %463
%465 = OpFAdd %v2float %462 %464
OpStore %_7_coord %465
%466 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %466
%467 = OpLoad %v4float %_6_output
OpStore %468 %121
%469 = OpLoad %v2float %_8_coordSampled
OpStore %470 %469
%471 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %468 %470
%472 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%473 = OpLoad %v4float %472
%474 = OpCompositeExtract %float %473 3
%475 = OpVectorTimesScalar %v4float %471 %474
%476 = OpFAdd %v4float %467 %475
OpStore %_6_output %476
%477 = OpLoad %v2float %_7_coord
%478 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%479 = OpLoad %v2float %478
%480 = OpFAdd %v2float %477 %479
OpStore %_7_coord %480
%481 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %481
%482 = OpLoad %v4float %_6_output
OpStore %483 %121
%484 = OpLoad %v2float %_8_coordSampled
OpStore %485 %484
%486 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %483 %485
%487 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%488 = OpLoad %v4float %487
%489 = OpCompositeExtract %float %488 0
%490 = OpVectorTimesScalar %v4float %486 %489
%491 = OpFAdd %v4float %482 %490
OpStore %_6_output %491
%492 = OpLoad %v2float %_7_coord
%493 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%494 = OpLoad %v2float %493
%495 = OpFAdd %v2float %492 %494
OpStore %_7_coord %495
%496 = OpLoad %v4float %_6_output
OpStore %output_Stage1 %496
%497 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %497
OpReturn
OpFunctionEnd
