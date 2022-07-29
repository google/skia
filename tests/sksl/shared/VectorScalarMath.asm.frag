OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_int_b "test_int_b"
OpName %ok "ok"
OpName %inputRed "inputRed"
OpName %inputGreen "inputGreen"
OpName %x "x"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_inputRed "_1_inputRed"
OpName %_2_inputGreen "_2_inputGreen"
OpName %_3_x "_3_x"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%61 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%67 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%78 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%87 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%94 = OpConstantComposite %v3int %int_9 %int_9 %int_9
%100 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%106 = OpConstantComposite %v2int %int_3 %int_3
%112 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%117 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
%122 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%134 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%138 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%147 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%162 = OpConstantComposite %v2int %int_36 %int_36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%170 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%174 = OpConstantComposite %v4int %int_36 %int_36 %int_36 %int_36
%179 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%185 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%202 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%215 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%220 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%230 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%240 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%252 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%264 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%273 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%285 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%289 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%298 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_8 = OpConstant %float 8
%309 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_2
%float_32 = OpConstant %float 32
%315 = OpConstantComposite %v2float %float_32 %float_32
%float_16 = OpConstant %float 16
%322 = OpConstantComposite %v4float %float_4 %float_16 %float_8 %float_2
%326 = OpConstantComposite %v4float %float_32 %float_32 %float_32 %float_32
%331 = OpConstantComposite %v4float %float_2 %float_8 %float_16 %float_4
%337 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%33 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %33
%37 = OpCompositeExtract %float %36 0
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %36 1
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %36 2
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %36 3
%44 = OpConvertFToS %int %43
%45 = OpCompositeConstruct %v4int %38 %40 %42 %44
OpStore %inputRed %45
%47 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %inputGreen %58
%62 = OpIAdd %v4int %45 %61
OpStore %x %62
OpSelectionMerge %65 None
OpBranchConditional %true %64 %65
%64 = OpLabel
%68 = OpIEqual %v4bool %62 %67
%70 = OpAll %bool %68
OpBranch %65
%65 = OpLabel
%71 = OpPhi %bool %false %25 %70 %64
OpStore %ok %71
%72 = OpVectorShuffle %v4int %58 %58 1 3 0 2
%73 = OpISub %v4int %72 %61
OpStore %x %73
OpSelectionMerge %75 None
OpBranchConditional %71 %74 %75
%74 = OpLabel
%79 = OpIEqual %v4bool %73 %78
%80 = OpAll %bool %79
OpBranch %75
%75 = OpLabel
%81 = OpPhi %bool %false %65 %80 %74
OpStore %ok %81
%82 = OpCompositeExtract %int %58 1
%83 = OpCompositeConstruct %v4int %82 %82 %82 %82
%84 = OpIAdd %v4int %45 %83
OpStore %x %84
OpSelectionMerge %86 None
OpBranchConditional %81 %85 %86
%85 = OpLabel
%88 = OpIEqual %v4bool %84 %87
%89 = OpAll %bool %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %75 %89 %85
OpStore %ok %90
%91 = OpVectorShuffle %v3int %58 %58 3 1 3
%95 = OpIMul %v3int %91 %94
%96 = OpLoad %v4int %x
%97 = OpVectorShuffle %v4int %96 %95 4 5 6 3
OpStore %x %97
OpSelectionMerge %99 None
OpBranchConditional %90 %98 %99
%98 = OpLabel
%101 = OpIEqual %v4bool %97 %100
%102 = OpAll %bool %101
OpBranch %99
%99 = OpLabel
%103 = OpPhi %bool %false %86 %102 %98
OpStore %ok %103
%104 = OpVectorShuffle %v2int %97 %97 2 3
%107 = OpSDiv %v2int %104 %106
%108 = OpLoad %v4int %x
%109 = OpVectorShuffle %v4int %108 %107 4 5 2 3
OpStore %x %109
OpSelectionMerge %111 None
OpBranchConditional %103 %110 %111
%110 = OpLabel
%113 = OpIEqual %v4bool %109 %112
%114 = OpAll %bool %113
OpBranch %111
%111 = OpLabel
%115 = OpPhi %bool %false %99 %114 %110
OpStore %ok %115
%118 = OpIMul %v4int %45 %117
%119 = OpVectorShuffle %v4int %118 %118 1 0 3 2
OpStore %x %119
OpSelectionMerge %121 None
OpBranchConditional %115 %120 %121
%120 = OpLabel
%123 = OpIEqual %v4bool %119 %122
%124 = OpAll %bool %123
OpBranch %121
%121 = OpLabel
%125 = OpPhi %bool %false %111 %124 %120
OpStore %ok %125
%126 = OpIAdd %v4int %61 %45
OpStore %x %126
OpSelectionMerge %128 None
OpBranchConditional %125 %127 %128
%127 = OpLabel
%129 = OpIEqual %v4bool %126 %67
%130 = OpAll %bool %129
OpBranch %128
%128 = OpLabel
%131 = OpPhi %bool %false %121 %130 %127
OpStore %ok %131
%133 = OpVectorShuffle %v4int %58 %58 1 3 0 2
%135 = OpISub %v4int %134 %133
OpStore %x %135
OpSelectionMerge %137 None
OpBranchConditional %131 %136 %137
%136 = OpLabel
%139 = OpIEqual %v4bool %135 %138
%140 = OpAll %bool %139
OpBranch %137
%137 = OpLabel
%141 = OpPhi %bool %false %128 %140 %136
OpStore %ok %141
%142 = OpCompositeExtract %int %45 0
%143 = OpCompositeConstruct %v4int %142 %142 %142 %142
%144 = OpIAdd %v4int %143 %58
OpStore %x %144
OpSelectionMerge %146 None
OpBranchConditional %141 %145 %146
%145 = OpLabel
%148 = OpIEqual %v4bool %144 %147
%149 = OpAll %bool %148
OpBranch %146
%146 = OpLabel
%150 = OpPhi %bool %false %137 %149 %145
OpStore %ok %150
%151 = OpVectorShuffle %v3int %58 %58 3 1 3
%152 = OpIMul %v3int %94 %151
%153 = OpLoad %v4int %x
%154 = OpVectorShuffle %v4int %153 %152 4 5 6 3
OpStore %x %154
OpSelectionMerge %156 None
OpBranchConditional %150 %155 %156
%155 = OpLabel
%157 = OpIEqual %v4bool %154 %100
%158 = OpAll %bool %157
OpBranch %156
%156 = OpLabel
%159 = OpPhi %bool %false %146 %158 %155
OpStore %ok %159
%161 = OpVectorShuffle %v2int %154 %154 2 3
%163 = OpSDiv %v2int %162 %161
%164 = OpLoad %v4int %x
%165 = OpVectorShuffle %v4int %164 %163 4 5 2 3
OpStore %x %165
OpSelectionMerge %167 None
OpBranchConditional %159 %166 %167
%166 = OpLabel
%171 = OpIEqual %v4bool %165 %170
%172 = OpAll %bool %171
OpBranch %167
%167 = OpLabel
%173 = OpPhi %bool %false %156 %172 %166
OpStore %ok %173
%175 = OpSDiv %v4int %174 %165
%176 = OpVectorShuffle %v4int %175 %175 1 0 3 2
OpStore %x %176
OpSelectionMerge %178 None
OpBranchConditional %173 %177 %178
%177 = OpLabel
%180 = OpIEqual %v4bool %176 %179
%181 = OpAll %bool %180
OpBranch %178
%178 = OpLabel
%182 = OpPhi %bool %false %167 %181 %177
OpStore %ok %182
%183 = OpIAdd %v4int %176 %61
OpStore %x %183
%184 = OpIMul %v4int %183 %61
OpStore %x %184
%186 = OpISub %v4int %184 %185
OpStore %x %186
%187 = OpSDiv %v4int %186 %61
OpStore %x %187
OpSelectionMerge %189 None
OpBranchConditional %182 %188 %189
%188 = OpLabel
%190 = OpIEqual %v4bool %187 %179
%191 = OpAll %bool %190
OpBranch %189
%189 = OpLabel
%192 = OpPhi %bool %false %178 %191 %188
OpStore %ok %192
%193 = OpIAdd %v4int %187 %61
OpStore %x %193
%194 = OpIMul %v4int %193 %61
OpStore %x %194
%195 = OpISub %v4int %194 %185
OpStore %x %195
%196 = OpSDiv %v4int %195 %61
OpStore %x %196
OpSelectionMerge %198 None
OpBranchConditional %192 %197 %198
%197 = OpLabel
%199 = OpIEqual %v4bool %196 %179
%200 = OpAll %bool %199
OpBranch %198
%198 = OpLabel
%201 = OpPhi %bool %false %189 %200 %197
OpStore %ok %201
OpReturnValue %201
OpFunctionEnd
%main = OpFunction %v4float None %202
%203 = OpFunctionParameter %_ptr_Function_v2float
%204 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%360 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%208 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%209 = OpLoad %v4float %208
OpStore %_1_inputRed %209
%211 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%212 = OpLoad %v4float %211
OpStore %_2_inputGreen %212
%216 = OpFAdd %v4float %209 %215
OpStore %_3_x %216
OpSelectionMerge %218 None
OpBranchConditional %true %217 %218
%217 = OpLabel
%221 = OpFOrdEqual %v4bool %216 %220
%222 = OpAll %bool %221
OpBranch %218
%218 = OpLabel
%223 = OpPhi %bool %false %204 %222 %217
OpStore %_0_ok %223
%224 = OpVectorShuffle %v4float %212 %212 1 3 0 2
%225 = OpFSub %v4float %224 %215
OpStore %_3_x %225
OpSelectionMerge %227 None
OpBranchConditional %223 %226 %227
%226 = OpLabel
%231 = OpFOrdEqual %v4bool %225 %230
%232 = OpAll %bool %231
OpBranch %227
%227 = OpLabel
%233 = OpPhi %bool %false %218 %232 %226
OpStore %_0_ok %233
%234 = OpCompositeExtract %float %212 1
%235 = OpCompositeConstruct %v4float %234 %234 %234 %234
%236 = OpFAdd %v4float %209 %235
OpStore %_3_x %236
OpSelectionMerge %238 None
OpBranchConditional %233 %237 %238
%237 = OpLabel
%241 = OpFOrdEqual %v4bool %236 %240
%242 = OpAll %bool %241
OpBranch %238
%238 = OpLabel
%243 = OpPhi %bool %false %227 %242 %237
OpStore %_0_ok %243
%244 = OpVectorShuffle %v3float %212 %212 3 1 3
%247 = OpVectorTimesScalar %v3float %244 %float_9
%248 = OpLoad %v4float %_3_x
%249 = OpVectorShuffle %v4float %248 %247 4 5 6 3
OpStore %_3_x %249
OpSelectionMerge %251 None
OpBranchConditional %243 %250 %251
%250 = OpLabel
%253 = OpFOrdEqual %v4bool %249 %252
%254 = OpAll %bool %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %238 %254 %250
OpStore %_0_ok %255
%256 = OpVectorShuffle %v2float %249 %249 2 3
%257 = OpVectorTimesScalar %v2float %256 %float_2
%258 = OpLoad %v4float %_3_x
%259 = OpVectorShuffle %v4float %258 %257 4 5 2 3
OpStore %_3_x %259
OpSelectionMerge %261 None
OpBranchConditional %255 %260 %261
%260 = OpLabel
%265 = OpFOrdEqual %v4bool %259 %264
%266 = OpAll %bool %265
OpBranch %261
%261 = OpLabel
%267 = OpPhi %bool %false %251 %266 %260
OpStore %_0_ok %267
%269 = OpVectorTimesScalar %v4float %209 %float_5
%270 = OpVectorShuffle %v4float %269 %269 1 0 3 2
OpStore %_3_x %270
OpSelectionMerge %272 None
OpBranchConditional %267 %271 %272
%271 = OpLabel
%274 = OpFOrdEqual %v4bool %270 %273
%275 = OpAll %bool %274
OpBranch %272
%272 = OpLabel
%276 = OpPhi %bool %false %261 %275 %271
OpStore %_0_ok %276
%277 = OpFAdd %v4float %215 %209
OpStore %_3_x %277
OpSelectionMerge %279 None
OpBranchConditional %276 %278 %279
%278 = OpLabel
%280 = OpFOrdEqual %v4bool %277 %220
%281 = OpAll %bool %280
OpBranch %279
%279 = OpLabel
%282 = OpPhi %bool %false %272 %281 %278
OpStore %_0_ok %282
%284 = OpVectorShuffle %v4float %212 %212 1 3 0 2
%286 = OpFSub %v4float %285 %284
OpStore %_3_x %286
OpSelectionMerge %288 None
OpBranchConditional %282 %287 %288
%287 = OpLabel
%290 = OpFOrdEqual %v4bool %286 %289
%291 = OpAll %bool %290
OpBranch %288
%288 = OpLabel
%292 = OpPhi %bool %false %279 %291 %287
OpStore %_0_ok %292
%293 = OpCompositeExtract %float %209 0
%294 = OpCompositeConstruct %v4float %293 %293 %293 %293
%295 = OpFAdd %v4float %294 %212
OpStore %_3_x %295
OpSelectionMerge %297 None
OpBranchConditional %292 %296 %297
%296 = OpLabel
%299 = OpFOrdEqual %v4bool %295 %298
%300 = OpAll %bool %299
OpBranch %297
%297 = OpLabel
%301 = OpPhi %bool %false %288 %300 %296
OpStore %_0_ok %301
%303 = OpVectorShuffle %v3float %212 %212 3 1 3
%304 = OpVectorTimesScalar %v3float %303 %float_8
%305 = OpLoad %v4float %_3_x
%306 = OpVectorShuffle %v4float %305 %304 4 5 6 3
OpStore %_3_x %306
OpSelectionMerge %308 None
OpBranchConditional %301 %307 %308
%307 = OpLabel
%310 = OpFOrdEqual %v4bool %306 %309
%311 = OpAll %bool %310
OpBranch %308
%308 = OpLabel
%312 = OpPhi %bool %false %297 %311 %307
OpStore %_0_ok %312
%314 = OpVectorShuffle %v2float %306 %306 2 3
%316 = OpFDiv %v2float %315 %314
%317 = OpLoad %v4float %_3_x
%318 = OpVectorShuffle %v4float %317 %316 4 5 2 3
OpStore %_3_x %318
OpSelectionMerge %320 None
OpBranchConditional %312 %319 %320
%319 = OpLabel
%323 = OpFOrdEqual %v4bool %318 %322
%324 = OpAll %bool %323
OpBranch %320
%320 = OpLabel
%325 = OpPhi %bool %false %308 %324 %319
OpStore %_0_ok %325
%327 = OpFDiv %v4float %326 %318
%328 = OpVectorShuffle %v4float %327 %327 1 0 3 2
OpStore %_3_x %328
OpSelectionMerge %330 None
OpBranchConditional %325 %329 %330
%329 = OpLabel
%332 = OpFOrdEqual %v4bool %328 %331
%333 = OpAll %bool %332
OpBranch %330
%330 = OpLabel
%334 = OpPhi %bool %false %320 %333 %329
OpStore %_0_ok %334
%335 = OpFAdd %v4float %328 %215
OpStore %_3_x %335
%336 = OpVectorTimesScalar %v4float %335 %float_2
OpStore %_3_x %336
%338 = OpFSub %v4float %336 %337
OpStore %_3_x %338
%339 = OpFDiv %float %float_1 %float_2
%340 = OpVectorTimesScalar %v4float %338 %339
OpStore %_3_x %340
OpSelectionMerge %342 None
OpBranchConditional %334 %341 %342
%341 = OpLabel
%343 = OpFOrdEqual %v4bool %340 %331
%344 = OpAll %bool %343
OpBranch %342
%342 = OpLabel
%345 = OpPhi %bool %false %330 %344 %341
OpStore %_0_ok %345
%346 = OpFAdd %v4float %340 %215
OpStore %_3_x %346
%347 = OpVectorTimesScalar %v4float %346 %float_2
OpStore %_3_x %347
%348 = OpFSub %v4float %347 %337
OpStore %_3_x %348
%350 = OpVectorTimesScalar %v4float %348 %float_0_5
OpStore %_3_x %350
OpSelectionMerge %352 None
OpBranchConditional %345 %351 %352
%351 = OpLabel
%353 = OpFOrdEqual %v4bool %350 %331
%354 = OpAll %bool %353
OpBranch %352
%352 = OpLabel
%355 = OpPhi %bool %false %342 %354 %351
OpStore %_0_ok %355
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%358 = OpFunctionCall %bool %test_int_b
OpBranch %357
%357 = OpLabel
%359 = OpPhi %bool %false %352 %358 %356
OpSelectionMerge %363 None
OpBranchConditional %359 %361 %362
%361 = OpLabel
%364 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%365 = OpLoad %v4float %364
OpStore %360 %365
OpBranch %363
%362 = OpLabel
%366 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%367 = OpLoad %v4float %366
OpStore %360 %367
OpBranch %363
%363 = OpLabel
%368 = OpLoad %v4float %360
OpReturnValue %368
OpFunctionEnd
