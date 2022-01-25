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
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
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
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %629 RelaxedPrecision
OpDecorate %631 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
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
%343 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%39 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
%47 = OpCompositeConstruct %mat2v2float %45 %46
%49 = OpCompositeExtract %v2float %44 0
%50 = OpCompositeExtract %v2float %47 0
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpCompositeExtract %v2float %44 1
%54 = OpCompositeExtract %v2float %47 1
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %52 %56
OpBranch %43
%43 = OpLabel
%58 = OpPhi %bool %false %26 %57 %42
OpStore %ok %58
%60 = OpLoad %mat2v2float %m1
OpStore %m3 %60
%61 = OpLoad %bool %ok
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %mat2v2float %m3
%65 = OpCompositeConstruct %v2float %float_1 %float_2
%66 = OpCompositeConstruct %v2float %float_3 %float_4
%67 = OpCompositeConstruct %mat2v2float %65 %66
%68 = OpCompositeExtract %v2float %64 0
%69 = OpCompositeExtract %v2float %67 0
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpCompositeExtract %v2float %64 1
%73 = OpCompositeExtract %v2float %67 1
%74 = OpFOrdEqual %v2bool %72 %73
%75 = OpAll %bool %74
%76 = OpLogicalAnd %bool %71 %75
OpBranch %63
%63 = OpLabel
%77 = OpPhi %bool %false %43 %76 %62
OpStore %ok %77
%81 = OpCompositeConstruct %v2float %float_6 %float_0
%82 = OpCompositeConstruct %v2float %float_0 %float_6
%80 = OpCompositeConstruct %mat2v2float %81 %82
OpStore %m4 %80
%83 = OpLoad %bool %ok
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %mat2v2float %m4
%87 = OpCompositeConstruct %v2float %float_6 %float_0
%88 = OpCompositeConstruct %v2float %float_0 %float_6
%89 = OpCompositeConstruct %mat2v2float %87 %88
%90 = OpCompositeExtract %v2float %86 0
%91 = OpCompositeExtract %v2float %89 0
%92 = OpFOrdEqual %v2bool %90 %91
%93 = OpAll %bool %92
%94 = OpCompositeExtract %v2float %86 1
%95 = OpCompositeExtract %v2float %89 1
%96 = OpFOrdEqual %v2bool %94 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %93 %97
OpBranch %85
%85 = OpLabel
%99 = OpPhi %bool %false %63 %98 %84
OpStore %ok %99
%100 = OpLoad %mat2v2float %m3
%101 = OpLoad %mat2v2float %m4
%102 = OpMatrixTimesMatrix %mat2v2float %100 %101
OpStore %m3 %102
%103 = OpLoad %bool %ok
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %mat2v2float %m3
%110 = OpCompositeConstruct %v2float %float_6 %float_12
%111 = OpCompositeConstruct %v2float %float_18 %float_24
%112 = OpCompositeConstruct %mat2v2float %110 %111
%113 = OpCompositeExtract %v2float %106 0
%114 = OpCompositeExtract %v2float %112 0
%115 = OpFOrdEqual %v2bool %113 %114
%116 = OpAll %bool %115
%117 = OpCompositeExtract %v2float %106 1
%118 = OpCompositeExtract %v2float %112 1
%119 = OpFOrdEqual %v2bool %117 %118
%120 = OpAll %bool %119
%121 = OpLogicalAnd %bool %116 %120
OpBranch %105
%105 = OpLabel
%122 = OpPhi %bool %false %85 %121 %104
OpStore %ok %122
%126 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%127 = OpLoad %v2float %126
%128 = OpCompositeExtract %float %127 1
%130 = OpCompositeConstruct %v2float %128 %float_0
%131 = OpCompositeConstruct %v2float %float_0 %128
%129 = OpCompositeConstruct %mat2v2float %130 %131
OpStore %m5 %129
%132 = OpLoad %bool %ok
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %mat2v2float %m5
%136 = OpCompositeConstruct %v2float %float_4 %float_0
%137 = OpCompositeConstruct %v2float %float_0 %float_4
%138 = OpCompositeConstruct %mat2v2float %136 %137
%139 = OpCompositeExtract %v2float %135 0
%140 = OpCompositeExtract %v2float %138 0
%141 = OpFOrdEqual %v2bool %139 %140
%142 = OpAll %bool %141
%143 = OpCompositeExtract %v2float %135 1
%144 = OpCompositeExtract %v2float %138 1
%145 = OpFOrdEqual %v2bool %143 %144
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %142 %146
OpBranch %134
%134 = OpLabel
%148 = OpPhi %bool %false %105 %147 %133
OpStore %ok %148
%149 = OpLoad %mat2v2float %m1
%150 = OpLoad %mat2v2float %m5
%151 = OpCompositeExtract %v2float %149 0
%152 = OpCompositeExtract %v2float %150 0
%153 = OpFAdd %v2float %151 %152
%154 = OpCompositeExtract %v2float %149 1
%155 = OpCompositeExtract %v2float %150 1
%156 = OpFAdd %v2float %154 %155
%157 = OpCompositeConstruct %mat2v2float %153 %156
OpStore %m1 %157
%158 = OpLoad %bool %ok
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %mat2v2float %m1
%164 = OpCompositeConstruct %v2float %float_5 %float_2
%165 = OpCompositeConstruct %v2float %float_3 %float_8
%166 = OpCompositeConstruct %mat2v2float %164 %165
%167 = OpCompositeExtract %v2float %161 0
%168 = OpCompositeExtract %v2float %166 0
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpCompositeExtract %v2float %161 1
%172 = OpCompositeExtract %v2float %166 1
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
OpBranch %160
%160 = OpLabel
%176 = OpPhi %bool %false %134 %175 %159
OpStore %ok %176
%179 = OpCompositeConstruct %v2float %float_5 %float_6
%180 = OpCompositeConstruct %v2float %float_7 %float_8
%181 = OpCompositeConstruct %mat2v2float %179 %180
OpStore %m7 %181
%182 = OpLoad %bool %ok
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %mat2v2float %m7
%186 = OpCompositeConstruct %v2float %float_5 %float_6
%187 = OpCompositeConstruct %v2float %float_7 %float_8
%188 = OpCompositeConstruct %mat2v2float %186 %187
%189 = OpCompositeExtract %v2float %185 0
%190 = OpCompositeExtract %v2float %188 0
%191 = OpFOrdEqual %v2bool %189 %190
%192 = OpAll %bool %191
%193 = OpCompositeExtract %v2float %185 1
%194 = OpCompositeExtract %v2float %188 1
%195 = OpFOrdEqual %v2bool %193 %194
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %192 %196
OpBranch %184
%184 = OpLabel
%198 = OpPhi %bool %false %160 %197 %183
OpStore %ok %198
%205 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%206 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%207 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%204 = OpCompositeConstruct %mat3v3float %205 %206 %207
OpStore %m9 %204
%208 = OpLoad %bool %ok
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpLoad %mat3v3float %m9
%212 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%215 = OpCompositeConstruct %mat3v3float %212 %213 %214
%217 = OpCompositeExtract %v3float %211 0
%218 = OpCompositeExtract %v3float %215 0
%219 = OpFOrdEqual %v3bool %217 %218
%220 = OpAll %bool %219
%221 = OpCompositeExtract %v3float %211 1
%222 = OpCompositeExtract %v3float %215 1
%223 = OpFOrdEqual %v3bool %221 %222
%224 = OpAll %bool %223
%225 = OpLogicalAnd %bool %220 %224
%226 = OpCompositeExtract %v3float %211 2
%227 = OpCompositeExtract %v3float %215 2
%228 = OpFOrdEqual %v3bool %226 %227
%229 = OpAll %bool %228
%230 = OpLogicalAnd %bool %225 %229
OpBranch %210
%210 = OpLabel
%231 = OpPhi %bool %false %184 %230 %209
OpStore %ok %231
%237 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%238 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%239 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%240 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%236 = OpCompositeConstruct %mat4v4float %237 %238 %239 %240
OpStore %m10 %236
%241 = OpLoad %bool %ok
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
%244 = OpLoad %mat4v4float %m10
%245 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%246 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%247 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%248 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%249 = OpCompositeConstruct %mat4v4float %245 %246 %247 %248
%251 = OpCompositeExtract %v4float %244 0
%252 = OpCompositeExtract %v4float %249 0
%253 = OpFOrdEqual %v4bool %251 %252
%254 = OpAll %bool %253
%255 = OpCompositeExtract %v4float %244 1
%256 = OpCompositeExtract %v4float %249 1
%257 = OpFOrdEqual %v4bool %255 %256
%258 = OpAll %bool %257
%259 = OpLogicalAnd %bool %254 %258
%260 = OpCompositeExtract %v4float %244 2
%261 = OpCompositeExtract %v4float %249 2
%262 = OpFOrdEqual %v4bool %260 %261
%263 = OpAll %bool %262
%264 = OpLogicalAnd %bool %259 %263
%265 = OpCompositeExtract %v4float %244 3
%266 = OpCompositeExtract %v4float %249 3
%267 = OpFOrdEqual %v4bool %265 %266
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %264 %268
OpBranch %243
%243 = OpLabel
%270 = OpPhi %bool %false %210 %269 %242
OpStore %ok %270
%273 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%274 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%275 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%276 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%277 = OpCompositeConstruct %mat4v4float %273 %274 %275 %276
OpStore %m11 %277
%278 = OpLoad %mat4v4float %m11
%279 = OpLoad %mat4v4float %m10
%280 = OpCompositeExtract %v4float %278 0
%281 = OpCompositeExtract %v4float %279 0
%282 = OpFSub %v4float %280 %281
%283 = OpCompositeExtract %v4float %278 1
%284 = OpCompositeExtract %v4float %279 1
%285 = OpFSub %v4float %283 %284
%286 = OpCompositeExtract %v4float %278 2
%287 = OpCompositeExtract %v4float %279 2
%288 = OpFSub %v4float %286 %287
%289 = OpCompositeExtract %v4float %278 3
%290 = OpCompositeExtract %v4float %279 3
%291 = OpFSub %v4float %289 %290
%292 = OpCompositeConstruct %mat4v4float %282 %285 %288 %291
OpStore %m11 %292
%293 = OpLoad %bool %ok
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpLoad %mat4v4float %m11
%297 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%298 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%299 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%300 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%301 = OpCompositeConstruct %mat4v4float %297 %298 %299 %300
%302 = OpCompositeExtract %v4float %296 0
%303 = OpCompositeExtract %v4float %301 0
%304 = OpFOrdEqual %v4bool %302 %303
%305 = OpAll %bool %304
%306 = OpCompositeExtract %v4float %296 1
%307 = OpCompositeExtract %v4float %301 1
%308 = OpFOrdEqual %v4bool %306 %307
%309 = OpAll %bool %308
%310 = OpLogicalAnd %bool %305 %309
%311 = OpCompositeExtract %v4float %296 2
%312 = OpCompositeExtract %v4float %301 2
%313 = OpFOrdEqual %v4bool %311 %312
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %310 %314
%316 = OpCompositeExtract %v4float %296 3
%317 = OpCompositeExtract %v4float %301 3
%318 = OpFOrdEqual %v4bool %316 %317
%319 = OpAll %bool %318
%320 = OpLogicalAnd %bool %315 %319
OpBranch %295
%295 = OpLabel
%321 = OpPhi %bool %false %243 %320 %294
OpStore %ok %321
%322 = OpLoad %bool %ok
OpReturnValue %322
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%323 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%326 = OpCompositeConstruct %v2float %float_1 %float_2
%327 = OpCompositeConstruct %v2float %float_3 %float_4
%328 = OpCompositeConstruct %mat2v2float %326 %327
OpStore %x %328
%329 = OpCompositeConstruct %v2float %float_1 %float_2
%330 = OpCompositeConstruct %v2float %float_3 %float_4
%331 = OpCompositeConstruct %mat2v2float %329 %330
OpStore %y %331
%332 = OpLoad %mat2v2float %x
%333 = OpLoad %mat2v2float %y
%334 = OpCompositeExtract %v2float %332 0
%335 = OpCompositeExtract %v2float %333 0
%336 = OpFOrdEqual %v2bool %334 %335
%337 = OpAll %bool %336
%338 = OpCompositeExtract %v2float %332 1
%339 = OpCompositeExtract %v2float %333 1
%340 = OpFOrdEqual %v2bool %338 %339
%341 = OpAll %bool %340
%342 = OpLogicalAnd %bool %337 %341
OpReturnValue %342
OpFunctionEnd
%main = OpFunction %v4float None %343
%344 = OpFunctionParameter %_ptr_Function_v2float
%345 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%621 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%348 = OpCompositeConstruct %v2float %float_1 %float_2
%349 = OpCompositeConstruct %v2float %float_3 %float_4
%350 = OpCompositeConstruct %mat2v2float %348 %349
OpStore %_1_m1 %350
%351 = OpLoad %bool %_0_ok
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %mat2v2float %_1_m1
%355 = OpCompositeConstruct %v2float %float_1 %float_2
%356 = OpCompositeConstruct %v2float %float_3 %float_4
%357 = OpCompositeConstruct %mat2v2float %355 %356
%358 = OpCompositeExtract %v2float %354 0
%359 = OpCompositeExtract %v2float %357 0
%360 = OpFOrdEqual %v2bool %358 %359
%361 = OpAll %bool %360
%362 = OpCompositeExtract %v2float %354 1
%363 = OpCompositeExtract %v2float %357 1
%364 = OpFOrdEqual %v2bool %362 %363
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %361 %365
OpBranch %353
%353 = OpLabel
%367 = OpPhi %bool %false %345 %366 %352
OpStore %_0_ok %367
%369 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %369
%370 = OpLoad %bool %_0_ok
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %mat2v2float %_2_m3
%374 = OpCompositeConstruct %v2float %float_1 %float_2
%375 = OpCompositeConstruct %v2float %float_3 %float_4
%376 = OpCompositeConstruct %mat2v2float %374 %375
%377 = OpCompositeExtract %v2float %373 0
%378 = OpCompositeExtract %v2float %376 0
%379 = OpFOrdEqual %v2bool %377 %378
%380 = OpAll %bool %379
%381 = OpCompositeExtract %v2float %373 1
%382 = OpCompositeExtract %v2float %376 1
%383 = OpFOrdEqual %v2bool %381 %382
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %380 %384
OpBranch %372
%372 = OpLabel
%386 = OpPhi %bool %false %353 %385 %371
OpStore %_0_ok %386
%389 = OpCompositeConstruct %v2float %float_6 %float_0
%390 = OpCompositeConstruct %v2float %float_0 %float_6
%388 = OpCompositeConstruct %mat2v2float %389 %390
OpStore %_3_m4 %388
%391 = OpLoad %bool %_0_ok
OpSelectionMerge %393 None
OpBranchConditional %391 %392 %393
%392 = OpLabel
%394 = OpLoad %mat2v2float %_3_m4
%395 = OpCompositeConstruct %v2float %float_6 %float_0
%396 = OpCompositeConstruct %v2float %float_0 %float_6
%397 = OpCompositeConstruct %mat2v2float %395 %396
%398 = OpCompositeExtract %v2float %394 0
%399 = OpCompositeExtract %v2float %397 0
%400 = OpFOrdEqual %v2bool %398 %399
%401 = OpAll %bool %400
%402 = OpCompositeExtract %v2float %394 1
%403 = OpCompositeExtract %v2float %397 1
%404 = OpFOrdEqual %v2bool %402 %403
%405 = OpAll %bool %404
%406 = OpLogicalAnd %bool %401 %405
OpBranch %393
%393 = OpLabel
%407 = OpPhi %bool %false %372 %406 %392
OpStore %_0_ok %407
%408 = OpLoad %mat2v2float %_2_m3
%409 = OpLoad %mat2v2float %_3_m4
%410 = OpMatrixTimesMatrix %mat2v2float %408 %409
OpStore %_2_m3 %410
%411 = OpLoad %bool %_0_ok
OpSelectionMerge %413 None
OpBranchConditional %411 %412 %413
%412 = OpLabel
%414 = OpLoad %mat2v2float %_2_m3
%415 = OpCompositeConstruct %v2float %float_6 %float_12
%416 = OpCompositeConstruct %v2float %float_18 %float_24
%417 = OpCompositeConstruct %mat2v2float %415 %416
%418 = OpCompositeExtract %v2float %414 0
%419 = OpCompositeExtract %v2float %417 0
%420 = OpFOrdEqual %v2bool %418 %419
%421 = OpAll %bool %420
%422 = OpCompositeExtract %v2float %414 1
%423 = OpCompositeExtract %v2float %417 1
%424 = OpFOrdEqual %v2bool %422 %423
%425 = OpAll %bool %424
%426 = OpLogicalAnd %bool %421 %425
OpBranch %413
%413 = OpLabel
%427 = OpPhi %bool %false %393 %426 %412
OpStore %_0_ok %427
%429 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%430 = OpLoad %v2float %429
%431 = OpCompositeExtract %float %430 1
%433 = OpCompositeConstruct %v2float %431 %float_0
%434 = OpCompositeConstruct %v2float %float_0 %431
%432 = OpCompositeConstruct %mat2v2float %433 %434
OpStore %_4_m5 %432
%435 = OpLoad %bool %_0_ok
OpSelectionMerge %437 None
OpBranchConditional %435 %436 %437
%436 = OpLabel
%438 = OpLoad %mat2v2float %_4_m5
%439 = OpCompositeConstruct %v2float %float_4 %float_0
%440 = OpCompositeConstruct %v2float %float_0 %float_4
%441 = OpCompositeConstruct %mat2v2float %439 %440
%442 = OpCompositeExtract %v2float %438 0
%443 = OpCompositeExtract %v2float %441 0
%444 = OpFOrdEqual %v2bool %442 %443
%445 = OpAll %bool %444
%446 = OpCompositeExtract %v2float %438 1
%447 = OpCompositeExtract %v2float %441 1
%448 = OpFOrdEqual %v2bool %446 %447
%449 = OpAll %bool %448
%450 = OpLogicalAnd %bool %445 %449
OpBranch %437
%437 = OpLabel
%451 = OpPhi %bool %false %413 %450 %436
OpStore %_0_ok %451
%452 = OpLoad %mat2v2float %_1_m1
%453 = OpLoad %mat2v2float %_4_m5
%454 = OpCompositeExtract %v2float %452 0
%455 = OpCompositeExtract %v2float %453 0
%456 = OpFAdd %v2float %454 %455
%457 = OpCompositeExtract %v2float %452 1
%458 = OpCompositeExtract %v2float %453 1
%459 = OpFAdd %v2float %457 %458
%460 = OpCompositeConstruct %mat2v2float %456 %459
OpStore %_1_m1 %460
%461 = OpLoad %bool %_0_ok
OpSelectionMerge %463 None
OpBranchConditional %461 %462 %463
%462 = OpLabel
%464 = OpLoad %mat2v2float %_1_m1
%465 = OpCompositeConstruct %v2float %float_5 %float_2
%466 = OpCompositeConstruct %v2float %float_3 %float_8
%467 = OpCompositeConstruct %mat2v2float %465 %466
%468 = OpCompositeExtract %v2float %464 0
%469 = OpCompositeExtract %v2float %467 0
%470 = OpFOrdEqual %v2bool %468 %469
%471 = OpAll %bool %470
%472 = OpCompositeExtract %v2float %464 1
%473 = OpCompositeExtract %v2float %467 1
%474 = OpFOrdEqual %v2bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
OpBranch %463
%463 = OpLabel
%477 = OpPhi %bool %false %437 %476 %462
OpStore %_0_ok %477
%479 = OpCompositeConstruct %v2float %float_5 %float_6
%480 = OpCompositeConstruct %v2float %float_7 %float_8
%481 = OpCompositeConstruct %mat2v2float %479 %480
OpStore %_5_m7 %481
%482 = OpLoad %bool %_0_ok
OpSelectionMerge %484 None
OpBranchConditional %482 %483 %484
%483 = OpLabel
%485 = OpLoad %mat2v2float %_5_m7
%486 = OpCompositeConstruct %v2float %float_5 %float_6
%487 = OpCompositeConstruct %v2float %float_7 %float_8
%488 = OpCompositeConstruct %mat2v2float %486 %487
%489 = OpCompositeExtract %v2float %485 0
%490 = OpCompositeExtract %v2float %488 0
%491 = OpFOrdEqual %v2bool %489 %490
%492 = OpAll %bool %491
%493 = OpCompositeExtract %v2float %485 1
%494 = OpCompositeExtract %v2float %488 1
%495 = OpFOrdEqual %v2bool %493 %494
%496 = OpAll %bool %495
%497 = OpLogicalAnd %bool %492 %496
OpBranch %484
%484 = OpLabel
%498 = OpPhi %bool %false %463 %497 %483
OpStore %_0_ok %498
%501 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%502 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%503 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%500 = OpCompositeConstruct %mat3v3float %501 %502 %503
OpStore %_6_m9 %500
%504 = OpLoad %bool %_0_ok
OpSelectionMerge %506 None
OpBranchConditional %504 %505 %506
%505 = OpLabel
%507 = OpLoad %mat3v3float %_6_m9
%508 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%509 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%510 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%511 = OpCompositeConstruct %mat3v3float %508 %509 %510
%512 = OpCompositeExtract %v3float %507 0
%513 = OpCompositeExtract %v3float %511 0
%514 = OpFOrdEqual %v3bool %512 %513
%515 = OpAll %bool %514
%516 = OpCompositeExtract %v3float %507 1
%517 = OpCompositeExtract %v3float %511 1
%518 = OpFOrdEqual %v3bool %516 %517
%519 = OpAll %bool %518
%520 = OpLogicalAnd %bool %515 %519
%521 = OpCompositeExtract %v3float %507 2
%522 = OpCompositeExtract %v3float %511 2
%523 = OpFOrdEqual %v3bool %521 %522
%524 = OpAll %bool %523
%525 = OpLogicalAnd %bool %520 %524
OpBranch %506
%506 = OpLabel
%526 = OpPhi %bool %false %484 %525 %505
OpStore %_0_ok %526
%529 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%530 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%531 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%532 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%528 = OpCompositeConstruct %mat4v4float %529 %530 %531 %532
OpStore %_7_m10 %528
%533 = OpLoad %bool %_0_ok
OpSelectionMerge %535 None
OpBranchConditional %533 %534 %535
%534 = OpLabel
%536 = OpLoad %mat4v4float %_7_m10
%537 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%538 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%539 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%540 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%541 = OpCompositeConstruct %mat4v4float %537 %538 %539 %540
%542 = OpCompositeExtract %v4float %536 0
%543 = OpCompositeExtract %v4float %541 0
%544 = OpFOrdEqual %v4bool %542 %543
%545 = OpAll %bool %544
%546 = OpCompositeExtract %v4float %536 1
%547 = OpCompositeExtract %v4float %541 1
%548 = OpFOrdEqual %v4bool %546 %547
%549 = OpAll %bool %548
%550 = OpLogicalAnd %bool %545 %549
%551 = OpCompositeExtract %v4float %536 2
%552 = OpCompositeExtract %v4float %541 2
%553 = OpFOrdEqual %v4bool %551 %552
%554 = OpAll %bool %553
%555 = OpLogicalAnd %bool %550 %554
%556 = OpCompositeExtract %v4float %536 3
%557 = OpCompositeExtract %v4float %541 3
%558 = OpFOrdEqual %v4bool %556 %557
%559 = OpAll %bool %558
%560 = OpLogicalAnd %bool %555 %559
OpBranch %535
%535 = OpLabel
%561 = OpPhi %bool %false %506 %560 %534
OpStore %_0_ok %561
%563 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%564 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%565 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%566 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%567 = OpCompositeConstruct %mat4v4float %563 %564 %565 %566
OpStore %_8_m11 %567
%568 = OpLoad %mat4v4float %_8_m11
%569 = OpLoad %mat4v4float %_7_m10
%570 = OpCompositeExtract %v4float %568 0
%571 = OpCompositeExtract %v4float %569 0
%572 = OpFSub %v4float %570 %571
%573 = OpCompositeExtract %v4float %568 1
%574 = OpCompositeExtract %v4float %569 1
%575 = OpFSub %v4float %573 %574
%576 = OpCompositeExtract %v4float %568 2
%577 = OpCompositeExtract %v4float %569 2
%578 = OpFSub %v4float %576 %577
%579 = OpCompositeExtract %v4float %568 3
%580 = OpCompositeExtract %v4float %569 3
%581 = OpFSub %v4float %579 %580
%582 = OpCompositeConstruct %mat4v4float %572 %575 %578 %581
OpStore %_8_m11 %582
%583 = OpLoad %bool %_0_ok
OpSelectionMerge %585 None
OpBranchConditional %583 %584 %585
%584 = OpLabel
%586 = OpLoad %mat4v4float %_8_m11
%587 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%588 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%589 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%590 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%591 = OpCompositeConstruct %mat4v4float %587 %588 %589 %590
%592 = OpCompositeExtract %v4float %586 0
%593 = OpCompositeExtract %v4float %591 0
%594 = OpFOrdEqual %v4bool %592 %593
%595 = OpAll %bool %594
%596 = OpCompositeExtract %v4float %586 1
%597 = OpCompositeExtract %v4float %591 1
%598 = OpFOrdEqual %v4bool %596 %597
%599 = OpAll %bool %598
%600 = OpLogicalAnd %bool %595 %599
%601 = OpCompositeExtract %v4float %586 2
%602 = OpCompositeExtract %v4float %591 2
%603 = OpFOrdEqual %v4bool %601 %602
%604 = OpAll %bool %603
%605 = OpLogicalAnd %bool %600 %604
%606 = OpCompositeExtract %v4float %586 3
%607 = OpCompositeExtract %v4float %591 3
%608 = OpFOrdEqual %v4bool %606 %607
%609 = OpAll %bool %608
%610 = OpLogicalAnd %bool %605 %609
OpBranch %585
%585 = OpLabel
%611 = OpPhi %bool %false %535 %610 %584
OpStore %_0_ok %611
%612 = OpLoad %bool %_0_ok
OpSelectionMerge %614 None
OpBranchConditional %612 %613 %614
%613 = OpLabel
%615 = OpFunctionCall %bool %test_half_b
OpBranch %614
%614 = OpLabel
%616 = OpPhi %bool %false %585 %615 %613
OpSelectionMerge %618 None
OpBranchConditional %616 %617 %618
%617 = OpLabel
%619 = OpFunctionCall %bool %test_comma_b
OpBranch %618
%618 = OpLabel
%620 = OpPhi %bool %false %614 %619 %617
OpSelectionMerge %625 None
OpBranchConditional %620 %623 %624
%623 = OpLabel
%626 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%629 = OpLoad %v4float %626
OpStore %621 %629
OpBranch %625
%624 = OpLabel
%630 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%631 = OpLoad %v4float %630
OpStore %621 %631
OpBranch %625
%625 = OpLabel
%632 = OpLoad %v4float %621
OpReturnValue %632
OpFunctionEnd
