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
OpDecorate %353 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %535 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %631 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%345 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%330 = OpCompositeConstruct %v2float %float_2 %float_4
%331 = OpCompositeConstruct %v2float %float_6 %float_8
%332 = OpCompositeConstruct %mat2v2float %330 %331
%333 = OpMatrixTimesScalar %mat2v2float %332 %float_0_5
OpStore %y %333
%334 = OpLoad %mat2v2float %x
%335 = OpLoad %mat2v2float %y
%336 = OpCompositeExtract %v2float %334 0
%337 = OpCompositeExtract %v2float %335 0
%338 = OpFOrdEqual %v2bool %336 %337
%339 = OpAll %bool %338
%340 = OpCompositeExtract %v2float %334 1
%341 = OpCompositeExtract %v2float %335 1
%342 = OpFOrdEqual %v2bool %340 %341
%343 = OpAll %bool %342
%344 = OpLogicalAnd %bool %339 %343
OpReturnValue %344
OpFunctionEnd
%main = OpFunction %v4float None %345
%346 = OpFunctionParameter %_ptr_Function_v2float
%347 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%623 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%350 = OpCompositeConstruct %v2float %float_1 %float_2
%351 = OpCompositeConstruct %v2float %float_3 %float_4
%352 = OpCompositeConstruct %mat2v2float %350 %351
OpStore %_1_m1 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v2float %_1_m1
%357 = OpCompositeConstruct %v2float %float_1 %float_2
%358 = OpCompositeConstruct %v2float %float_3 %float_4
%359 = OpCompositeConstruct %mat2v2float %357 %358
%360 = OpCompositeExtract %v2float %356 0
%361 = OpCompositeExtract %v2float %359 0
%362 = OpFOrdEqual %v2bool %360 %361
%363 = OpAll %bool %362
%364 = OpCompositeExtract %v2float %356 1
%365 = OpCompositeExtract %v2float %359 1
%366 = OpFOrdEqual %v2bool %364 %365
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %363 %367
OpBranch %355
%355 = OpLabel
%369 = OpPhi %bool %false %347 %368 %354
OpStore %_0_ok %369
%371 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %371
%372 = OpLoad %bool %_0_ok
OpSelectionMerge %374 None
OpBranchConditional %372 %373 %374
%373 = OpLabel
%375 = OpLoad %mat2v2float %_2_m3
%376 = OpCompositeConstruct %v2float %float_1 %float_2
%377 = OpCompositeConstruct %v2float %float_3 %float_4
%378 = OpCompositeConstruct %mat2v2float %376 %377
%379 = OpCompositeExtract %v2float %375 0
%380 = OpCompositeExtract %v2float %378 0
%381 = OpFOrdEqual %v2bool %379 %380
%382 = OpAll %bool %381
%383 = OpCompositeExtract %v2float %375 1
%384 = OpCompositeExtract %v2float %378 1
%385 = OpFOrdEqual %v2bool %383 %384
%386 = OpAll %bool %385
%387 = OpLogicalAnd %bool %382 %386
OpBranch %374
%374 = OpLabel
%388 = OpPhi %bool %false %355 %387 %373
OpStore %_0_ok %388
%391 = OpCompositeConstruct %v2float %float_6 %float_0
%392 = OpCompositeConstruct %v2float %float_0 %float_6
%390 = OpCompositeConstruct %mat2v2float %391 %392
OpStore %_3_m4 %390
%393 = OpLoad %bool %_0_ok
OpSelectionMerge %395 None
OpBranchConditional %393 %394 %395
%394 = OpLabel
%396 = OpLoad %mat2v2float %_3_m4
%397 = OpCompositeConstruct %v2float %float_6 %float_0
%398 = OpCompositeConstruct %v2float %float_0 %float_6
%399 = OpCompositeConstruct %mat2v2float %397 %398
%400 = OpCompositeExtract %v2float %396 0
%401 = OpCompositeExtract %v2float %399 0
%402 = OpFOrdEqual %v2bool %400 %401
%403 = OpAll %bool %402
%404 = OpCompositeExtract %v2float %396 1
%405 = OpCompositeExtract %v2float %399 1
%406 = OpFOrdEqual %v2bool %404 %405
%407 = OpAll %bool %406
%408 = OpLogicalAnd %bool %403 %407
OpBranch %395
%395 = OpLabel
%409 = OpPhi %bool %false %374 %408 %394
OpStore %_0_ok %409
%410 = OpLoad %mat2v2float %_2_m3
%411 = OpLoad %mat2v2float %_3_m4
%412 = OpMatrixTimesMatrix %mat2v2float %410 %411
OpStore %_2_m3 %412
%413 = OpLoad %bool %_0_ok
OpSelectionMerge %415 None
OpBranchConditional %413 %414 %415
%414 = OpLabel
%416 = OpLoad %mat2v2float %_2_m3
%417 = OpCompositeConstruct %v2float %float_6 %float_12
%418 = OpCompositeConstruct %v2float %float_18 %float_24
%419 = OpCompositeConstruct %mat2v2float %417 %418
%420 = OpCompositeExtract %v2float %416 0
%421 = OpCompositeExtract %v2float %419 0
%422 = OpFOrdEqual %v2bool %420 %421
%423 = OpAll %bool %422
%424 = OpCompositeExtract %v2float %416 1
%425 = OpCompositeExtract %v2float %419 1
%426 = OpFOrdEqual %v2bool %424 %425
%427 = OpAll %bool %426
%428 = OpLogicalAnd %bool %423 %427
OpBranch %415
%415 = OpLabel
%429 = OpPhi %bool %false %395 %428 %414
OpStore %_0_ok %429
%431 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%432 = OpLoad %v2float %431
%433 = OpCompositeExtract %float %432 1
%435 = OpCompositeConstruct %v2float %433 %float_0
%436 = OpCompositeConstruct %v2float %float_0 %433
%434 = OpCompositeConstruct %mat2v2float %435 %436
OpStore %_4_m5 %434
%437 = OpLoad %bool %_0_ok
OpSelectionMerge %439 None
OpBranchConditional %437 %438 %439
%438 = OpLabel
%440 = OpLoad %mat2v2float %_4_m5
%441 = OpCompositeConstruct %v2float %float_4 %float_0
%442 = OpCompositeConstruct %v2float %float_0 %float_4
%443 = OpCompositeConstruct %mat2v2float %441 %442
%444 = OpCompositeExtract %v2float %440 0
%445 = OpCompositeExtract %v2float %443 0
%446 = OpFOrdEqual %v2bool %444 %445
%447 = OpAll %bool %446
%448 = OpCompositeExtract %v2float %440 1
%449 = OpCompositeExtract %v2float %443 1
%450 = OpFOrdEqual %v2bool %448 %449
%451 = OpAll %bool %450
%452 = OpLogicalAnd %bool %447 %451
OpBranch %439
%439 = OpLabel
%453 = OpPhi %bool %false %415 %452 %438
OpStore %_0_ok %453
%454 = OpLoad %mat2v2float %_1_m1
%455 = OpLoad %mat2v2float %_4_m5
%456 = OpCompositeExtract %v2float %454 0
%457 = OpCompositeExtract %v2float %455 0
%458 = OpFAdd %v2float %456 %457
%459 = OpCompositeExtract %v2float %454 1
%460 = OpCompositeExtract %v2float %455 1
%461 = OpFAdd %v2float %459 %460
%462 = OpCompositeConstruct %mat2v2float %458 %461
OpStore %_1_m1 %462
%463 = OpLoad %bool %_0_ok
OpSelectionMerge %465 None
OpBranchConditional %463 %464 %465
%464 = OpLabel
%466 = OpLoad %mat2v2float %_1_m1
%467 = OpCompositeConstruct %v2float %float_5 %float_2
%468 = OpCompositeConstruct %v2float %float_3 %float_8
%469 = OpCompositeConstruct %mat2v2float %467 %468
%470 = OpCompositeExtract %v2float %466 0
%471 = OpCompositeExtract %v2float %469 0
%472 = OpFOrdEqual %v2bool %470 %471
%473 = OpAll %bool %472
%474 = OpCompositeExtract %v2float %466 1
%475 = OpCompositeExtract %v2float %469 1
%476 = OpFOrdEqual %v2bool %474 %475
%477 = OpAll %bool %476
%478 = OpLogicalAnd %bool %473 %477
OpBranch %465
%465 = OpLabel
%479 = OpPhi %bool %false %439 %478 %464
OpStore %_0_ok %479
%481 = OpCompositeConstruct %v2float %float_5 %float_6
%482 = OpCompositeConstruct %v2float %float_7 %float_8
%483 = OpCompositeConstruct %mat2v2float %481 %482
OpStore %_5_m7 %483
%484 = OpLoad %bool %_0_ok
OpSelectionMerge %486 None
OpBranchConditional %484 %485 %486
%485 = OpLabel
%487 = OpLoad %mat2v2float %_5_m7
%488 = OpCompositeConstruct %v2float %float_5 %float_6
%489 = OpCompositeConstruct %v2float %float_7 %float_8
%490 = OpCompositeConstruct %mat2v2float %488 %489
%491 = OpCompositeExtract %v2float %487 0
%492 = OpCompositeExtract %v2float %490 0
%493 = OpFOrdEqual %v2bool %491 %492
%494 = OpAll %bool %493
%495 = OpCompositeExtract %v2float %487 1
%496 = OpCompositeExtract %v2float %490 1
%497 = OpFOrdEqual %v2bool %495 %496
%498 = OpAll %bool %497
%499 = OpLogicalAnd %bool %494 %498
OpBranch %486
%486 = OpLabel
%500 = OpPhi %bool %false %465 %499 %485
OpStore %_0_ok %500
%503 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%504 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%505 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%502 = OpCompositeConstruct %mat3v3float %503 %504 %505
OpStore %_6_m9 %502
%506 = OpLoad %bool %_0_ok
OpSelectionMerge %508 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%509 = OpLoad %mat3v3float %_6_m9
%510 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%511 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%512 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%513 = OpCompositeConstruct %mat3v3float %510 %511 %512
%514 = OpCompositeExtract %v3float %509 0
%515 = OpCompositeExtract %v3float %513 0
%516 = OpFOrdEqual %v3bool %514 %515
%517 = OpAll %bool %516
%518 = OpCompositeExtract %v3float %509 1
%519 = OpCompositeExtract %v3float %513 1
%520 = OpFOrdEqual %v3bool %518 %519
%521 = OpAll %bool %520
%522 = OpLogicalAnd %bool %517 %521
%523 = OpCompositeExtract %v3float %509 2
%524 = OpCompositeExtract %v3float %513 2
%525 = OpFOrdEqual %v3bool %523 %524
%526 = OpAll %bool %525
%527 = OpLogicalAnd %bool %522 %526
OpBranch %508
%508 = OpLabel
%528 = OpPhi %bool %false %486 %527 %507
OpStore %_0_ok %528
%531 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%532 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%533 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%534 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%530 = OpCompositeConstruct %mat4v4float %531 %532 %533 %534
OpStore %_7_m10 %530
%535 = OpLoad %bool %_0_ok
OpSelectionMerge %537 None
OpBranchConditional %535 %536 %537
%536 = OpLabel
%538 = OpLoad %mat4v4float %_7_m10
%539 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%540 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%541 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%542 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%543 = OpCompositeConstruct %mat4v4float %539 %540 %541 %542
%544 = OpCompositeExtract %v4float %538 0
%545 = OpCompositeExtract %v4float %543 0
%546 = OpFOrdEqual %v4bool %544 %545
%547 = OpAll %bool %546
%548 = OpCompositeExtract %v4float %538 1
%549 = OpCompositeExtract %v4float %543 1
%550 = OpFOrdEqual %v4bool %548 %549
%551 = OpAll %bool %550
%552 = OpLogicalAnd %bool %547 %551
%553 = OpCompositeExtract %v4float %538 2
%554 = OpCompositeExtract %v4float %543 2
%555 = OpFOrdEqual %v4bool %553 %554
%556 = OpAll %bool %555
%557 = OpLogicalAnd %bool %552 %556
%558 = OpCompositeExtract %v4float %538 3
%559 = OpCompositeExtract %v4float %543 3
%560 = OpFOrdEqual %v4bool %558 %559
%561 = OpAll %bool %560
%562 = OpLogicalAnd %bool %557 %561
OpBranch %537
%537 = OpLabel
%563 = OpPhi %bool %false %508 %562 %536
OpStore %_0_ok %563
%565 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%566 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%567 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%568 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%569 = OpCompositeConstruct %mat4v4float %565 %566 %567 %568
OpStore %_8_m11 %569
%570 = OpLoad %mat4v4float %_8_m11
%571 = OpLoad %mat4v4float %_7_m10
%572 = OpCompositeExtract %v4float %570 0
%573 = OpCompositeExtract %v4float %571 0
%574 = OpFSub %v4float %572 %573
%575 = OpCompositeExtract %v4float %570 1
%576 = OpCompositeExtract %v4float %571 1
%577 = OpFSub %v4float %575 %576
%578 = OpCompositeExtract %v4float %570 2
%579 = OpCompositeExtract %v4float %571 2
%580 = OpFSub %v4float %578 %579
%581 = OpCompositeExtract %v4float %570 3
%582 = OpCompositeExtract %v4float %571 3
%583 = OpFSub %v4float %581 %582
%584 = OpCompositeConstruct %mat4v4float %574 %577 %580 %583
OpStore %_8_m11 %584
%585 = OpLoad %bool %_0_ok
OpSelectionMerge %587 None
OpBranchConditional %585 %586 %587
%586 = OpLabel
%588 = OpLoad %mat4v4float %_8_m11
%589 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%590 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%591 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%592 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%593 = OpCompositeConstruct %mat4v4float %589 %590 %591 %592
%594 = OpCompositeExtract %v4float %588 0
%595 = OpCompositeExtract %v4float %593 0
%596 = OpFOrdEqual %v4bool %594 %595
%597 = OpAll %bool %596
%598 = OpCompositeExtract %v4float %588 1
%599 = OpCompositeExtract %v4float %593 1
%600 = OpFOrdEqual %v4bool %598 %599
%601 = OpAll %bool %600
%602 = OpLogicalAnd %bool %597 %601
%603 = OpCompositeExtract %v4float %588 2
%604 = OpCompositeExtract %v4float %593 2
%605 = OpFOrdEqual %v4bool %603 %604
%606 = OpAll %bool %605
%607 = OpLogicalAnd %bool %602 %606
%608 = OpCompositeExtract %v4float %588 3
%609 = OpCompositeExtract %v4float %593 3
%610 = OpFOrdEqual %v4bool %608 %609
%611 = OpAll %bool %610
%612 = OpLogicalAnd %bool %607 %611
OpBranch %587
%587 = OpLabel
%613 = OpPhi %bool %false %537 %612 %586
OpStore %_0_ok %613
%614 = OpLoad %bool %_0_ok
OpSelectionMerge %616 None
OpBranchConditional %614 %615 %616
%615 = OpLabel
%617 = OpFunctionCall %bool %test_half_b
OpBranch %616
%616 = OpLabel
%618 = OpPhi %bool %false %587 %617 %615
OpSelectionMerge %620 None
OpBranchConditional %618 %619 %620
%619 = OpLabel
%621 = OpFunctionCall %bool %test_comma_b
OpBranch %620
%620 = OpLabel
%622 = OpPhi %bool %false %616 %621 %619
OpSelectionMerge %627 None
OpBranchConditional %622 %625 %626
%625 = OpLabel
%628 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%631 = OpLoad %v4float %628
OpStore %623 %631
OpBranch %627
%626 = OpLabel
%632 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%633 = OpLoad %v4float %632
OpStore %623 %633
OpBranch %627
%627 = OpLabel
%634 = OpLoad %v4float %623
OpReturnValue %634
OpFunctionEnd
