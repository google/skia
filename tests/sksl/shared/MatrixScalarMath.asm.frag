OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %divisionTest_b "divisionTest_b"
OpName %ten "ten"
OpName %mat "mat"
OpName %div "div"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %_0_expected "_0_expected"
OpName %_1_one "_1_one"
OpName %_2_m2 "_2_m2"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%30 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%63 = OpConstantComposite %v2float %float_1 %float_1
%64 = OpConstantComposite %mat2v2float %63 %63
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%119 = OpTypeFunction %bool
%float_10 = OpConstant %float 10
%int_2 = OpConstant %int 2
%float_n8 = OpConstant %float -8
%149 = OpConstantComposite %v2float %float_n8 %float_n8
%150 = OpConstantComposite %mat2v2float %149 %149
%v2bool = OpTypeVector %bool 2
%165 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %30
%31 = OpFunctionParameter %_ptr_Function_int
%32 = OpFunctionParameter %_ptr_Function_float
%33 = OpFunctionParameter %_ptr_Function_float
%34 = OpFunctionParameter %_ptr_Function_float
%35 = OpFunctionParameter %_ptr_Function_float
%36 = OpFunctionParameter %_ptr_Function_mat2v2float
%37 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%42 = OpLoad %v4float %39
%43 = OpCompositeExtract %float %42 0
OpStore %one %43
%45 = OpLoad %float %32
%46 = OpFMul %float %45 %43
%47 = OpLoad %float %33
%48 = OpFMul %float %47 %43
%49 = OpLoad %float %34
%50 = OpFMul %float %49 %43
%51 = OpLoad %float %35
%52 = OpFMul %float %51 %43
%53 = OpCompositeConstruct %v2float %46 %48
%54 = OpCompositeConstruct %v2float %50 %52
%55 = OpCompositeConstruct %mat2v2float %53 %54
OpStore %m2 %55
%56 = OpLoad %int %31
OpSelectionMerge %57 None
OpSwitch %56 %57 1 %58 2 %59 3 %60 4 %61
%58 = OpLabel
%65 = OpFAdd %v2float %53 %63
%66 = OpFAdd %v2float %54 %63
%67 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %m2 %67
OpBranch %57
%59 = OpLabel
%68 = OpLoad %mat2v2float %m2
%69 = OpCompositeExtract %v2float %68 0
%70 = OpFSub %v2float %69 %63
%71 = OpCompositeExtract %v2float %68 1
%72 = OpFSub %v2float %71 %63
%73 = OpCompositeConstruct %mat2v2float %70 %72
OpStore %m2 %73
OpBranch %57
%60 = OpLabel
%74 = OpLoad %mat2v2float %m2
%76 = OpMatrixTimesScalar %mat2v2float %74 %float_2
OpStore %m2 %76
OpBranch %57
%61 = OpLabel
%77 = OpLoad %mat2v2float %m2
%79 = OpMatrixTimesScalar %mat2v2float %77 %float_0_5
OpStore %m2 %79
OpBranch %57
%57 = OpLabel
%82 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%83 = OpLoad %v2float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpAccessChain %_ptr_Function_v2float %36 %int_0
%86 = OpLoad %v2float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %84 %87
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%92 = OpLoad %v2float %91
%93 = OpCompositeExtract %float %92 1
%94 = OpAccessChain %_ptr_Function_v2float %36 %int_0
%95 = OpLoad %v2float %94
%96 = OpCompositeExtract %float %95 1
%97 = OpFOrdEqual %bool %93 %96
OpBranch %90
%90 = OpLabel
%98 = OpPhi %bool %false %57 %97 %89
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%102 = OpLoad %v2float %101
%103 = OpCompositeExtract %float %102 0
%104 = OpAccessChain %_ptr_Function_v2float %36 %int_1
%105 = OpLoad %v2float %104
%106 = OpCompositeExtract %float %105 0
%107 = OpFOrdEqual %bool %103 %106
OpBranch %100
%100 = OpLabel
%108 = OpPhi %bool %false %90 %107 %99
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%112 = OpLoad %v2float %111
%113 = OpCompositeExtract %float %112 1
%114 = OpAccessChain %_ptr_Function_v2float %36 %int_1
%115 = OpLoad %v2float %114
%116 = OpCompositeExtract %float %115 1
%117 = OpFOrdEqual %bool %113 %116
OpBranch %110
%110 = OpLabel
%118 = OpPhi %bool %false %100 %117 %109
OpReturnValue %118
OpFunctionEnd
%divisionTest_b = OpFunction %bool None %119
%120 = OpLabel
%ten = OpVariable %_ptr_Function_float Function
%mat = OpVariable %_ptr_Function_mat2v2float Function
%div = OpVariable %_ptr_Function_mat2v2float Function
%122 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 0
%126 = OpFMul %float %124 %float_10
OpStore %ten %126
%128 = OpCompositeConstruct %v2float %126 %126
%129 = OpCompositeConstruct %mat2v2float %128 %128
OpStore %mat %129
%131 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%133 = OpLoad %v4float %131
%134 = OpCompositeExtract %float %133 0
%135 = OpCompositeConstruct %v2float %134 %134
%136 = OpCompositeConstruct %mat2v2float %135 %135
%137 = OpFDiv %v2float %128 %135
%138 = OpFDiv %v2float %128 %135
%139 = OpCompositeConstruct %mat2v2float %137 %138
OpStore %div %139
%140 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%141 = OpLoad %v4float %140
%142 = OpCompositeExtract %float %141 0
%143 = OpCompositeConstruct %v2float %142 %142
%144 = OpCompositeConstruct %mat2v2float %143 %143
%145 = OpFDiv %v2float %128 %143
%146 = OpFDiv %v2float %128 %143
%147 = OpCompositeConstruct %mat2v2float %145 %146
OpStore %mat %147
%152 = OpFOrdEqual %v2bool %137 %149
%153 = OpAll %bool %152
%154 = OpFOrdEqual %v2bool %138 %149
%155 = OpAll %bool %154
%156 = OpLogicalAnd %bool %153 %155
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpFOrdEqual %v2bool %145 %149
%160 = OpAll %bool %159
%161 = OpFOrdEqual %v2bool %146 %149
%162 = OpAll %bool %161
%163 = OpLogicalAnd %bool %160 %162
OpBranch %158
%158 = OpLabel
%164 = OpPhi %bool %false %120 %163 %157
OpReturnValue %164
OpFunctionEnd
%main = OpFunction %v4float None %165
%166 = OpFunctionParameter %_ptr_Function_v2float
%167 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%251 = OpVariable %_ptr_Function_int Function
%252 = OpVariable %_ptr_Function_float Function
%253 = OpVariable %_ptr_Function_float Function
%254 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%263 = OpVariable %_ptr_Function_mat2v2float Function
%269 = OpVariable %_ptr_Function_int Function
%270 = OpVariable %_ptr_Function_float Function
%271 = OpVariable %_ptr_Function_float Function
%272 = OpVariable %_ptr_Function_float Function
%273 = OpVariable %_ptr_Function_float Function
%281 = OpVariable %_ptr_Function_mat2v2float Function
%287 = OpVariable %_ptr_Function_int Function
%288 = OpVariable %_ptr_Function_float Function
%289 = OpVariable %_ptr_Function_float Function
%290 = OpVariable %_ptr_Function_float Function
%291 = OpVariable %_ptr_Function_float Function
%299 = OpVariable %_ptr_Function_mat2v2float Function
%306 = OpVariable %_ptr_Function_v4float Function
%169 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 1
OpStore %f1 %171
%173 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%174 = OpLoad %v4float %173
%175 = OpCompositeExtract %float %174 1
%176 = OpFMul %float %float_2 %175
OpStore %f2 %176
%179 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%180 = OpLoad %v4float %179
%181 = OpCompositeExtract %float %180 1
%182 = OpFMul %float %float_3 %181
OpStore %f3 %182
%185 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%186 = OpLoad %v4float %185
%187 = OpCompositeExtract %float %186 1
%188 = OpFMul %float %float_4 %187
OpStore %f4 %188
%190 = OpFAdd %float %171 %float_1
%191 = OpFAdd %float %176 %float_1
%192 = OpFAdd %float %182 %float_1
%193 = OpFAdd %float %188 %float_1
%194 = OpCompositeConstruct %v2float %190 %191
%195 = OpCompositeConstruct %v2float %192 %193
%196 = OpCompositeConstruct %mat2v2float %194 %195
OpStore %_0_expected %196
%198 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%199 = OpLoad %v4float %198
%200 = OpCompositeExtract %float %199 0
OpStore %_1_one %200
%202 = OpFMul %float %171 %200
%203 = OpFMul %float %176 %200
%204 = OpFMul %float %182 %200
%205 = OpFMul %float %188 %200
%206 = OpCompositeConstruct %v2float %202 %203
%207 = OpCompositeConstruct %v2float %204 %205
%208 = OpCompositeConstruct %mat2v2float %206 %207
OpStore %_2_m2 %208
%209 = OpFAdd %v2float %206 %63
%210 = OpFAdd %v2float %207 %63
%211 = OpCompositeConstruct %mat2v2float %209 %210
OpStore %_2_m2 %211
%212 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%213 = OpLoad %v2float %212
%214 = OpCompositeExtract %float %213 0
%215 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%216 = OpLoad %v2float %215
%217 = OpCompositeExtract %float %216 0
%218 = OpFOrdEqual %bool %214 %217
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%222 = OpLoad %v2float %221
%223 = OpCompositeExtract %float %222 1
%224 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%225 = OpLoad %v2float %224
%226 = OpCompositeExtract %float %225 1
%227 = OpFOrdEqual %bool %223 %226
OpBranch %220
%220 = OpLabel
%228 = OpPhi %bool %false %167 %227 %219
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%232 = OpLoad %v2float %231
%233 = OpCompositeExtract %float %232 0
%234 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%235 = OpLoad %v2float %234
%236 = OpCompositeExtract %float %235 0
%237 = OpFOrdEqual %bool %233 %236
OpBranch %230
%230 = OpLabel
%238 = OpPhi %bool %false %220 %237 %229
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%242 = OpLoad %v2float %241
%243 = OpCompositeExtract %float %242 1
%244 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%245 = OpLoad %v2float %244
%246 = OpCompositeExtract %float %245 1
%247 = OpFOrdEqual %bool %243 %246
OpBranch %240
%240 = OpLabel
%248 = OpPhi %bool %false %230 %247 %239
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
OpStore %251 %int_2
OpStore %252 %171
OpStore %253 %176
OpStore %254 %182
OpStore %255 %188
%256 = OpFSub %float %171 %float_1
%257 = OpFSub %float %176 %float_1
%258 = OpFSub %float %182 %float_1
%259 = OpFSub %float %188 %float_1
%260 = OpCompositeConstruct %v2float %256 %257
%261 = OpCompositeConstruct %v2float %258 %259
%262 = OpCompositeConstruct %mat2v2float %260 %261
OpStore %263 %262
%264 = OpFunctionCall %bool %test_bifffff22 %251 %252 %253 %254 %255 %263
OpBranch %250
%250 = OpLabel
%265 = OpPhi %bool %false %240 %264 %249
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
OpStore %269 %int_3
OpStore %270 %171
OpStore %271 %176
OpStore %272 %182
OpStore %273 %188
%274 = OpFMul %float %171 %float_2
%275 = OpFMul %float %176 %float_2
%276 = OpFMul %float %182 %float_2
%277 = OpFMul %float %188 %float_2
%278 = OpCompositeConstruct %v2float %274 %275
%279 = OpCompositeConstruct %v2float %276 %277
%280 = OpCompositeConstruct %mat2v2float %278 %279
OpStore %281 %280
%282 = OpFunctionCall %bool %test_bifffff22 %269 %270 %271 %272 %273 %281
OpBranch %267
%267 = OpLabel
%283 = OpPhi %bool %false %250 %282 %266
OpSelectionMerge %285 None
OpBranchConditional %283 %284 %285
%284 = OpLabel
OpStore %287 %int_4
OpStore %288 %171
OpStore %289 %176
OpStore %290 %182
OpStore %291 %188
%292 = OpFMul %float %171 %float_0_5
%293 = OpFMul %float %176 %float_0_5
%294 = OpFMul %float %182 %float_0_5
%295 = OpFMul %float %188 %float_0_5
%296 = OpCompositeConstruct %v2float %292 %293
%297 = OpCompositeConstruct %v2float %294 %295
%298 = OpCompositeConstruct %mat2v2float %296 %297
OpStore %299 %298
%300 = OpFunctionCall %bool %test_bifffff22 %287 %288 %289 %290 %291 %299
OpBranch %285
%285 = OpLabel
%301 = OpPhi %bool %false %267 %300 %284
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpFunctionCall %bool %divisionTest_b
OpBranch %303
%303 = OpLabel
%305 = OpPhi %bool %false %285 %304 %302
OpSelectionMerge %310 None
OpBranchConditional %305 %308 %309
%308 = OpLabel
%311 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%312 = OpLoad %v4float %311
OpStore %306 %312
OpBranch %310
%309 = OpLabel
%313 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%314 = OpLoad %v4float %313
OpStore %306 %314
OpBranch %310
%310 = OpLabel
%315 = OpLoad %v4float %306
OpReturnValue %315
OpFunctionEnd
