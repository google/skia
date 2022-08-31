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
OpName %MatrixEffect_Stage1_c0_c0_h4h4f2 "MatrixEffect_Stage1_c0_c0_h4h4f2"
OpName %_1_inCoord "_1_inCoord"
OpName %_2_subsetCoord "_2_subsetCoord"
OpName %_3_clampedCoord "_3_clampedCoord"
OpName %_4_textureColor "_4_textureColor"
OpName %_5_snappedX "_5_snappedX"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %_4_textureColor RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_6_output RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
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
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%21 = OpTypeImage %float 2D 0 0 0 1 Unknown
%22 = OpTypeSampledImage %21
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_22 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
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
%96 = OpTypeFunction %void
%100 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%104 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%114 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0_h4h4f2 = OpFunction %v4float None %28
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
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
OpStore %_1_inCoord %43
%45 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%47 = OpLoad %v4float %45
%48 = OpVectorShuffle %v2float %47 %47 0 1
%49 = OpFMul %v2float %43 %48
OpStore %_1_inCoord %49
%51 = OpCompositeExtract %float %49 0
%52 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_0
OpStore %52 %51
%55 = OpLoad %v2float %_1_inCoord
%56 = OpCompositeExtract %float %55 1
%57 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_1
OpStore %57 %56
%60 = OpLoad %v2float %_2_subsetCoord
OpStore %_3_clampedCoord %60
%63 = OpLoad %22 %uTextureSampler_0_Stage1
%64 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v2float %65 %65 2 3
%67 = OpFMul %v2float %60 %66
%62 = OpImageSampleImplicitLod %v4float %63 %67
OpStore %_4_textureColor %62
%70 = OpLoad %v2float %_1_inCoord
%71 = OpCompositeExtract %float %70 0
%73 = OpFAdd %float %71 %float_0_00100000005
%69 = OpExtInst %float %1 Floor %73
%75 = OpFAdd %float %69 %float_0_5
OpStore %_5_snappedX %75
%78 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%79 = OpLoad %v4float %78
%80 = OpCompositeExtract %float %79 0
%81 = OpFOrdLessThan %bool %75 %80
OpSelectionMerge %83 None
OpBranchConditional %81 %83 %82
%82 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 2
%87 = OpFOrdGreaterThan %bool %75 %86
OpBranch %83
%83 = OpLabel
%88 = OpPhi %bool %true %31 %87 %82
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%93 = OpLoad %v4float %92
OpStore %_4_textureColor %93
OpBranch %90
%90 = OpLabel
%94 = OpLoad %v4float %_4_textureColor
OpReturnValue %94
OpFunctionEnd
%main = OpFunction %void None %96
%97 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_6_output = OpVariable %_ptr_Function_v4float Function
%_7_coord = OpVariable %_ptr_Function_v2float Function
%_8_coordSampled = OpVariable %_ptr_Function_v2float Function
%115 = OpVariable %_ptr_Function_v4float Function
%116 = OpVariable %_ptr_Function_v2float Function
%127 = OpVariable %_ptr_Function_v4float Function
%128 = OpVariable %_ptr_Function_v2float Function
%138 = OpVariable %_ptr_Function_v4float Function
%139 = OpVariable %_ptr_Function_v2float Function
%149 = OpVariable %_ptr_Function_v4float Function
%150 = OpVariable %_ptr_Function_v2float Function
%160 = OpVariable %_ptr_Function_v4float Function
%161 = OpVariable %_ptr_Function_v2float Function
%171 = OpVariable %_ptr_Function_v4float Function
%172 = OpVariable %_ptr_Function_v2float Function
%182 = OpVariable %_ptr_Function_v4float Function
%183 = OpVariable %_ptr_Function_v2float Function
%193 = OpVariable %_ptr_Function_v4float Function
%194 = OpVariable %_ptr_Function_v2float Function
%204 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_v2float Function
%215 = OpVariable %_ptr_Function_v4float Function
%216 = OpVariable %_ptr_Function_v2float Function
%226 = OpVariable %_ptr_Function_v4float Function
%227 = OpVariable %_ptr_Function_v2float Function
%237 = OpVariable %_ptr_Function_v4float Function
%238 = OpVariable %_ptr_Function_v2float Function
%248 = OpVariable %_ptr_Function_v4float Function
%249 = OpVariable %_ptr_Function_v2float Function
%259 = OpVariable %_ptr_Function_v4float Function
%260 = OpVariable %_ptr_Function_v2float Function
%270 = OpVariable %_ptr_Function_v4float Function
%271 = OpVariable %_ptr_Function_v2float Function
%281 = OpVariable %_ptr_Function_v4float Function
%282 = OpVariable %_ptr_Function_v2float Function
%292 = OpVariable %_ptr_Function_v4float Function
%293 = OpVariable %_ptr_Function_v2float Function
%303 = OpVariable %_ptr_Function_v4float Function
%304 = OpVariable %_ptr_Function_v2float Function
%314 = OpVariable %_ptr_Function_v4float Function
%315 = OpVariable %_ptr_Function_v2float Function
%325 = OpVariable %_ptr_Function_v4float Function
%326 = OpVariable %_ptr_Function_v2float Function
%336 = OpVariable %_ptr_Function_v4float Function
%337 = OpVariable %_ptr_Function_v2float Function
%347 = OpVariable %_ptr_Function_v4float Function
%348 = OpVariable %_ptr_Function_v2float Function
%358 = OpVariable %_ptr_Function_v4float Function
%359 = OpVariable %_ptr_Function_v2float Function
%369 = OpVariable %_ptr_Function_v4float Function
%370 = OpVariable %_ptr_Function_v2float Function
%380 = OpVariable %_ptr_Function_v4float Function
%381 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %100
OpStore %outputCoverage_Stage0 %100
OpStore %_6_output %104
%106 = OpLoad %v2float %vLocalCoord_Stage0
%108 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%110 = OpLoad %v2float %108
%111 = OpVectorTimesScalar %v2float %110 %float_12
%112 = OpFSub %v2float %106 %111
OpStore %_7_coord %112
OpStore %_8_coordSampled %114
OpStore %_8_coordSampled %112
OpStore %115 %100
OpStore %116 %112
%117 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %115 %116
%119 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpVectorTimesScalar %v4float %117 %121
%123 = OpFAdd %v4float %104 %122
OpStore %_6_output %123
%124 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%125 = OpLoad %v2float %124
%126 = OpFAdd %v2float %112 %125
OpStore %_7_coord %126
OpStore %_8_coordSampled %126
OpStore %127 %100
OpStore %128 %126
%129 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %127 %128
%130 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 1
%133 = OpVectorTimesScalar %v4float %129 %132
%134 = OpFAdd %v4float %123 %133
OpStore %_6_output %134
%135 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%136 = OpLoad %v2float %135
%137 = OpFAdd %v2float %126 %136
OpStore %_7_coord %137
OpStore %_8_coordSampled %137
OpStore %138 %100
OpStore %139 %137
%140 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %138 %139
%141 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%142 = OpLoad %v4float %141
%143 = OpCompositeExtract %float %142 2
%144 = OpVectorTimesScalar %v4float %140 %143
%145 = OpFAdd %v4float %134 %144
OpStore %_6_output %145
%146 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%147 = OpLoad %v2float %146
%148 = OpFAdd %v2float %137 %147
OpStore %_7_coord %148
OpStore %_8_coordSampled %148
OpStore %149 %100
OpStore %150 %148
%151 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %149 %150
%152 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 3
%155 = OpVectorTimesScalar %v4float %151 %154
%156 = OpFAdd %v4float %145 %155
OpStore %_6_output %156
%157 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%158 = OpLoad %v2float %157
%159 = OpFAdd %v2float %148 %158
OpStore %_7_coord %159
OpStore %_8_coordSampled %159
OpStore %160 %100
OpStore %161 %159
%162 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %160 %161
%163 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%164 = OpLoad %v4float %163
%165 = OpCompositeExtract %float %164 0
%166 = OpVectorTimesScalar %v4float %162 %165
%167 = OpFAdd %v4float %156 %166
OpStore %_6_output %167
%168 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%169 = OpLoad %v2float %168
%170 = OpFAdd %v2float %159 %169
OpStore %_7_coord %170
OpStore %_8_coordSampled %170
OpStore %171 %100
OpStore %172 %170
%173 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %171 %172
%174 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 1
%177 = OpVectorTimesScalar %v4float %173 %176
%178 = OpFAdd %v4float %167 %177
OpStore %_6_output %178
%179 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%180 = OpLoad %v2float %179
%181 = OpFAdd %v2float %170 %180
OpStore %_7_coord %181
OpStore %_8_coordSampled %181
OpStore %182 %100
OpStore %183 %181
%184 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %182 %183
%185 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%186 = OpLoad %v4float %185
%187 = OpCompositeExtract %float %186 2
%188 = OpVectorTimesScalar %v4float %184 %187
%189 = OpFAdd %v4float %178 %188
OpStore %_6_output %189
%190 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%191 = OpLoad %v2float %190
%192 = OpFAdd %v2float %181 %191
OpStore %_7_coord %192
OpStore %_8_coordSampled %192
OpStore %193 %100
OpStore %194 %192
%195 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %193 %194
%196 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%197 = OpLoad %v4float %196
%198 = OpCompositeExtract %float %197 3
%199 = OpVectorTimesScalar %v4float %195 %198
%200 = OpFAdd %v4float %189 %199
OpStore %_6_output %200
%201 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%202 = OpLoad %v2float %201
%203 = OpFAdd %v2float %192 %202
OpStore %_7_coord %203
OpStore %_8_coordSampled %203
OpStore %204 %100
OpStore %205 %203
%206 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %204 %205
%207 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpVectorTimesScalar %v4float %206 %209
%211 = OpFAdd %v4float %200 %210
OpStore %_6_output %211
%212 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%213 = OpLoad %v2float %212
%214 = OpFAdd %v2float %203 %213
OpStore %_7_coord %214
OpStore %_8_coordSampled %214
OpStore %215 %100
OpStore %216 %214
%217 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %215 %216
%218 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%219 = OpLoad %v4float %218
%220 = OpCompositeExtract %float %219 1
%221 = OpVectorTimesScalar %v4float %217 %220
%222 = OpFAdd %v4float %211 %221
OpStore %_6_output %222
%223 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%224 = OpLoad %v2float %223
%225 = OpFAdd %v2float %214 %224
OpStore %_7_coord %225
OpStore %_8_coordSampled %225
OpStore %226 %100
OpStore %227 %225
%228 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %226 %227
%229 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%230 = OpLoad %v4float %229
%231 = OpCompositeExtract %float %230 2
%232 = OpVectorTimesScalar %v4float %228 %231
%233 = OpFAdd %v4float %222 %232
OpStore %_6_output %233
%234 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%235 = OpLoad %v2float %234
%236 = OpFAdd %v2float %225 %235
OpStore %_7_coord %236
OpStore %_8_coordSampled %236
OpStore %237 %100
OpStore %238 %236
%239 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %237 %238
%240 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%241 = OpLoad %v4float %240
%242 = OpCompositeExtract %float %241 3
%243 = OpVectorTimesScalar %v4float %239 %242
%244 = OpFAdd %v4float %233 %243
OpStore %_6_output %244
%245 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%246 = OpLoad %v2float %245
%247 = OpFAdd %v2float %236 %246
OpStore %_7_coord %247
OpStore %_8_coordSampled %247
OpStore %248 %100
OpStore %249 %247
%250 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %248 %249
%251 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%252 = OpLoad %v4float %251
%253 = OpCompositeExtract %float %252 0
%254 = OpVectorTimesScalar %v4float %250 %253
%255 = OpFAdd %v4float %244 %254
OpStore %_6_output %255
%256 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%257 = OpLoad %v2float %256
%258 = OpFAdd %v2float %247 %257
OpStore %_7_coord %258
OpStore %_8_coordSampled %258
OpStore %259 %100
OpStore %260 %258
%261 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %259 %260
%262 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%263 = OpLoad %v4float %262
%264 = OpCompositeExtract %float %263 1
%265 = OpVectorTimesScalar %v4float %261 %264
%266 = OpFAdd %v4float %255 %265
OpStore %_6_output %266
%267 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%268 = OpLoad %v2float %267
%269 = OpFAdd %v2float %258 %268
OpStore %_7_coord %269
OpStore %_8_coordSampled %269
OpStore %270 %100
OpStore %271 %269
%272 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %270 %271
%273 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%274 = OpLoad %v4float %273
%275 = OpCompositeExtract %float %274 2
%276 = OpVectorTimesScalar %v4float %272 %275
%277 = OpFAdd %v4float %266 %276
OpStore %_6_output %277
%278 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%279 = OpLoad %v2float %278
%280 = OpFAdd %v2float %269 %279
OpStore %_7_coord %280
OpStore %_8_coordSampled %280
OpStore %281 %100
OpStore %282 %280
%283 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %281 %282
%284 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%285 = OpLoad %v4float %284
%286 = OpCompositeExtract %float %285 3
%287 = OpVectorTimesScalar %v4float %283 %286
%288 = OpFAdd %v4float %277 %287
OpStore %_6_output %288
%289 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%290 = OpLoad %v2float %289
%291 = OpFAdd %v2float %280 %290
OpStore %_7_coord %291
OpStore %_8_coordSampled %291
OpStore %292 %100
OpStore %293 %291
%294 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %292 %293
%295 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%296 = OpLoad %v4float %295
%297 = OpCompositeExtract %float %296 0
%298 = OpVectorTimesScalar %v4float %294 %297
%299 = OpFAdd %v4float %288 %298
OpStore %_6_output %299
%300 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%301 = OpLoad %v2float %300
%302 = OpFAdd %v2float %291 %301
OpStore %_7_coord %302
OpStore %_8_coordSampled %302
OpStore %303 %100
OpStore %304 %302
%305 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %303 %304
%306 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%307 = OpLoad %v4float %306
%308 = OpCompositeExtract %float %307 1
%309 = OpVectorTimesScalar %v4float %305 %308
%310 = OpFAdd %v4float %299 %309
OpStore %_6_output %310
%311 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%312 = OpLoad %v2float %311
%313 = OpFAdd %v2float %302 %312
OpStore %_7_coord %313
OpStore %_8_coordSampled %313
OpStore %314 %100
OpStore %315 %313
%316 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %314 %315
%317 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%318 = OpLoad %v4float %317
%319 = OpCompositeExtract %float %318 2
%320 = OpVectorTimesScalar %v4float %316 %319
%321 = OpFAdd %v4float %310 %320
OpStore %_6_output %321
%322 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%323 = OpLoad %v2float %322
%324 = OpFAdd %v2float %313 %323
OpStore %_7_coord %324
OpStore %_8_coordSampled %324
OpStore %325 %100
OpStore %326 %324
%327 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %325 %326
%328 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%329 = OpLoad %v4float %328
%330 = OpCompositeExtract %float %329 3
%331 = OpVectorTimesScalar %v4float %327 %330
%332 = OpFAdd %v4float %321 %331
OpStore %_6_output %332
%333 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%334 = OpLoad %v2float %333
%335 = OpFAdd %v2float %324 %334
OpStore %_7_coord %335
OpStore %_8_coordSampled %335
OpStore %336 %100
OpStore %337 %335
%338 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %336 %337
%339 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%340 = OpLoad %v4float %339
%341 = OpCompositeExtract %float %340 0
%342 = OpVectorTimesScalar %v4float %338 %341
%343 = OpFAdd %v4float %332 %342
OpStore %_6_output %343
%344 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%345 = OpLoad %v2float %344
%346 = OpFAdd %v2float %335 %345
OpStore %_7_coord %346
OpStore %_8_coordSampled %346
OpStore %347 %100
OpStore %348 %346
%349 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %347 %348
%350 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%351 = OpLoad %v4float %350
%352 = OpCompositeExtract %float %351 1
%353 = OpVectorTimesScalar %v4float %349 %352
%354 = OpFAdd %v4float %343 %353
OpStore %_6_output %354
%355 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%356 = OpLoad %v2float %355
%357 = OpFAdd %v2float %346 %356
OpStore %_7_coord %357
OpStore %_8_coordSampled %357
OpStore %358 %100
OpStore %359 %357
%360 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %358 %359
%361 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%362 = OpLoad %v4float %361
%363 = OpCompositeExtract %float %362 2
%364 = OpVectorTimesScalar %v4float %360 %363
%365 = OpFAdd %v4float %354 %364
OpStore %_6_output %365
%366 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%367 = OpLoad %v2float %366
%368 = OpFAdd %v2float %357 %367
OpStore %_7_coord %368
OpStore %_8_coordSampled %368
OpStore %369 %100
OpStore %370 %368
%371 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %369 %370
%372 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%373 = OpLoad %v4float %372
%374 = OpCompositeExtract %float %373 3
%375 = OpVectorTimesScalar %v4float %371 %374
%376 = OpFAdd %v4float %365 %375
OpStore %_6_output %376
%377 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%378 = OpLoad %v2float %377
%379 = OpFAdd %v2float %368 %378
OpStore %_7_coord %379
OpStore %_8_coordSampled %379
OpStore %380 %100
OpStore %381 %379
%382 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %380 %381
%383 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%384 = OpLoad %v4float %383
%385 = OpCompositeExtract %float %384 0
%386 = OpVectorTimesScalar %v4float %382 %385
%387 = OpFAdd %v4float %376 %386
OpStore %_6_output %387
%388 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%389 = OpLoad %v2float %388
%390 = OpFAdd %v2float %379 %389
OpStore %_7_coord %390
%391 = OpFMul %v4float %387 %100
OpStore %_6_output %391
OpStore %output_Stage1 %391
%392 = OpFMul %v4float %391 %100
OpStore %sk_FragColor %392
OpReturn
OpFunctionEnd
