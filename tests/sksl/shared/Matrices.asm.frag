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
OpDecorate %m3 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
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
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
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
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%83 = OpConstantComposite %v2float %float_6 %float_0
%84 = OpConstantComposite %v2float %float_0 %float_6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%106 = OpConstantComposite %v2float %float_6 %float_12
%107 = OpConstantComposite %v2float %float_18 %float_24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%132 = OpConstantComposite %v2float %float_4 %float_0
%133 = OpConstantComposite %v2float %float_0 %float_4
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%160 = OpConstantComposite %v2float %float_5 %float_2
%161 = OpConstantComposite %v2float %float_3 %float_8
%float_7 = OpConstant %float 7
%175 = OpConstantComposite %v2float %float_5 %float_6
%176 = OpConstantComposite %v2float %float_7 %float_8
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%206 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%207 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%208 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%239 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%240 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%241 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%242 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%267 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%288 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%289 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%290 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%291 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%330 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%39 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %mat2v2float %37 %38
%47 = OpCompositeExtract %v2float %44 0
%48 = OpCompositeExtract %v2float %45 0
%49 = OpFOrdEqual %v2bool %47 %48
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v2float %44 1
%52 = OpCompositeExtract %v2float %45 1
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %50 %54
OpBranch %43
%43 = OpLabel
%56 = OpPhi %bool %false %26 %55 %42
OpStore %ok %56
%58 = OpLoad %mat2v2float %m1
OpStore %m3 %58
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %mat2v2float %m3
%63 = OpCompositeConstruct %mat2v2float %37 %38
%64 = OpCompositeExtract %v2float %62 0
%65 = OpCompositeExtract %v2float %63 0
%66 = OpFOrdEqual %v2bool %64 %65
%67 = OpAll %bool %66
%68 = OpCompositeExtract %v2float %62 1
%69 = OpCompositeExtract %v2float %63 1
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpLogicalAnd %bool %67 %71
OpBranch %61
%61 = OpLabel
%73 = OpPhi %bool %false %43 %72 %60
OpStore %ok %73
%77 = OpCompositeConstruct %v2float %float_6 %float_0
%78 = OpCompositeConstruct %v2float %float_0 %float_6
%76 = OpCompositeConstruct %mat2v2float %77 %78
OpStore %m4 %76
%79 = OpLoad %bool %ok
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %mat2v2float %m4
%85 = OpCompositeConstruct %mat2v2float %83 %84
%86 = OpCompositeExtract %v2float %82 0
%87 = OpCompositeExtract %v2float %85 0
%88 = OpFOrdEqual %v2bool %86 %87
%89 = OpAll %bool %88
%90 = OpCompositeExtract %v2float %82 1
%91 = OpCompositeExtract %v2float %85 1
%92 = OpFOrdEqual %v2bool %90 %91
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %89 %93
OpBranch %81
%81 = OpLabel
%95 = OpPhi %bool %false %61 %94 %80
OpStore %ok %95
%96 = OpLoad %mat2v2float %m3
%97 = OpLoad %mat2v2float %m4
%98 = OpMatrixTimesMatrix %mat2v2float %96 %97
OpStore %m3 %98
%99 = OpLoad %bool %ok
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %mat2v2float %m3
%108 = OpCompositeConstruct %mat2v2float %106 %107
%109 = OpCompositeExtract %v2float %102 0
%110 = OpCompositeExtract %v2float %108 0
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpCompositeExtract %v2float %102 1
%114 = OpCompositeExtract %v2float %108 1
%115 = OpFOrdEqual %v2bool %113 %114
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %112 %116
OpBranch %101
%101 = OpLabel
%118 = OpPhi %bool %false %81 %117 %100
OpStore %ok %118
%122 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%123 = OpLoad %v2float %122
%124 = OpCompositeExtract %float %123 1
%126 = OpCompositeConstruct %v2float %124 %float_0
%127 = OpCompositeConstruct %v2float %float_0 %124
%125 = OpCompositeConstruct %mat2v2float %126 %127
OpStore %m5 %125
%128 = OpLoad %bool %ok
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpLoad %mat2v2float %m5
%134 = OpCompositeConstruct %mat2v2float %132 %133
%135 = OpCompositeExtract %v2float %131 0
%136 = OpCompositeExtract %v2float %134 0
%137 = OpFOrdEqual %v2bool %135 %136
%138 = OpAll %bool %137
%139 = OpCompositeExtract %v2float %131 1
%140 = OpCompositeExtract %v2float %134 1
%141 = OpFOrdEqual %v2bool %139 %140
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %138 %142
OpBranch %130
%130 = OpLabel
%144 = OpPhi %bool %false %101 %143 %129
OpStore %ok %144
%145 = OpLoad %mat2v2float %m1
%146 = OpLoad %mat2v2float %m5
%147 = OpCompositeExtract %v2float %145 0
%148 = OpCompositeExtract %v2float %146 0
%149 = OpFAdd %v2float %147 %148
%150 = OpCompositeExtract %v2float %145 1
%151 = OpCompositeExtract %v2float %146 1
%152 = OpFAdd %v2float %150 %151
%153 = OpCompositeConstruct %mat2v2float %149 %152
OpStore %m1 %153
%154 = OpLoad %bool %ok
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %mat2v2float %m1
%162 = OpCompositeConstruct %mat2v2float %160 %161
%163 = OpCompositeExtract %v2float %157 0
%164 = OpCompositeExtract %v2float %162 0
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpCompositeExtract %v2float %157 1
%168 = OpCompositeExtract %v2float %162 1
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %166 %170
OpBranch %156
%156 = OpLabel
%172 = OpPhi %bool %false %130 %171 %155
OpStore %ok %172
%177 = OpCompositeConstruct %mat2v2float %175 %176
OpStore %m7 %177
%178 = OpLoad %bool %ok
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %mat2v2float %m7
%182 = OpCompositeConstruct %mat2v2float %175 %176
%183 = OpCompositeExtract %v2float %181 0
%184 = OpCompositeExtract %v2float %182 0
%185 = OpFOrdEqual %v2bool %183 %184
%186 = OpAll %bool %185
%187 = OpCompositeExtract %v2float %181 1
%188 = OpCompositeExtract %v2float %182 1
%189 = OpFOrdEqual %v2bool %187 %188
%190 = OpAll %bool %189
%191 = OpLogicalAnd %bool %186 %190
OpBranch %180
%180 = OpLabel
%192 = OpPhi %bool %false %156 %191 %179
OpStore %ok %192
%199 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%200 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%201 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%198 = OpCompositeConstruct %mat3v3float %199 %200 %201
OpStore %m9 %198
%202 = OpLoad %bool %ok
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %mat3v3float %m9
%209 = OpCompositeConstruct %mat3v3float %206 %207 %208
%211 = OpCompositeExtract %v3float %205 0
%212 = OpCompositeExtract %v3float %209 0
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v3float %205 1
%216 = OpCompositeExtract %v3float %209 1
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
%220 = OpCompositeExtract %v3float %205 2
%221 = OpCompositeExtract %v3float %209 2
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpLogicalAnd %bool %219 %223
OpBranch %204
%204 = OpLabel
%225 = OpPhi %bool %false %180 %224 %203
OpStore %ok %225
%231 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%232 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%233 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%234 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%230 = OpCompositeConstruct %mat4v4float %231 %232 %233 %234
OpStore %m10 %230
%235 = OpLoad %bool %ok
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpLoad %mat4v4float %m10
%243 = OpCompositeConstruct %mat4v4float %239 %240 %241 %242
%245 = OpCompositeExtract %v4float %238 0
%246 = OpCompositeExtract %v4float %243 0
%247 = OpFOrdEqual %v4bool %245 %246
%248 = OpAll %bool %247
%249 = OpCompositeExtract %v4float %238 1
%250 = OpCompositeExtract %v4float %243 1
%251 = OpFOrdEqual %v4bool %249 %250
%252 = OpAll %bool %251
%253 = OpLogicalAnd %bool %248 %252
%254 = OpCompositeExtract %v4float %238 2
%255 = OpCompositeExtract %v4float %243 2
%256 = OpFOrdEqual %v4bool %254 %255
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %253 %257
%259 = OpCompositeExtract %v4float %238 3
%260 = OpCompositeExtract %v4float %243 3
%261 = OpFOrdEqual %v4bool %259 %260
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %258 %262
OpBranch %237
%237 = OpLabel
%264 = OpPhi %bool %false %204 %263 %236
OpStore %ok %264
%268 = OpCompositeConstruct %mat4v4float %267 %267 %267 %267
OpStore %m11 %268
%269 = OpLoad %mat4v4float %m11
%270 = OpLoad %mat4v4float %m10
%271 = OpCompositeExtract %v4float %269 0
%272 = OpCompositeExtract %v4float %270 0
%273 = OpFSub %v4float %271 %272
%274 = OpCompositeExtract %v4float %269 1
%275 = OpCompositeExtract %v4float %270 1
%276 = OpFSub %v4float %274 %275
%277 = OpCompositeExtract %v4float %269 2
%278 = OpCompositeExtract %v4float %270 2
%279 = OpFSub %v4float %277 %278
%280 = OpCompositeExtract %v4float %269 3
%281 = OpCompositeExtract %v4float %270 3
%282 = OpFSub %v4float %280 %281
%283 = OpCompositeConstruct %mat4v4float %273 %276 %279 %282
OpStore %m11 %283
%284 = OpLoad %bool %ok
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpLoad %mat4v4float %m11
%292 = OpCompositeConstruct %mat4v4float %288 %289 %290 %291
%293 = OpCompositeExtract %v4float %287 0
%294 = OpCompositeExtract %v4float %292 0
%295 = OpFOrdEqual %v4bool %293 %294
%296 = OpAll %bool %295
%297 = OpCompositeExtract %v4float %287 1
%298 = OpCompositeExtract %v4float %292 1
%299 = OpFOrdEqual %v4bool %297 %298
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %296 %300
%302 = OpCompositeExtract %v4float %287 2
%303 = OpCompositeExtract %v4float %292 2
%304 = OpFOrdEqual %v4bool %302 %303
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %301 %305
%307 = OpCompositeExtract %v4float %287 3
%308 = OpCompositeExtract %v4float %292 3
%309 = OpFOrdEqual %v4bool %307 %308
%310 = OpAll %bool %309
%311 = OpLogicalAnd %bool %306 %310
OpBranch %286
%286 = OpLabel
%312 = OpPhi %bool %false %237 %311 %285
OpStore %ok %312
%313 = OpLoad %bool %ok
OpReturnValue %313
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%314 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%317 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %x %317
%318 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %y %318
%319 = OpLoad %mat2v2float %x
%320 = OpLoad %mat2v2float %y
%321 = OpCompositeExtract %v2float %319 0
%322 = OpCompositeExtract %v2float %320 0
%323 = OpFOrdEqual %v2bool %321 %322
%324 = OpAll %bool %323
%325 = OpCompositeExtract %v2float %319 1
%326 = OpCompositeExtract %v2float %320 1
%327 = OpFOrdEqual %v2bool %325 %326
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %324 %328
OpReturnValue %329
OpFunctionEnd
%main = OpFunction %v4float None %330
%331 = OpFunctionParameter %_ptr_Function_v2float
%332 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%575 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%335 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %_1_m1 %335
%336 = OpLoad %bool %_0_ok
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %mat2v2float %_1_m1
%340 = OpCompositeConstruct %mat2v2float %37 %38
%341 = OpCompositeExtract %v2float %339 0
%342 = OpCompositeExtract %v2float %340 0
%343 = OpFOrdEqual %v2bool %341 %342
%344 = OpAll %bool %343
%345 = OpCompositeExtract %v2float %339 1
%346 = OpCompositeExtract %v2float %340 1
%347 = OpFOrdEqual %v2bool %345 %346
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %344 %348
OpBranch %338
%338 = OpLabel
%350 = OpPhi %bool %false %332 %349 %337
OpStore %_0_ok %350
%352 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v2float %_2_m3
%357 = OpCompositeConstruct %mat2v2float %37 %38
%358 = OpCompositeExtract %v2float %356 0
%359 = OpCompositeExtract %v2float %357 0
%360 = OpFOrdEqual %v2bool %358 %359
%361 = OpAll %bool %360
%362 = OpCompositeExtract %v2float %356 1
%363 = OpCompositeExtract %v2float %357 1
%364 = OpFOrdEqual %v2bool %362 %363
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %361 %365
OpBranch %355
%355 = OpLabel
%367 = OpPhi %bool %false %338 %366 %354
OpStore %_0_ok %367
%370 = OpCompositeConstruct %v2float %float_6 %float_0
%371 = OpCompositeConstruct %v2float %float_0 %float_6
%369 = OpCompositeConstruct %mat2v2float %370 %371
OpStore %_3_m4 %369
%372 = OpLoad %bool %_0_ok
OpSelectionMerge %374 None
OpBranchConditional %372 %373 %374
%373 = OpLabel
%375 = OpLoad %mat2v2float %_3_m4
%376 = OpCompositeConstruct %mat2v2float %83 %84
%377 = OpCompositeExtract %v2float %375 0
%378 = OpCompositeExtract %v2float %376 0
%379 = OpFOrdEqual %v2bool %377 %378
%380 = OpAll %bool %379
%381 = OpCompositeExtract %v2float %375 1
%382 = OpCompositeExtract %v2float %376 1
%383 = OpFOrdEqual %v2bool %381 %382
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %380 %384
OpBranch %374
%374 = OpLabel
%386 = OpPhi %bool %false %355 %385 %373
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
%394 = OpCompositeConstruct %mat2v2float %106 %107
%395 = OpCompositeExtract %v2float %393 0
%396 = OpCompositeExtract %v2float %394 0
%397 = OpFOrdEqual %v2bool %395 %396
%398 = OpAll %bool %397
%399 = OpCompositeExtract %v2float %393 1
%400 = OpCompositeExtract %v2float %394 1
%401 = OpFOrdEqual %v2bool %399 %400
%402 = OpAll %bool %401
%403 = OpLogicalAnd %bool %398 %402
OpBranch %392
%392 = OpLabel
%404 = OpPhi %bool %false %374 %403 %391
OpStore %_0_ok %404
%406 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%407 = OpLoad %v2float %406
%408 = OpCompositeExtract %float %407 1
%410 = OpCompositeConstruct %v2float %408 %float_0
%411 = OpCompositeConstruct %v2float %float_0 %408
%409 = OpCompositeConstruct %mat2v2float %410 %411
OpStore %_4_m5 %409
%412 = OpLoad %bool %_0_ok
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpLoad %mat2v2float %_4_m5
%416 = OpCompositeConstruct %mat2v2float %132 %133
%417 = OpCompositeExtract %v2float %415 0
%418 = OpCompositeExtract %v2float %416 0
%419 = OpFOrdEqual %v2bool %417 %418
%420 = OpAll %bool %419
%421 = OpCompositeExtract %v2float %415 1
%422 = OpCompositeExtract %v2float %416 1
%423 = OpFOrdEqual %v2bool %421 %422
%424 = OpAll %bool %423
%425 = OpLogicalAnd %bool %420 %424
OpBranch %414
%414 = OpLabel
%426 = OpPhi %bool %false %392 %425 %413
OpStore %_0_ok %426
%427 = OpLoad %mat2v2float %_1_m1
%428 = OpLoad %mat2v2float %_4_m5
%429 = OpCompositeExtract %v2float %427 0
%430 = OpCompositeExtract %v2float %428 0
%431 = OpFAdd %v2float %429 %430
%432 = OpCompositeExtract %v2float %427 1
%433 = OpCompositeExtract %v2float %428 1
%434 = OpFAdd %v2float %432 %433
%435 = OpCompositeConstruct %mat2v2float %431 %434
OpStore %_1_m1 %435
%436 = OpLoad %bool %_0_ok
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%439 = OpLoad %mat2v2float %_1_m1
%440 = OpCompositeConstruct %mat2v2float %160 %161
%441 = OpCompositeExtract %v2float %439 0
%442 = OpCompositeExtract %v2float %440 0
%443 = OpFOrdEqual %v2bool %441 %442
%444 = OpAll %bool %443
%445 = OpCompositeExtract %v2float %439 1
%446 = OpCompositeExtract %v2float %440 1
%447 = OpFOrdEqual %v2bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
OpBranch %438
%438 = OpLabel
%450 = OpPhi %bool %false %414 %449 %437
OpStore %_0_ok %450
%452 = OpCompositeConstruct %mat2v2float %175 %176
OpStore %_5_m7 %452
%453 = OpLoad %bool %_0_ok
OpSelectionMerge %455 None
OpBranchConditional %453 %454 %455
%454 = OpLabel
%456 = OpLoad %mat2v2float %_5_m7
%457 = OpCompositeConstruct %mat2v2float %175 %176
%458 = OpCompositeExtract %v2float %456 0
%459 = OpCompositeExtract %v2float %457 0
%460 = OpFOrdEqual %v2bool %458 %459
%461 = OpAll %bool %460
%462 = OpCompositeExtract %v2float %456 1
%463 = OpCompositeExtract %v2float %457 1
%464 = OpFOrdEqual %v2bool %462 %463
%465 = OpAll %bool %464
%466 = OpLogicalAnd %bool %461 %465
OpBranch %455
%455 = OpLabel
%467 = OpPhi %bool %false %438 %466 %454
OpStore %_0_ok %467
%470 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%471 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%472 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%469 = OpCompositeConstruct %mat3v3float %470 %471 %472
OpStore %_6_m9 %469
%473 = OpLoad %bool %_0_ok
OpSelectionMerge %475 None
OpBranchConditional %473 %474 %475
%474 = OpLabel
%476 = OpLoad %mat3v3float %_6_m9
%477 = OpCompositeConstruct %mat3v3float %206 %207 %208
%478 = OpCompositeExtract %v3float %476 0
%479 = OpCompositeExtract %v3float %477 0
%480 = OpFOrdEqual %v3bool %478 %479
%481 = OpAll %bool %480
%482 = OpCompositeExtract %v3float %476 1
%483 = OpCompositeExtract %v3float %477 1
%484 = OpFOrdEqual %v3bool %482 %483
%485 = OpAll %bool %484
%486 = OpLogicalAnd %bool %481 %485
%487 = OpCompositeExtract %v3float %476 2
%488 = OpCompositeExtract %v3float %477 2
%489 = OpFOrdEqual %v3bool %487 %488
%490 = OpAll %bool %489
%491 = OpLogicalAnd %bool %486 %490
OpBranch %475
%475 = OpLabel
%492 = OpPhi %bool %false %455 %491 %474
OpStore %_0_ok %492
%495 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%496 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%497 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%498 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%494 = OpCompositeConstruct %mat4v4float %495 %496 %497 %498
OpStore %_7_m10 %494
%499 = OpLoad %bool %_0_ok
OpSelectionMerge %501 None
OpBranchConditional %499 %500 %501
%500 = OpLabel
%502 = OpLoad %mat4v4float %_7_m10
%503 = OpCompositeConstruct %mat4v4float %239 %240 %241 %242
%504 = OpCompositeExtract %v4float %502 0
%505 = OpCompositeExtract %v4float %503 0
%506 = OpFOrdEqual %v4bool %504 %505
%507 = OpAll %bool %506
%508 = OpCompositeExtract %v4float %502 1
%509 = OpCompositeExtract %v4float %503 1
%510 = OpFOrdEqual %v4bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
%513 = OpCompositeExtract %v4float %502 2
%514 = OpCompositeExtract %v4float %503 2
%515 = OpFOrdEqual %v4bool %513 %514
%516 = OpAll %bool %515
%517 = OpLogicalAnd %bool %512 %516
%518 = OpCompositeExtract %v4float %502 3
%519 = OpCompositeExtract %v4float %503 3
%520 = OpFOrdEqual %v4bool %518 %519
%521 = OpAll %bool %520
%522 = OpLogicalAnd %bool %517 %521
OpBranch %501
%501 = OpLabel
%523 = OpPhi %bool %false %475 %522 %500
OpStore %_0_ok %523
%525 = OpCompositeConstruct %mat4v4float %267 %267 %267 %267
OpStore %_8_m11 %525
%526 = OpLoad %mat4v4float %_8_m11
%527 = OpLoad %mat4v4float %_7_m10
%528 = OpCompositeExtract %v4float %526 0
%529 = OpCompositeExtract %v4float %527 0
%530 = OpFSub %v4float %528 %529
%531 = OpCompositeExtract %v4float %526 1
%532 = OpCompositeExtract %v4float %527 1
%533 = OpFSub %v4float %531 %532
%534 = OpCompositeExtract %v4float %526 2
%535 = OpCompositeExtract %v4float %527 2
%536 = OpFSub %v4float %534 %535
%537 = OpCompositeExtract %v4float %526 3
%538 = OpCompositeExtract %v4float %527 3
%539 = OpFSub %v4float %537 %538
%540 = OpCompositeConstruct %mat4v4float %530 %533 %536 %539
OpStore %_8_m11 %540
%541 = OpLoad %bool %_0_ok
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpLoad %mat4v4float %_8_m11
%545 = OpCompositeConstruct %mat4v4float %288 %289 %290 %291
%546 = OpCompositeExtract %v4float %544 0
%547 = OpCompositeExtract %v4float %545 0
%548 = OpFOrdEqual %v4bool %546 %547
%549 = OpAll %bool %548
%550 = OpCompositeExtract %v4float %544 1
%551 = OpCompositeExtract %v4float %545 1
%552 = OpFOrdEqual %v4bool %550 %551
%553 = OpAll %bool %552
%554 = OpLogicalAnd %bool %549 %553
%555 = OpCompositeExtract %v4float %544 2
%556 = OpCompositeExtract %v4float %545 2
%557 = OpFOrdEqual %v4bool %555 %556
%558 = OpAll %bool %557
%559 = OpLogicalAnd %bool %554 %558
%560 = OpCompositeExtract %v4float %544 3
%561 = OpCompositeExtract %v4float %545 3
%562 = OpFOrdEqual %v4bool %560 %561
%563 = OpAll %bool %562
%564 = OpLogicalAnd %bool %559 %563
OpBranch %543
%543 = OpLabel
%565 = OpPhi %bool %false %501 %564 %542
OpStore %_0_ok %565
%566 = OpLoad %bool %_0_ok
OpSelectionMerge %568 None
OpBranchConditional %566 %567 %568
%567 = OpLabel
%569 = OpFunctionCall %bool %test_half_b
OpBranch %568
%568 = OpLabel
%570 = OpPhi %bool %false %543 %569 %567
OpSelectionMerge %572 None
OpBranchConditional %570 %571 %572
%571 = OpLabel
%573 = OpFunctionCall %bool %test_comma_b
OpBranch %572
%572 = OpLabel
%574 = OpPhi %bool %false %568 %573 %571
OpSelectionMerge %579 None
OpBranchConditional %574 %577 %578
%577 = OpLabel
%580 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%583 = OpLoad %v4float %580
OpStore %575 %583
OpBranch %579
%578 = OpLabel
%584 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%585 = OpLoad %v4float %584
OpStore %575 %585
OpBranch %579
%579 = OpLabel
%586 = OpLoad %v4float %575
OpReturnValue %586
OpFunctionEnd
