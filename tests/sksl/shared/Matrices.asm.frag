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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %607 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%float_7 = OpConstant %float 7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%322 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v2float %float_1 %float_2
%38 = OpCompositeConstruct %v2float %float_3 %float_4
%36 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
%44 = OpCompositeConstruct %mat2v2float %45 %46
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %44 0
%50 = OpFOrdEqual %v2bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %43 1
%53 = OpCompositeExtract %v2float %44 1
%54 = OpFOrdEqual %v2bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
OpBranch %42
%42 = OpLabel
%57 = OpPhi %bool %false %25 %56 %41
OpStore %ok %57
%59 = OpLoad %mat2v2float %m1
OpStore %m3 %59
%60 = OpLoad %bool %ok
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %mat2v2float %m3
%65 = OpCompositeConstruct %v2float %float_1 %float_2
%66 = OpCompositeConstruct %v2float %float_3 %float_4
%64 = OpCompositeConstruct %mat2v2float %65 %66
%67 = OpCompositeExtract %v2float %63 0
%68 = OpCompositeExtract %v2float %64 0
%69 = OpFOrdEqual %v2bool %67 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v2float %63 1
%72 = OpCompositeExtract %v2float %64 1
%73 = OpFOrdEqual %v2bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
OpBranch %62
%62 = OpLabel
%76 = OpPhi %bool %false %42 %75 %61
OpStore %ok %76
%80 = OpCompositeConstruct %v2float %float_6 %float_0
%81 = OpCompositeConstruct %v2float %float_0 %float_6
%79 = OpCompositeConstruct %mat2v2float %80 %81
OpStore %m4 %79
%82 = OpLoad %bool %ok
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %mat2v2float %m4
%87 = OpCompositeConstruct %v2float %float_6 %float_0
%88 = OpCompositeConstruct %v2float %float_0 %float_6
%86 = OpCompositeConstruct %mat2v2float %87 %88
%89 = OpCompositeExtract %v2float %85 0
%90 = OpCompositeExtract %v2float %86 0
%91 = OpFOrdEqual %v2bool %89 %90
%92 = OpAll %bool %91
%93 = OpCompositeExtract %v2float %85 1
%94 = OpCompositeExtract %v2float %86 1
%95 = OpFOrdEqual %v2bool %93 %94
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %92 %96
OpBranch %84
%84 = OpLabel
%98 = OpPhi %bool %false %62 %97 %83
OpStore %ok %98
%99 = OpLoad %mat2v2float %m3
%100 = OpLoad %mat2v2float %m4
%101 = OpMatrixTimesMatrix %mat2v2float %99 %100
OpStore %m3 %101
%102 = OpLoad %bool %ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %mat2v2float %m3
%110 = OpCompositeConstruct %v2float %float_6 %float_12
%111 = OpCompositeConstruct %v2float %float_18 %float_24
%109 = OpCompositeConstruct %mat2v2float %110 %111
%112 = OpCompositeExtract %v2float %105 0
%113 = OpCompositeExtract %v2float %109 0
%114 = OpFOrdEqual %v2bool %112 %113
%115 = OpAll %bool %114
%116 = OpCompositeExtract %v2float %105 1
%117 = OpCompositeExtract %v2float %109 1
%118 = OpFOrdEqual %v2bool %116 %117
%119 = OpAll %bool %118
%120 = OpLogicalAnd %bool %115 %119
OpBranch %104
%104 = OpLabel
%121 = OpPhi %bool %false %84 %120 %103
OpStore %ok %121
%125 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%126 = OpLoad %v2float %125
%127 = OpCompositeExtract %float %126 1
%129 = OpCompositeConstruct %v2float %127 %float_0
%130 = OpCompositeConstruct %v2float %float_0 %127
%128 = OpCompositeConstruct %mat2v2float %129 %130
OpStore %m5 %128
%131 = OpLoad %bool %ok
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpLoad %mat2v2float %m5
%136 = OpCompositeConstruct %v2float %float_4 %float_0
%137 = OpCompositeConstruct %v2float %float_0 %float_4
%135 = OpCompositeConstruct %mat2v2float %136 %137
%138 = OpCompositeExtract %v2float %134 0
%139 = OpCompositeExtract %v2float %135 0
%140 = OpFOrdEqual %v2bool %138 %139
%141 = OpAll %bool %140
%142 = OpCompositeExtract %v2float %134 1
%143 = OpCompositeExtract %v2float %135 1
%144 = OpFOrdEqual %v2bool %142 %143
%145 = OpAll %bool %144
%146 = OpLogicalAnd %bool %141 %145
OpBranch %133
%133 = OpLabel
%147 = OpPhi %bool %false %104 %146 %132
OpStore %ok %147
%148 = OpLoad %mat2v2float %m1
%149 = OpLoad %mat2v2float %m5
%150 = OpCompositeExtract %v2float %148 0
%151 = OpCompositeExtract %v2float %149 0
%152 = OpFAdd %v2float %150 %151
%153 = OpCompositeExtract %v2float %148 1
%154 = OpCompositeExtract %v2float %149 1
%155 = OpFAdd %v2float %153 %154
%156 = OpCompositeConstruct %mat2v2float %152 %155
OpStore %m1 %156
%157 = OpLoad %bool %ok
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpLoad %mat2v2float %m1
%164 = OpCompositeConstruct %v2float %float_5 %float_2
%165 = OpCompositeConstruct %v2float %float_3 %float_8
%163 = OpCompositeConstruct %mat2v2float %164 %165
%166 = OpCompositeExtract %v2float %160 0
%167 = OpCompositeExtract %v2float %163 0
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
%170 = OpCompositeExtract %v2float %160 1
%171 = OpCompositeExtract %v2float %163 1
%172 = OpFOrdEqual %v2bool %170 %171
%173 = OpAll %bool %172
%174 = OpLogicalAnd %bool %169 %173
OpBranch %159
%159 = OpLabel
%175 = OpPhi %bool %false %133 %174 %158
OpStore %ok %175
%179 = OpCompositeConstruct %v2float %float_5 %float_6
%180 = OpCompositeConstruct %v2float %float_7 %float_8
%178 = OpCompositeConstruct %mat2v2float %179 %180
OpStore %m7 %178
%181 = OpLoad %bool %ok
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpLoad %mat2v2float %m7
%186 = OpCompositeConstruct %v2float %float_5 %float_6
%187 = OpCompositeConstruct %v2float %float_7 %float_8
%185 = OpCompositeConstruct %mat2v2float %186 %187
%188 = OpCompositeExtract %v2float %184 0
%189 = OpCompositeExtract %v2float %185 0
%190 = OpFOrdEqual %v2bool %188 %189
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v2float %184 1
%193 = OpCompositeExtract %v2float %185 1
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %191 %195
OpBranch %183
%183 = OpLabel
%197 = OpPhi %bool %false %159 %196 %182
OpStore %ok %197
%204 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%205 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%206 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%203 = OpCompositeConstruct %mat3v3float %204 %205 %206
OpStore %m9 %203
%207 = OpLoad %bool %ok
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%210 = OpLoad %mat3v3float %m9
%212 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%211 = OpCompositeConstruct %mat3v3float %212 %213 %214
%216 = OpCompositeExtract %v3float %210 0
%217 = OpCompositeExtract %v3float %211 0
%218 = OpFOrdEqual %v3bool %216 %217
%219 = OpAll %bool %218
%220 = OpCompositeExtract %v3float %210 1
%221 = OpCompositeExtract %v3float %211 1
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpLogicalAnd %bool %219 %223
%225 = OpCompositeExtract %v3float %210 2
%226 = OpCompositeExtract %v3float %211 2
%227 = OpFOrdEqual %v3bool %225 %226
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %224 %228
OpBranch %209
%209 = OpLabel
%230 = OpPhi %bool %false %183 %229 %208
OpStore %ok %230
%236 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%237 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%238 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%239 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%235 = OpCompositeConstruct %mat4v4float %236 %237 %238 %239
OpStore %m10 %235
%240 = OpLoad %bool %ok
OpSelectionMerge %242 None
OpBranchConditional %240 %241 %242
%241 = OpLabel
%243 = OpLoad %mat4v4float %m10
%245 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%246 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%247 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%248 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%244 = OpCompositeConstruct %mat4v4float %245 %246 %247 %248
%250 = OpCompositeExtract %v4float %243 0
%251 = OpCompositeExtract %v4float %244 0
%252 = OpFOrdEqual %v4bool %250 %251
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v4float %243 1
%255 = OpCompositeExtract %v4float %244 1
%256 = OpFOrdEqual %v4bool %254 %255
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %253 %257
%259 = OpCompositeExtract %v4float %243 2
%260 = OpCompositeExtract %v4float %244 2
%261 = OpFOrdEqual %v4bool %259 %260
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %258 %262
%264 = OpCompositeExtract %v4float %243 3
%265 = OpCompositeExtract %v4float %244 3
%266 = OpFOrdEqual %v4bool %264 %265
%267 = OpAll %bool %266
%268 = OpLogicalAnd %bool %263 %267
OpBranch %242
%242 = OpLabel
%269 = OpPhi %bool %false %209 %268 %241
OpStore %ok %269
%273 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%274 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%275 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%276 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%272 = OpCompositeConstruct %mat4v4float %273 %274 %275 %276
OpStore %m11 %272
%277 = OpLoad %mat4v4float %m11
%278 = OpLoad %mat4v4float %m10
%279 = OpCompositeExtract %v4float %277 0
%280 = OpCompositeExtract %v4float %278 0
%281 = OpFSub %v4float %279 %280
%282 = OpCompositeExtract %v4float %277 1
%283 = OpCompositeExtract %v4float %278 1
%284 = OpFSub %v4float %282 %283
%285 = OpCompositeExtract %v4float %277 2
%286 = OpCompositeExtract %v4float %278 2
%287 = OpFSub %v4float %285 %286
%288 = OpCompositeExtract %v4float %277 3
%289 = OpCompositeExtract %v4float %278 3
%290 = OpFSub %v4float %288 %289
%291 = OpCompositeConstruct %mat4v4float %281 %284 %287 %290
OpStore %m11 %291
%292 = OpLoad %bool %ok
OpSelectionMerge %294 None
OpBranchConditional %292 %293 %294
%293 = OpLabel
%295 = OpLoad %mat4v4float %m11
%297 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%298 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%299 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%300 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%296 = OpCompositeConstruct %mat4v4float %297 %298 %299 %300
%301 = OpCompositeExtract %v4float %295 0
%302 = OpCompositeExtract %v4float %296 0
%303 = OpFOrdEqual %v4bool %301 %302
%304 = OpAll %bool %303
%305 = OpCompositeExtract %v4float %295 1
%306 = OpCompositeExtract %v4float %296 1
%307 = OpFOrdEqual %v4bool %305 %306
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %304 %308
%310 = OpCompositeExtract %v4float %295 2
%311 = OpCompositeExtract %v4float %296 2
%312 = OpFOrdEqual %v4bool %310 %311
%313 = OpAll %bool %312
%314 = OpLogicalAnd %bool %309 %313
%315 = OpCompositeExtract %v4float %295 3
%316 = OpCompositeExtract %v4float %296 3
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %314 %318
OpBranch %294
%294 = OpLabel
%320 = OpPhi %bool %false %242 %319 %293
OpStore %ok %320
%321 = OpLoad %bool %ok
OpReturnValue %321
OpFunctionEnd
%main = OpFunction %v4float None %322
%323 = OpFunctionParameter %_ptr_Function_v2float
%324 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%596 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%328 = OpCompositeConstruct %v2float %float_1 %float_2
%329 = OpCompositeConstruct %v2float %float_3 %float_4
%327 = OpCompositeConstruct %mat2v2float %328 %329
OpStore %_1_m1 %327
%330 = OpLoad %bool %_0_ok
OpSelectionMerge %332 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%333 = OpLoad %mat2v2float %_1_m1
%335 = OpCompositeConstruct %v2float %float_1 %float_2
%336 = OpCompositeConstruct %v2float %float_3 %float_4
%334 = OpCompositeConstruct %mat2v2float %335 %336
%337 = OpCompositeExtract %v2float %333 0
%338 = OpCompositeExtract %v2float %334 0
%339 = OpFOrdEqual %v2bool %337 %338
%340 = OpAll %bool %339
%341 = OpCompositeExtract %v2float %333 1
%342 = OpCompositeExtract %v2float %334 1
%343 = OpFOrdEqual %v2bool %341 %342
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %340 %344
OpBranch %332
%332 = OpLabel
%346 = OpPhi %bool %false %324 %345 %331
OpStore %_0_ok %346
%348 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %348
%349 = OpLoad %bool %_0_ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpLoad %mat2v2float %_2_m3
%354 = OpCompositeConstruct %v2float %float_1 %float_2
%355 = OpCompositeConstruct %v2float %float_3 %float_4
%353 = OpCompositeConstruct %mat2v2float %354 %355
%356 = OpCompositeExtract %v2float %352 0
%357 = OpCompositeExtract %v2float %353 0
%358 = OpFOrdEqual %v2bool %356 %357
%359 = OpAll %bool %358
%360 = OpCompositeExtract %v2float %352 1
%361 = OpCompositeExtract %v2float %353 1
%362 = OpFOrdEqual %v2bool %360 %361
%363 = OpAll %bool %362
%364 = OpLogicalAnd %bool %359 %363
OpBranch %351
%351 = OpLabel
%365 = OpPhi %bool %false %332 %364 %350
OpStore %_0_ok %365
%368 = OpCompositeConstruct %v2float %float_6 %float_0
%369 = OpCompositeConstruct %v2float %float_0 %float_6
%367 = OpCompositeConstruct %mat2v2float %368 %369
OpStore %_3_m4 %367
%370 = OpLoad %bool %_0_ok
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %mat2v2float %_3_m4
%375 = OpCompositeConstruct %v2float %float_6 %float_0
%376 = OpCompositeConstruct %v2float %float_0 %float_6
%374 = OpCompositeConstruct %mat2v2float %375 %376
%377 = OpCompositeExtract %v2float %373 0
%378 = OpCompositeExtract %v2float %374 0
%379 = OpFOrdEqual %v2bool %377 %378
%380 = OpAll %bool %379
%381 = OpCompositeExtract %v2float %373 1
%382 = OpCompositeExtract %v2float %374 1
%383 = OpFOrdEqual %v2bool %381 %382
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %380 %384
OpBranch %372
%372 = OpLabel
%386 = OpPhi %bool %false %351 %385 %371
OpStore %_0_ok %386
%387 = OpLoad %mat2v2float %_2_m3
%388 = OpLoad %mat2v2float %_3_m4
%389 = OpMatrixTimesMatrix %mat2v2float %387 %388
OpStore %_2_m3 %389
%390 = OpLoad %bool %_0_ok
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpLoad %mat2v2float %_2_m3
%395 = OpCompositeConstruct %v2float %float_6 %float_12
%396 = OpCompositeConstruct %v2float %float_18 %float_24
%394 = OpCompositeConstruct %mat2v2float %395 %396
%397 = OpCompositeExtract %v2float %393 0
%398 = OpCompositeExtract %v2float %394 0
%399 = OpFOrdEqual %v2bool %397 %398
%400 = OpAll %bool %399
%401 = OpCompositeExtract %v2float %393 1
%402 = OpCompositeExtract %v2float %394 1
%403 = OpFOrdEqual %v2bool %401 %402
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %400 %404
OpBranch %392
%392 = OpLabel
%406 = OpPhi %bool %false %372 %405 %391
OpStore %_0_ok %406
%408 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%409 = OpLoad %v2float %408
%410 = OpCompositeExtract %float %409 1
%412 = OpCompositeConstruct %v2float %410 %float_0
%413 = OpCompositeConstruct %v2float %float_0 %410
%411 = OpCompositeConstruct %mat2v2float %412 %413
OpStore %_4_m5 %411
%414 = OpLoad %bool %_0_ok
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%417 = OpLoad %mat2v2float %_4_m5
%419 = OpCompositeConstruct %v2float %float_4 %float_0
%420 = OpCompositeConstruct %v2float %float_0 %float_4
%418 = OpCompositeConstruct %mat2v2float %419 %420
%421 = OpCompositeExtract %v2float %417 0
%422 = OpCompositeExtract %v2float %418 0
%423 = OpFOrdEqual %v2bool %421 %422
%424 = OpAll %bool %423
%425 = OpCompositeExtract %v2float %417 1
%426 = OpCompositeExtract %v2float %418 1
%427 = OpFOrdEqual %v2bool %425 %426
%428 = OpAll %bool %427
%429 = OpLogicalAnd %bool %424 %428
OpBranch %416
%416 = OpLabel
%430 = OpPhi %bool %false %392 %429 %415
OpStore %_0_ok %430
%431 = OpLoad %mat2v2float %_1_m1
%432 = OpLoad %mat2v2float %_4_m5
%433 = OpCompositeExtract %v2float %431 0
%434 = OpCompositeExtract %v2float %432 0
%435 = OpFAdd %v2float %433 %434
%436 = OpCompositeExtract %v2float %431 1
%437 = OpCompositeExtract %v2float %432 1
%438 = OpFAdd %v2float %436 %437
%439 = OpCompositeConstruct %mat2v2float %435 %438
OpStore %_1_m1 %439
%440 = OpLoad %bool %_0_ok
OpSelectionMerge %442 None
OpBranchConditional %440 %441 %442
%441 = OpLabel
%443 = OpLoad %mat2v2float %_1_m1
%445 = OpCompositeConstruct %v2float %float_5 %float_2
%446 = OpCompositeConstruct %v2float %float_3 %float_8
%444 = OpCompositeConstruct %mat2v2float %445 %446
%447 = OpCompositeExtract %v2float %443 0
%448 = OpCompositeExtract %v2float %444 0
%449 = OpFOrdEqual %v2bool %447 %448
%450 = OpAll %bool %449
%451 = OpCompositeExtract %v2float %443 1
%452 = OpCompositeExtract %v2float %444 1
%453 = OpFOrdEqual %v2bool %451 %452
%454 = OpAll %bool %453
%455 = OpLogicalAnd %bool %450 %454
OpBranch %442
%442 = OpLabel
%456 = OpPhi %bool %false %416 %455 %441
OpStore %_0_ok %456
%459 = OpCompositeConstruct %v2float %float_5 %float_6
%460 = OpCompositeConstruct %v2float %float_7 %float_8
%458 = OpCompositeConstruct %mat2v2float %459 %460
OpStore %_5_m7 %458
%461 = OpLoad %bool %_0_ok
OpSelectionMerge %463 None
OpBranchConditional %461 %462 %463
%462 = OpLabel
%464 = OpLoad %mat2v2float %_5_m7
%466 = OpCompositeConstruct %v2float %float_5 %float_6
%467 = OpCompositeConstruct %v2float %float_7 %float_8
%465 = OpCompositeConstruct %mat2v2float %466 %467
%468 = OpCompositeExtract %v2float %464 0
%469 = OpCompositeExtract %v2float %465 0
%470 = OpFOrdEqual %v2bool %468 %469
%471 = OpAll %bool %470
%472 = OpCompositeExtract %v2float %464 1
%473 = OpCompositeExtract %v2float %465 1
%474 = OpFOrdEqual %v2bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
OpBranch %463
%463 = OpLabel
%477 = OpPhi %bool %false %442 %476 %462
OpStore %_0_ok %477
%480 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%481 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%482 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%479 = OpCompositeConstruct %mat3v3float %480 %481 %482
OpStore %_6_m9 %479
%483 = OpLoad %bool %_0_ok
OpSelectionMerge %485 None
OpBranchConditional %483 %484 %485
%484 = OpLabel
%486 = OpLoad %mat3v3float %_6_m9
%488 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%489 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%490 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%487 = OpCompositeConstruct %mat3v3float %488 %489 %490
%491 = OpCompositeExtract %v3float %486 0
%492 = OpCompositeExtract %v3float %487 0
%493 = OpFOrdEqual %v3bool %491 %492
%494 = OpAll %bool %493
%495 = OpCompositeExtract %v3float %486 1
%496 = OpCompositeExtract %v3float %487 1
%497 = OpFOrdEqual %v3bool %495 %496
%498 = OpAll %bool %497
%499 = OpLogicalAnd %bool %494 %498
%500 = OpCompositeExtract %v3float %486 2
%501 = OpCompositeExtract %v3float %487 2
%502 = OpFOrdEqual %v3bool %500 %501
%503 = OpAll %bool %502
%504 = OpLogicalAnd %bool %499 %503
OpBranch %485
%485 = OpLabel
%505 = OpPhi %bool %false %463 %504 %484
OpStore %_0_ok %505
%508 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%509 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%510 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%511 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%507 = OpCompositeConstruct %mat4v4float %508 %509 %510 %511
OpStore %_7_m10 %507
%512 = OpLoad %bool %_0_ok
OpSelectionMerge %514 None
OpBranchConditional %512 %513 %514
%513 = OpLabel
%515 = OpLoad %mat4v4float %_7_m10
%517 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%518 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%519 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%520 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%516 = OpCompositeConstruct %mat4v4float %517 %518 %519 %520
%521 = OpCompositeExtract %v4float %515 0
%522 = OpCompositeExtract %v4float %516 0
%523 = OpFOrdEqual %v4bool %521 %522
%524 = OpAll %bool %523
%525 = OpCompositeExtract %v4float %515 1
%526 = OpCompositeExtract %v4float %516 1
%527 = OpFOrdEqual %v4bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
%530 = OpCompositeExtract %v4float %515 2
%531 = OpCompositeExtract %v4float %516 2
%532 = OpFOrdEqual %v4bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
%535 = OpCompositeExtract %v4float %515 3
%536 = OpCompositeExtract %v4float %516 3
%537 = OpFOrdEqual %v4bool %535 %536
%538 = OpAll %bool %537
%539 = OpLogicalAnd %bool %534 %538
OpBranch %514
%514 = OpLabel
%540 = OpPhi %bool %false %485 %539 %513
OpStore %_0_ok %540
%543 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%544 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%545 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%546 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%542 = OpCompositeConstruct %mat4v4float %543 %544 %545 %546
OpStore %_8_m11 %542
%547 = OpLoad %mat4v4float %_8_m11
%548 = OpLoad %mat4v4float %_7_m10
%549 = OpCompositeExtract %v4float %547 0
%550 = OpCompositeExtract %v4float %548 0
%551 = OpFSub %v4float %549 %550
%552 = OpCompositeExtract %v4float %547 1
%553 = OpCompositeExtract %v4float %548 1
%554 = OpFSub %v4float %552 %553
%555 = OpCompositeExtract %v4float %547 2
%556 = OpCompositeExtract %v4float %548 2
%557 = OpFSub %v4float %555 %556
%558 = OpCompositeExtract %v4float %547 3
%559 = OpCompositeExtract %v4float %548 3
%560 = OpFSub %v4float %558 %559
%561 = OpCompositeConstruct %mat4v4float %551 %554 %557 %560
OpStore %_8_m11 %561
%562 = OpLoad %bool %_0_ok
OpSelectionMerge %564 None
OpBranchConditional %562 %563 %564
%563 = OpLabel
%565 = OpLoad %mat4v4float %_8_m11
%567 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%568 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%569 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%570 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%566 = OpCompositeConstruct %mat4v4float %567 %568 %569 %570
%571 = OpCompositeExtract %v4float %565 0
%572 = OpCompositeExtract %v4float %566 0
%573 = OpFOrdEqual %v4bool %571 %572
%574 = OpAll %bool %573
%575 = OpCompositeExtract %v4float %565 1
%576 = OpCompositeExtract %v4float %566 1
%577 = OpFOrdEqual %v4bool %575 %576
%578 = OpAll %bool %577
%579 = OpLogicalAnd %bool %574 %578
%580 = OpCompositeExtract %v4float %565 2
%581 = OpCompositeExtract %v4float %566 2
%582 = OpFOrdEqual %v4bool %580 %581
%583 = OpAll %bool %582
%584 = OpLogicalAnd %bool %579 %583
%585 = OpCompositeExtract %v4float %565 3
%586 = OpCompositeExtract %v4float %566 3
%587 = OpFOrdEqual %v4bool %585 %586
%588 = OpAll %bool %587
%589 = OpLogicalAnd %bool %584 %588
OpBranch %564
%564 = OpLabel
%590 = OpPhi %bool %false %514 %589 %563
OpStore %_0_ok %590
%591 = OpLoad %bool %_0_ok
OpSelectionMerge %593 None
OpBranchConditional %591 %592 %593
%592 = OpLabel
%594 = OpFunctionCall %bool %test_half_b
OpBranch %593
%593 = OpLabel
%595 = OpPhi %bool %false %564 %594 %592
OpSelectionMerge %600 None
OpBranchConditional %595 %598 %599
%598 = OpLabel
%601 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%604 = OpLoad %v4float %601
OpStore %596 %604
OpBranch %600
%599 = OpLabel
%605 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%606 = OpLoad %v4float %605
OpStore %596 %606
OpBranch %600
%600 = OpLabel
%607 = OpLoad %v4float %596
OpReturnValue %607
OpFunctionEnd
