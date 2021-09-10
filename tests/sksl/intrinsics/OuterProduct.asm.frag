OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %c12 "c12"
OpName %c34 "c34"
OpName %c123 "c123"
OpName %c456 "c456"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 4 Offset 112
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %205 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%32 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%36 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Function_v3float = OpTypePointer Function %v3float
%39 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%43 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int_1 = OpConstant %int 1
%float_8 = OpConstant %float 8
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%float_18 = OpConstant %float 18
%v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_4 = OpConstant %int 4
%210 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%float_n2_5 = OpConstant %float -2.5
%float_1_5 = OpConstant %float 1.5
%float_4_5 = OpConstant %float 4.5
%v4bool = OpTypeVector %bool 4
%247 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%mat2v4float = OpTypeMatrix %v4float 2
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%c12 = OpVariable %_ptr_Function_v2float Function
%c34 = OpVariable %_ptr_Function_v2float Function
%c123 = OpVariable %_ptr_Function_v3float Function
%c456 = OpVariable %_ptr_Function_v3float Function
%325 = OpVariable %_ptr_Function_v4float Function
OpStore %c12 %32
OpStore %c34 %36
OpStore %c123 %39
OpStore %c456 %43
%46 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%51 = OpAccessChain %_ptr_Uniform_v2float %46 %int_0
%53 = OpLoad %v2float %51
%54 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%56 = OpAccessChain %_ptr_Uniform_v2float %54 %int_1
%57 = OpLoad %v2float %56
%45 = OpOuterProduct %mat2v2float %53 %57
%59 = OpCompositeConstruct %v2float %float_3 %float_6
%60 = OpCompositeConstruct %v2float %float_4 %float_8
%61 = OpCompositeConstruct %mat2v2float %59 %60
%63 = OpCompositeExtract %v2float %45 0
%64 = OpCompositeExtract %v2float %61 0
%65 = OpFOrdEqual %v2bool %63 %64
%66 = OpAll %bool %65
%67 = OpCompositeExtract %v2float %45 1
%68 = OpCompositeExtract %v2float %61 1
%69 = OpFOrdEqual %v2bool %67 %68
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %66 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpLoad %v2float %c12
%76 = OpLoad %v2float %c34
%74 = OpOuterProduct %mat2v2float %75 %76
%77 = OpCompositeConstruct %v2float %float_3 %float_6
%78 = OpCompositeConstruct %v2float %float_4 %float_8
%79 = OpCompositeConstruct %mat2v2float %77 %78
%80 = OpCompositeExtract %v2float %74 0
%81 = OpCompositeExtract %v2float %79 0
%82 = OpFOrdEqual %v2bool %80 %81
%83 = OpAll %bool %82
%84 = OpCompositeExtract %v2float %74 1
%85 = OpCompositeExtract %v2float %79 1
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
%88 = OpLogicalAnd %bool %83 %87
OpBranch %73
%73 = OpLabel
%89 = OpPhi %bool %false %28 %88 %72
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%96 = OpAccessChain %_ptr_Uniform_v3float %93 %int_0
%98 = OpLoad %v3float %96
%99 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%100 = OpAccessChain %_ptr_Uniform_v3float %99 %int_1
%101 = OpLoad %v3float %100
%92 = OpOuterProduct %mat3v3float %98 %101
%106 = OpCompositeConstruct %v3float %float_4 %float_8 %float_12
%107 = OpCompositeConstruct %v3float %float_5 %float_10 %float_15
%108 = OpCompositeConstruct %v3float %float_6 %float_12 %float_18
%109 = OpCompositeConstruct %mat3v3float %106 %107 %108
%111 = OpCompositeExtract %v3float %92 0
%112 = OpCompositeExtract %v3float %109 0
%113 = OpFOrdEqual %v3bool %111 %112
%114 = OpAll %bool %113
%115 = OpCompositeExtract %v3float %92 1
%116 = OpCompositeExtract %v3float %109 1
%117 = OpFOrdEqual %v3bool %115 %116
%118 = OpAll %bool %117
%119 = OpLogicalAnd %bool %114 %118
%120 = OpCompositeExtract %v3float %92 2
%121 = OpCompositeExtract %v3float %109 2
%122 = OpFOrdEqual %v3bool %120 %121
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %119 %123
OpBranch %91
%91 = OpLabel
%125 = OpPhi %bool %false %73 %124 %90
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v3float %c123
%130 = OpLoad %v3float %c456
%128 = OpOuterProduct %mat3v3float %129 %130
%131 = OpCompositeConstruct %v3float %float_4 %float_8 %float_12
%132 = OpCompositeConstruct %v3float %float_5 %float_10 %float_15
%133 = OpCompositeConstruct %v3float %float_6 %float_12 %float_18
%134 = OpCompositeConstruct %mat3v3float %131 %132 %133
%135 = OpCompositeExtract %v3float %128 0
%136 = OpCompositeExtract %v3float %134 0
%137 = OpFOrdEqual %v3bool %135 %136
%138 = OpAll %bool %137
%139 = OpCompositeExtract %v3float %128 1
%140 = OpCompositeExtract %v3float %134 1
%141 = OpFOrdEqual %v3bool %139 %140
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %138 %142
%144 = OpCompositeExtract %v3float %128 2
%145 = OpCompositeExtract %v3float %134 2
%146 = OpFOrdEqual %v3bool %144 %145
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %143 %147
OpBranch %127
%127 = OpLabel
%149 = OpPhi %bool %false %91 %148 %126
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%154 = OpAccessChain %_ptr_Uniform_v2float %153 %int_0
%155 = OpLoad %v2float %154
%156 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%157 = OpAccessChain %_ptr_Uniform_v3float %156 %int_1
%158 = OpLoad %v3float %157
%152 = OpOuterProduct %mat3v2float %155 %158
%160 = OpCompositeConstruct %v2float %float_4 %float_8
%161 = OpCompositeConstruct %v2float %float_5 %float_10
%162 = OpCompositeConstruct %v2float %float_6 %float_12
%163 = OpCompositeConstruct %mat3v2float %160 %161 %162
%164 = OpCompositeExtract %v2float %152 0
%165 = OpCompositeExtract %v2float %163 0
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %152 1
%169 = OpCompositeExtract %v2float %163 1
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
%173 = OpCompositeExtract %v2float %152 2
%174 = OpCompositeExtract %v2float %163 2
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
OpBranch %151
%151 = OpLabel
%178 = OpPhi %bool %false %127 %177 %150
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%182 = OpLoad %v2float %c12
%183 = OpLoad %v3float %c456
%181 = OpOuterProduct %mat3v2float %182 %183
%184 = OpCompositeConstruct %v2float %float_4 %float_8
%185 = OpCompositeConstruct %v2float %float_5 %float_10
%186 = OpCompositeConstruct %v2float %float_6 %float_12
%187 = OpCompositeConstruct %mat3v2float %184 %185 %186
%188 = OpCompositeExtract %v2float %181 0
%189 = OpCompositeExtract %v2float %187 0
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v2float %181 1
%193 = OpCompositeExtract %v2float %187 1
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
%197 = OpCompositeExtract %v2float %181 2
%198 = OpCompositeExtract %v2float %187 2
%199 = OpFOrdEqual %v2bool %197 %198
%200 = OpAll %bool %199
%201 = OpLogicalAnd %bool %196 %200
OpBranch %180
%180 = OpLabel
%202 = OpPhi %bool %false %151 %201 %179
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%209 = OpLoad %v4float %206
%205 = OpOuterProduct %mat4v4float %209 %210
%218 = OpCompositeConstruct %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%219 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%220 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%221 = OpCompositeConstruct %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%222 = OpCompositeConstruct %mat4v4float %218 %219 %220 %221
%224 = OpCompositeExtract %v4float %205 0
%225 = OpCompositeExtract %v4float %222 0
%226 = OpFOrdEqual %v4bool %224 %225
%227 = OpAll %bool %226
%228 = OpCompositeExtract %v4float %205 1
%229 = OpCompositeExtract %v4float %222 1
%230 = OpFOrdEqual %v4bool %228 %229
%231 = OpAll %bool %230
%232 = OpLogicalAnd %bool %227 %231
%233 = OpCompositeExtract %v4float %205 2
%234 = OpCompositeExtract %v4float %222 2
%235 = OpFOrdEqual %v4bool %233 %234
%236 = OpAll %bool %235
%237 = OpLogicalAnd %bool %232 %236
%238 = OpCompositeExtract %v4float %205 3
%239 = OpCompositeExtract %v4float %222 3
%240 = OpFOrdEqual %v4bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
OpBranch %204
%204 = OpLabel
%243 = OpPhi %bool %false %180 %242 %203
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpOuterProduct %mat4v4float %247 %210
%248 = OpCompositeConstruct %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%249 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%250 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%251 = OpCompositeConstruct %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%252 = OpCompositeConstruct %mat4v4float %248 %249 %250 %251
%253 = OpCompositeExtract %v4float %246 0
%254 = OpCompositeExtract %v4float %252 0
%255 = OpFOrdEqual %v4bool %253 %254
%256 = OpAll %bool %255
%257 = OpCompositeExtract %v4float %246 1
%258 = OpCompositeExtract %v4float %252 1
%259 = OpFOrdEqual %v4bool %257 %258
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %256 %260
%262 = OpCompositeExtract %v4float %246 2
%263 = OpCompositeExtract %v4float %252 2
%264 = OpFOrdEqual %v4bool %262 %263
%265 = OpAll %bool %264
%266 = OpLogicalAnd %bool %261 %265
%267 = OpCompositeExtract %v4float %246 3
%268 = OpCompositeExtract %v4float %252 3
%269 = OpFOrdEqual %v4bool %267 %268
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %266 %270
OpBranch %245
%245 = OpLabel
%272 = OpPhi %bool %false %204 %271 %244
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%276 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%277 = OpLoad %v4float %276
%278 = OpLoad %v2float %c12
%275 = OpOuterProduct %mat2v4float %277 %278
%280 = OpCompositeConstruct %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%281 = OpCompositeConstruct %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%282 = OpCompositeConstruct %mat2v4float %280 %281
%283 = OpCompositeExtract %v4float %275 0
%284 = OpCompositeExtract %v4float %282 0
%285 = OpFOrdEqual %v4bool %283 %284
%286 = OpAll %bool %285
%287 = OpCompositeExtract %v4float %275 1
%288 = OpCompositeExtract %v4float %282 1
%289 = OpFOrdEqual %v4bool %287 %288
%290 = OpAll %bool %289
%291 = OpLogicalAnd %bool %286 %290
OpBranch %274
%274 = OpLabel
%292 = OpPhi %bool %false %245 %291 %273
OpSelectionMerge %294 None
OpBranchConditional %292 %293 %294
%293 = OpLabel
%296 = OpLoad %v2float %c12
%297 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%298 = OpLoad %v4float %297
%295 = OpOuterProduct %mat4v2float %296 %298
%300 = OpCompositeConstruct %v2float %float_n1_25 %float_n2_5
%301 = OpCompositeConstruct %v2float %float_0 %float_0
%302 = OpCompositeConstruct %v2float %float_0_75 %float_1_5
%303 = OpCompositeConstruct %v2float %float_2_25 %float_4_5
%304 = OpCompositeConstruct %mat4v2float %300 %301 %302 %303
%305 = OpCompositeExtract %v2float %295 0
%306 = OpCompositeExtract %v2float %304 0
%307 = OpFOrdEqual %v2bool %305 %306
%308 = OpAll %bool %307
%309 = OpCompositeExtract %v2float %295 1
%310 = OpCompositeExtract %v2float %304 1
%311 = OpFOrdEqual %v2bool %309 %310
%312 = OpAll %bool %311
%313 = OpLogicalAnd %bool %308 %312
%314 = OpCompositeExtract %v2float %295 2
%315 = OpCompositeExtract %v2float %304 2
%316 = OpFOrdEqual %v2bool %314 %315
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %313 %317
%319 = OpCompositeExtract %v2float %295 3
%320 = OpCompositeExtract %v2float %304 3
%321 = OpFOrdEqual %v2bool %319 %320
%322 = OpAll %bool %321
%323 = OpLogicalAnd %bool %318 %322
OpBranch %294
%294 = OpLabel
%324 = OpPhi %bool %false %274 %323 %293
OpSelectionMerge %329 None
OpBranchConditional %324 %327 %328
%327 = OpLabel
%330 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%331 = OpLoad %v4float %330
OpStore %325 %331
OpBranch %329
%328 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%333 = OpLoad %v4float %332
OpStore %325 %333
OpBranch %329
%329 = OpLabel
%334 = OpLoad %v4float %325
OpReturnValue %334
OpFunctionEnd
