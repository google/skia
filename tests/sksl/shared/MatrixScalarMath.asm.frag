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
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
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
%143 = OpConstantComposite %v2float %float_n8 %float_n8
%144 = OpConstantComposite %mat2v2float %143 %143
%v2bool = OpTypeVector %bool 2
%163 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%135 = OpFDiv %float %float_1 %134
%136 = OpMatrixTimesScalar %mat2v2float %129 %135
OpStore %div %136
%137 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%138 = OpLoad %v4float %137
%139 = OpCompositeExtract %float %138 0
%140 = OpFDiv %float %float_1 %139
%141 = OpMatrixTimesScalar %mat2v2float %129 %140
OpStore %mat %141
%146 = OpCompositeExtract %v2float %136 0
%147 = OpFOrdEqual %v2bool %146 %143
%148 = OpAll %bool %147
%149 = OpCompositeExtract %v2float %136 1
%150 = OpFOrdEqual %v2bool %149 %143
%151 = OpAll %bool %150
%152 = OpLogicalAnd %bool %148 %151
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpCompositeExtract %v2float %141 0
%156 = OpFOrdEqual %v2bool %155 %143
%157 = OpAll %bool %156
%158 = OpCompositeExtract %v2float %141 1
%159 = OpFOrdEqual %v2bool %158 %143
%160 = OpAll %bool %159
%161 = OpLogicalAnd %bool %157 %160
OpBranch %154
%154 = OpLabel
%162 = OpPhi %bool %false %120 %161 %153
OpReturnValue %162
OpFunctionEnd
%main = OpFunction %v4float None %163
%164 = OpFunctionParameter %_ptr_Function_v2float
%165 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%249 = OpVariable %_ptr_Function_int Function
%250 = OpVariable %_ptr_Function_float Function
%251 = OpVariable %_ptr_Function_float Function
%252 = OpVariable %_ptr_Function_float Function
%253 = OpVariable %_ptr_Function_float Function
%261 = OpVariable %_ptr_Function_mat2v2float Function
%267 = OpVariable %_ptr_Function_int Function
%268 = OpVariable %_ptr_Function_float Function
%269 = OpVariable %_ptr_Function_float Function
%270 = OpVariable %_ptr_Function_float Function
%271 = OpVariable %_ptr_Function_float Function
%279 = OpVariable %_ptr_Function_mat2v2float Function
%285 = OpVariable %_ptr_Function_int Function
%286 = OpVariable %_ptr_Function_float Function
%287 = OpVariable %_ptr_Function_float Function
%288 = OpVariable %_ptr_Function_float Function
%289 = OpVariable %_ptr_Function_float Function
%297 = OpVariable %_ptr_Function_mat2v2float Function
%304 = OpVariable %_ptr_Function_v4float Function
%167 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%168 = OpLoad %v4float %167
%169 = OpCompositeExtract %float %168 1
OpStore %f1 %169
%171 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%172 = OpLoad %v4float %171
%173 = OpCompositeExtract %float %172 1
%174 = OpFMul %float %float_2 %173
OpStore %f2 %174
%177 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%178 = OpLoad %v4float %177
%179 = OpCompositeExtract %float %178 1
%180 = OpFMul %float %float_3 %179
OpStore %f3 %180
%183 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%184 = OpLoad %v4float %183
%185 = OpCompositeExtract %float %184 1
%186 = OpFMul %float %float_4 %185
OpStore %f4 %186
%188 = OpFAdd %float %169 %float_1
%189 = OpFAdd %float %174 %float_1
%190 = OpFAdd %float %180 %float_1
%191 = OpFAdd %float %186 %float_1
%192 = OpCompositeConstruct %v2float %188 %189
%193 = OpCompositeConstruct %v2float %190 %191
%194 = OpCompositeConstruct %mat2v2float %192 %193
OpStore %_0_expected %194
%196 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%197 = OpLoad %v4float %196
%198 = OpCompositeExtract %float %197 0
OpStore %_1_one %198
%200 = OpFMul %float %169 %198
%201 = OpFMul %float %174 %198
%202 = OpFMul %float %180 %198
%203 = OpFMul %float %186 %198
%204 = OpCompositeConstruct %v2float %200 %201
%205 = OpCompositeConstruct %v2float %202 %203
%206 = OpCompositeConstruct %mat2v2float %204 %205
OpStore %_2_m2 %206
%207 = OpFAdd %v2float %204 %63
%208 = OpFAdd %v2float %205 %63
%209 = OpCompositeConstruct %mat2v2float %207 %208
OpStore %_2_m2 %209
%210 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%211 = OpLoad %v2float %210
%212 = OpCompositeExtract %float %211 0
%213 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%214 = OpLoad %v2float %213
%215 = OpCompositeExtract %float %214 0
%216 = OpFOrdEqual %bool %212 %215
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%220 = OpLoad %v2float %219
%221 = OpCompositeExtract %float %220 1
%222 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%223 = OpLoad %v2float %222
%224 = OpCompositeExtract %float %223 1
%225 = OpFOrdEqual %bool %221 %224
OpBranch %218
%218 = OpLabel
%226 = OpPhi %bool %false %165 %225 %217
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%230 = OpLoad %v2float %229
%231 = OpCompositeExtract %float %230 0
%232 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%233 = OpLoad %v2float %232
%234 = OpCompositeExtract %float %233 0
%235 = OpFOrdEqual %bool %231 %234
OpBranch %228
%228 = OpLabel
%236 = OpPhi %bool %false %218 %235 %227
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%239 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%240 = OpLoad %v2float %239
%241 = OpCompositeExtract %float %240 1
%242 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%243 = OpLoad %v2float %242
%244 = OpCompositeExtract %float %243 1
%245 = OpFOrdEqual %bool %241 %244
OpBranch %238
%238 = OpLabel
%246 = OpPhi %bool %false %228 %245 %237
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
OpStore %249 %int_2
OpStore %250 %169
OpStore %251 %174
OpStore %252 %180
OpStore %253 %186
%254 = OpFSub %float %169 %float_1
%255 = OpFSub %float %174 %float_1
%256 = OpFSub %float %180 %float_1
%257 = OpFSub %float %186 %float_1
%258 = OpCompositeConstruct %v2float %254 %255
%259 = OpCompositeConstruct %v2float %256 %257
%260 = OpCompositeConstruct %mat2v2float %258 %259
OpStore %261 %260
%262 = OpFunctionCall %bool %test_bifffff22 %249 %250 %251 %252 %253 %261
OpBranch %248
%248 = OpLabel
%263 = OpPhi %bool %false %238 %262 %247
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
OpStore %267 %int_3
OpStore %268 %169
OpStore %269 %174
OpStore %270 %180
OpStore %271 %186
%272 = OpFMul %float %169 %float_2
%273 = OpFMul %float %174 %float_2
%274 = OpFMul %float %180 %float_2
%275 = OpFMul %float %186 %float_2
%276 = OpCompositeConstruct %v2float %272 %273
%277 = OpCompositeConstruct %v2float %274 %275
%278 = OpCompositeConstruct %mat2v2float %276 %277
OpStore %279 %278
%280 = OpFunctionCall %bool %test_bifffff22 %267 %268 %269 %270 %271 %279
OpBranch %265
%265 = OpLabel
%281 = OpPhi %bool %false %248 %280 %264
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
OpStore %285 %int_4
OpStore %286 %169
OpStore %287 %174
OpStore %288 %180
OpStore %289 %186
%290 = OpFMul %float %169 %float_0_5
%291 = OpFMul %float %174 %float_0_5
%292 = OpFMul %float %180 %float_0_5
%293 = OpFMul %float %186 %float_0_5
%294 = OpCompositeConstruct %v2float %290 %291
%295 = OpCompositeConstruct %v2float %292 %293
%296 = OpCompositeConstruct %mat2v2float %294 %295
OpStore %297 %296
%298 = OpFunctionCall %bool %test_bifffff22 %285 %286 %287 %288 %289 %297
OpBranch %283
%283 = OpLabel
%299 = OpPhi %bool %false %265 %298 %282
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpFunctionCall %bool %divisionTest_b
OpBranch %301
%301 = OpLabel
%303 = OpPhi %bool %false %283 %302 %300
OpSelectionMerge %308 None
OpBranchConditional %303 %306 %307
%306 = OpLabel
%309 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%310 = OpLoad %v4float %309
OpStore %304 %310
OpBranch %308
%307 = OpLabel
%311 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%312 = OpLoad %v4float %311
OpStore %304 %312
OpBranch %308
%308 = OpLabel
%313 = OpLoad %v4float %304
OpReturnValue %313
OpFunctionEnd
