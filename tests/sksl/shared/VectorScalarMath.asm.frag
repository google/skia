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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %inputRed "inputRed"
OpName %inputGreen "inputGreen"
OpName %x "x"
OpName %test_int_b "test_int_b"
OpName %ok_0 "ok"
OpName %inputRed_0 "inputRed"
OpName %inputGreen_0 "inputGreen"
OpName %x_0 "x"
OpName %main "main"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %inputRed RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %inputGreen RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %x RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%43 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%49 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%v4bool = OpTypeVector %bool 4
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%60 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%70 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%82 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%94 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%103 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%115 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%119 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%128 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_8 = OpConstant %float 8
%139 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_2
%float_32 = OpConstant %float 32
%145 = OpConstantComposite %v2float %float_32 %float_32
%float_16 = OpConstant %float 16
%152 = OpConstantComposite %v4float %float_4 %float_16 %float_8 %float_2
%156 = OpConstantComposite %v4float %float_32 %float_32 %float_32 %float_32
%161 = OpConstantComposite %v4float %float_2 %float_8 %float_16 %float_4
%167 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_5 = OpConstant %float 0.5
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_2 = OpConstant %int 2
%216 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%int_3 = OpConstant %int 3
%221 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%231 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%240 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%247 = OpConstantComposite %v3int %int_9 %int_9 %int_9
%253 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%int_4 = OpConstant %int 4
%260 = OpConstantComposite %v2int %int_4 %int_4
%266 = OpConstantComposite %v4int %int_2 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%271 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
%276 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%288 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%292 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%301 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_8 = OpConstant %int 8
%307 = OpConstantComposite %v3int %int_8 %int_8 %int_8
%313 = OpConstantComposite %v4int %int_8 %int_8 %int_8 %int_2
%int_36 = OpConstant %int 36
%319 = OpConstantComposite %v2int %int_36 %int_36
%int_18 = OpConstant %int 18
%326 = OpConstantComposite %v4int %int_4 %int_18 %int_8 %int_2
%int_37 = OpConstant %int 37
%331 = OpConstantComposite %v4int %int_37 %int_37 %int_37 %int_37
%336 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%342 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%359 = OpTypeFunction %v4float %_ptr_Function_v2float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4float Function
%inputGreen = OpVariable %_ptr_Function_v4float Function
%x = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%32 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%36 = OpLoad %v4float %32
OpStore %inputRed %36
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%40 = OpLoad %v4float %38
OpStore %inputGreen %40
%44 = OpFAdd %v4float %36 %43
OpStore %x %44
OpSelectionMerge %47 None
OpBranchConditional %true %46 %47
%46 = OpLabel
%50 = OpFOrdEqual %v4bool %44 %49
%52 = OpAll %bool %50
OpBranch %47
%47 = OpLabel
%53 = OpPhi %bool %false %26 %52 %46
OpStore %ok %53
%54 = OpVectorShuffle %v4float %40 %40 1 3 0 2
%55 = OpFSub %v4float %54 %43
OpStore %x %55
OpSelectionMerge %57 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
%61 = OpFOrdEqual %v4bool %55 %60
%62 = OpAll %bool %61
OpBranch %57
%57 = OpLabel
%63 = OpPhi %bool %false %47 %62 %56
OpStore %ok %63
%64 = OpCompositeExtract %float %40 1
%65 = OpCompositeConstruct %v4float %64 %64 %64 %64
%66 = OpFAdd %v4float %36 %65
OpStore %x %66
OpSelectionMerge %68 None
OpBranchConditional %63 %67 %68
%67 = OpLabel
%71 = OpFOrdEqual %v4bool %66 %70
%72 = OpAll %bool %71
OpBranch %68
%68 = OpLabel
%73 = OpPhi %bool %false %57 %72 %67
OpStore %ok %73
%74 = OpVectorShuffle %v3float %40 %40 3 1 3
%77 = OpVectorTimesScalar %v3float %74 %float_9
%78 = OpLoad %v4float %x
%79 = OpVectorShuffle %v4float %78 %77 4 5 6 3
OpStore %x %79
OpSelectionMerge %81 None
OpBranchConditional %73 %80 %81
%80 = OpLabel
%83 = OpFOrdEqual %v4bool %79 %82
%84 = OpAll %bool %83
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %68 %84 %80
OpStore %ok %85
%86 = OpVectorShuffle %v2float %79 %79 2 3
%87 = OpVectorTimesScalar %v2float %86 %float_2
%88 = OpLoad %v4float %x
%89 = OpVectorShuffle %v4float %88 %87 4 5 2 3
OpStore %x %89
OpSelectionMerge %91 None
OpBranchConditional %85 %90 %91
%90 = OpLabel
%95 = OpFOrdEqual %v4bool %89 %94
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %81 %96 %90
OpStore %ok %97
%99 = OpVectorTimesScalar %v4float %36 %float_5
%100 = OpVectorShuffle %v4float %99 %99 1 0 3 2
OpStore %x %100
OpSelectionMerge %102 None
OpBranchConditional %97 %101 %102
%101 = OpLabel
%104 = OpFOrdEqual %v4bool %100 %103
%105 = OpAll %bool %104
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %91 %105 %101
OpStore %ok %106
%107 = OpFAdd %v4float %43 %36
OpStore %x %107
OpSelectionMerge %109 None
OpBranchConditional %106 %108 %109
%108 = OpLabel
%110 = OpFOrdEqual %v4bool %107 %49
%111 = OpAll %bool %110
OpBranch %109
%109 = OpLabel
%112 = OpPhi %bool %false %102 %111 %108
OpStore %ok %112
%114 = OpVectorShuffle %v4float %40 %40 1 3 0 2
%116 = OpFSub %v4float %115 %114
OpStore %x %116
OpSelectionMerge %118 None
OpBranchConditional %112 %117 %118
%117 = OpLabel
%120 = OpFOrdEqual %v4bool %116 %119
%121 = OpAll %bool %120
OpBranch %118
%118 = OpLabel
%122 = OpPhi %bool %false %109 %121 %117
OpStore %ok %122
%123 = OpCompositeExtract %float %36 0
%124 = OpCompositeConstruct %v4float %123 %123 %123 %123
%125 = OpFAdd %v4float %124 %40
OpStore %x %125
OpSelectionMerge %127 None
OpBranchConditional %122 %126 %127
%126 = OpLabel
%129 = OpFOrdEqual %v4bool %125 %128
%130 = OpAll %bool %129
OpBranch %127
%127 = OpLabel
%131 = OpPhi %bool %false %118 %130 %126
OpStore %ok %131
%133 = OpVectorShuffle %v3float %40 %40 3 1 3
%134 = OpVectorTimesScalar %v3float %133 %float_8
%135 = OpLoad %v4float %x
%136 = OpVectorShuffle %v4float %135 %134 4 5 6 3
OpStore %x %136
OpSelectionMerge %138 None
OpBranchConditional %131 %137 %138
%137 = OpLabel
%140 = OpFOrdEqual %v4bool %136 %139
%141 = OpAll %bool %140
OpBranch %138
%138 = OpLabel
%142 = OpPhi %bool %false %127 %141 %137
OpStore %ok %142
%144 = OpVectorShuffle %v2float %136 %136 2 3
%146 = OpFDiv %v2float %145 %144
%147 = OpLoad %v4float %x
%148 = OpVectorShuffle %v4float %147 %146 4 5 2 3
OpStore %x %148
OpSelectionMerge %150 None
OpBranchConditional %142 %149 %150
%149 = OpLabel
%153 = OpFOrdEqual %v4bool %148 %152
%154 = OpAll %bool %153
OpBranch %150
%150 = OpLabel
%155 = OpPhi %bool %false %138 %154 %149
OpStore %ok %155
%157 = OpFDiv %v4float %156 %148
%158 = OpVectorShuffle %v4float %157 %157 1 0 3 2
OpStore %x %158
OpSelectionMerge %160 None
OpBranchConditional %155 %159 %160
%159 = OpLabel
%162 = OpFOrdEqual %v4bool %158 %161
%163 = OpAll %bool %162
OpBranch %160
%160 = OpLabel
%164 = OpPhi %bool %false %150 %163 %159
OpStore %ok %164
%165 = OpFAdd %v4float %158 %43
OpStore %x %165
%166 = OpVectorTimesScalar %v4float %165 %float_2
OpStore %x %166
%168 = OpFSub %v4float %166 %167
OpStore %x %168
%169 = OpFDiv %float %float_1 %float_2
%170 = OpVectorTimesScalar %v4float %168 %169
OpStore %x %170
OpSelectionMerge %172 None
OpBranchConditional %164 %171 %172
%171 = OpLabel
%173 = OpFOrdEqual %v4bool %170 %161
%174 = OpAll %bool %173
OpBranch %172
%172 = OpLabel
%175 = OpPhi %bool %false %160 %174 %171
OpStore %ok %175
%176 = OpFAdd %v4float %170 %43
OpStore %x %176
%177 = OpVectorTimesScalar %v4float %176 %float_2
OpStore %x %177
%178 = OpFSub %v4float %177 %167
OpStore %x %178
%180 = OpVectorTimesScalar %v4float %178 %float_0_5
OpStore %x %180
OpSelectionMerge %182 None
OpBranchConditional %175 %181 %182
%181 = OpLabel
%183 = OpFOrdEqual %v4bool %180 %161
%184 = OpAll %bool %183
OpBranch %182
%182 = OpLabel
%185 = OpPhi %bool %false %172 %184 %181
OpStore %ok %185
OpReturnValue %185
OpFunctionEnd
%test_int_b = OpFunction %bool None %25
%186 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
%inputRed_0 = OpVariable %_ptr_Function_v4int Function
%inputGreen_0 = OpVariable %_ptr_Function_v4int Function
%x_0 = OpVariable %_ptr_Function_v4int Function
OpStore %ok_0 %true
%191 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%192 = OpLoad %v4float %191
%193 = OpCompositeExtract %float %192 0
%194 = OpConvertFToS %int %193
%195 = OpCompositeExtract %float %192 1
%196 = OpConvertFToS %int %195
%197 = OpCompositeExtract %float %192 2
%198 = OpConvertFToS %int %197
%199 = OpCompositeExtract %float %192 3
%200 = OpConvertFToS %int %199
%201 = OpCompositeConstruct %v4int %194 %196 %198 %200
OpStore %inputRed_0 %201
%203 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%204 = OpLoad %v4float %203
%205 = OpCompositeExtract %float %204 0
%206 = OpConvertFToS %int %205
%207 = OpCompositeExtract %float %204 1
%208 = OpConvertFToS %int %207
%209 = OpCompositeExtract %float %204 2
%210 = OpConvertFToS %int %209
%211 = OpCompositeExtract %float %204 3
%212 = OpConvertFToS %int %211
%213 = OpCompositeConstruct %v4int %206 %208 %210 %212
OpStore %inputGreen_0 %213
%217 = OpIAdd %v4int %201 %216
OpStore %x_0 %217
OpSelectionMerge %219 None
OpBranchConditional %true %218 %219
%218 = OpLabel
%222 = OpIEqual %v4bool %217 %221
%223 = OpAll %bool %222
OpBranch %219
%219 = OpLabel
%224 = OpPhi %bool %false %186 %223 %218
OpStore %ok_0 %224
%225 = OpVectorShuffle %v4int %213 %213 1 3 0 2
%226 = OpISub %v4int %225 %216
OpStore %x_0 %226
OpSelectionMerge %228 None
OpBranchConditional %224 %227 %228
%227 = OpLabel
%232 = OpIEqual %v4bool %226 %231
%233 = OpAll %bool %232
OpBranch %228
%228 = OpLabel
%234 = OpPhi %bool %false %219 %233 %227
OpStore %ok_0 %234
%235 = OpCompositeExtract %int %213 1
%236 = OpCompositeConstruct %v4int %235 %235 %235 %235
%237 = OpIAdd %v4int %201 %236
OpStore %x_0 %237
OpSelectionMerge %239 None
OpBranchConditional %234 %238 %239
%238 = OpLabel
%241 = OpIEqual %v4bool %237 %240
%242 = OpAll %bool %241
OpBranch %239
%239 = OpLabel
%243 = OpPhi %bool %false %228 %242 %238
OpStore %ok_0 %243
%244 = OpVectorShuffle %v3int %213 %213 3 1 3
%248 = OpIMul %v3int %244 %247
%249 = OpLoad %v4int %x_0
%250 = OpVectorShuffle %v4int %249 %248 4 5 6 3
OpStore %x_0 %250
OpSelectionMerge %252 None
OpBranchConditional %243 %251 %252
%251 = OpLabel
%254 = OpIEqual %v4bool %250 %253
%255 = OpAll %bool %254
OpBranch %252
%252 = OpLabel
%256 = OpPhi %bool %false %239 %255 %251
OpStore %ok_0 %256
%257 = OpVectorShuffle %v2int %250 %250 2 3
%261 = OpSDiv %v2int %257 %260
%262 = OpLoad %v4int %x_0
%263 = OpVectorShuffle %v4int %262 %261 4 5 2 3
OpStore %x_0 %263
OpSelectionMerge %265 None
OpBranchConditional %256 %264 %265
%264 = OpLabel
%267 = OpIEqual %v4bool %263 %266
%268 = OpAll %bool %267
OpBranch %265
%265 = OpLabel
%269 = OpPhi %bool %false %252 %268 %264
OpStore %ok_0 %269
%272 = OpIMul %v4int %201 %271
%273 = OpVectorShuffle %v4int %272 %272 1 0 3 2
OpStore %x_0 %273
OpSelectionMerge %275 None
OpBranchConditional %269 %274 %275
%274 = OpLabel
%277 = OpIEqual %v4bool %273 %276
%278 = OpAll %bool %277
OpBranch %275
%275 = OpLabel
%279 = OpPhi %bool %false %265 %278 %274
OpStore %ok_0 %279
%280 = OpIAdd %v4int %216 %201
OpStore %x_0 %280
OpSelectionMerge %282 None
OpBranchConditional %279 %281 %282
%281 = OpLabel
%283 = OpIEqual %v4bool %280 %221
%284 = OpAll %bool %283
OpBranch %282
%282 = OpLabel
%285 = OpPhi %bool %false %275 %284 %281
OpStore %ok_0 %285
%287 = OpVectorShuffle %v4int %213 %213 1 3 0 2
%289 = OpISub %v4int %288 %287
OpStore %x_0 %289
OpSelectionMerge %291 None
OpBranchConditional %285 %290 %291
%290 = OpLabel
%293 = OpIEqual %v4bool %289 %292
%294 = OpAll %bool %293
OpBranch %291
%291 = OpLabel
%295 = OpPhi %bool %false %282 %294 %290
OpStore %ok_0 %295
%296 = OpCompositeExtract %int %201 0
%297 = OpCompositeConstruct %v4int %296 %296 %296 %296
%298 = OpIAdd %v4int %297 %213
OpStore %x_0 %298
OpSelectionMerge %300 None
OpBranchConditional %295 %299 %300
%299 = OpLabel
%302 = OpIEqual %v4bool %298 %301
%303 = OpAll %bool %302
OpBranch %300
%300 = OpLabel
%304 = OpPhi %bool %false %291 %303 %299
OpStore %ok_0 %304
%306 = OpVectorShuffle %v3int %213 %213 3 1 3
%308 = OpIMul %v3int %307 %306
%309 = OpLoad %v4int %x_0
%310 = OpVectorShuffle %v4int %309 %308 4 5 6 3
OpStore %x_0 %310
OpSelectionMerge %312 None
OpBranchConditional %304 %311 %312
%311 = OpLabel
%314 = OpIEqual %v4bool %310 %313
%315 = OpAll %bool %314
OpBranch %312
%312 = OpLabel
%316 = OpPhi %bool %false %300 %315 %311
OpStore %ok_0 %316
%318 = OpVectorShuffle %v2int %310 %310 2 3
%320 = OpSDiv %v2int %319 %318
%321 = OpLoad %v4int %x_0
%322 = OpVectorShuffle %v4int %321 %320 4 5 2 3
OpStore %x_0 %322
OpSelectionMerge %324 None
OpBranchConditional %316 %323 %324
%323 = OpLabel
%327 = OpIEqual %v4bool %322 %326
%328 = OpAll %bool %327
OpBranch %324
%324 = OpLabel
%329 = OpPhi %bool %false %312 %328 %323
OpStore %ok_0 %329
%332 = OpSDiv %v4int %331 %322
%333 = OpVectorShuffle %v4int %332 %332 1 0 3 2
OpStore %x_0 %333
OpSelectionMerge %335 None
OpBranchConditional %329 %334 %335
%334 = OpLabel
%337 = OpIEqual %v4bool %333 %336
%338 = OpAll %bool %337
OpBranch %335
%335 = OpLabel
%339 = OpPhi %bool %false %324 %338 %334
OpStore %ok_0 %339
%340 = OpIAdd %v4int %333 %216
OpStore %x_0 %340
%341 = OpIMul %v4int %340 %216
OpStore %x_0 %341
%343 = OpISub %v4int %341 %342
OpStore %x_0 %343
%344 = OpSDiv %v4int %343 %216
OpStore %x_0 %344
OpSelectionMerge %346 None
OpBranchConditional %339 %345 %346
%345 = OpLabel
%347 = OpIEqual %v4bool %344 %336
%348 = OpAll %bool %347
OpBranch %346
%346 = OpLabel
%349 = OpPhi %bool %false %335 %348 %345
OpStore %ok_0 %349
%350 = OpIAdd %v4int %344 %216
OpStore %x_0 %350
%351 = OpIMul %v4int %350 %216
OpStore %x_0 %351
%352 = OpISub %v4int %351 %342
OpStore %x_0 %352
%353 = OpSDiv %v4int %352 %216
OpStore %x_0 %353
OpSelectionMerge %355 None
OpBranchConditional %349 %354 %355
%354 = OpLabel
%356 = OpIEqual %v4bool %353 %336
%357 = OpAll %bool %356
OpBranch %355
%355 = OpLabel
%358 = OpPhi %bool %false %346 %357 %354
OpStore %ok_0 %358
OpReturnValue %358
OpFunctionEnd
%main = OpFunction %v4float None %359
%360 = OpFunctionParameter %_ptr_Function_v2float
%361 = OpLabel
%367 = OpVariable %_ptr_Function_v4float Function
%362 = OpFunctionCall %bool %test_half_b
OpSelectionMerge %364 None
OpBranchConditional %362 %363 %364
%363 = OpLabel
%365 = OpFunctionCall %bool %test_int_b
OpBranch %364
%364 = OpLabel
%366 = OpPhi %bool %false %361 %365 %363
OpSelectionMerge %370 None
OpBranchConditional %366 %368 %369
%368 = OpLabel
%371 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%372 = OpLoad %v4float %371
OpStore %367 %372
OpBranch %370
%369 = OpLabel
%373 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%374 = OpLoad %v4float %373
OpStore %367 %374
OpBranch %370
%370 = OpLabel
%375 = OpLoad %v4float %367
OpReturnValue %375
OpFunctionEnd
