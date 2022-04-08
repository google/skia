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
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %548 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
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
%77 = OpConstantComposite %v2float %float_6 %float_0
%78 = OpConstantComposite %v2float %float_0 %float_6
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%104 = OpConstantComposite %v2float %float_6 %float_12
%105 = OpConstantComposite %v2float %float_18 %float_24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%130 = OpConstantComposite %v2float %float_4 %float_0
%131 = OpConstantComposite %v2float %float_0 %float_4
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%158 = OpConstantComposite %v2float %float_5 %float_2
%159 = OpConstantComposite %v2float %float_3 %float_8
%float_7 = OpConstant %float 7
%173 = OpConstantComposite %v2float %float_5 %float_6
%174 = OpConstantComposite %v2float %float_7 %float_8
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%197 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%198 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%199 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%226 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%227 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%228 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%229 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%258 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%279 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%280 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%281 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%282 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%321 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%76 = OpCompositeConstruct %mat2v2float %77 %78
OpStore %m4 %76
%79 = OpLoad %bool %ok
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %mat2v2float %m4
%83 = OpCompositeConstruct %mat2v2float %77 %78
%84 = OpCompositeExtract %v2float %82 0
%85 = OpCompositeExtract %v2float %83 0
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
%88 = OpCompositeExtract %v2float %82 1
%89 = OpCompositeExtract %v2float %83 1
%90 = OpFOrdEqual %v2bool %88 %89
%91 = OpAll %bool %90
%92 = OpLogicalAnd %bool %87 %91
OpBranch %81
%81 = OpLabel
%93 = OpPhi %bool %false %61 %92 %80
OpStore %ok %93
%94 = OpLoad %mat2v2float %m3
%95 = OpLoad %mat2v2float %m4
%96 = OpMatrixTimesMatrix %mat2v2float %94 %95
OpStore %m3 %96
%97 = OpLoad %bool %ok
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %mat2v2float %m3
%106 = OpCompositeConstruct %mat2v2float %104 %105
%107 = OpCompositeExtract %v2float %100 0
%108 = OpCompositeExtract %v2float %106 0
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpCompositeExtract %v2float %100 1
%112 = OpCompositeExtract %v2float %106 1
%113 = OpFOrdEqual %v2bool %111 %112
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %110 %114
OpBranch %99
%99 = OpLabel
%116 = OpPhi %bool %false %81 %115 %98
OpStore %ok %116
%120 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%121 = OpLoad %v2float %120
%122 = OpCompositeExtract %float %121 1
%124 = OpCompositeConstruct %v2float %122 %float_0
%125 = OpCompositeConstruct %v2float %float_0 %122
%123 = OpCompositeConstruct %mat2v2float %124 %125
OpStore %m5 %123
%126 = OpLoad %bool %ok
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpLoad %mat2v2float %m5
%132 = OpCompositeConstruct %mat2v2float %130 %131
%133 = OpCompositeExtract %v2float %129 0
%134 = OpCompositeExtract %v2float %132 0
%135 = OpFOrdEqual %v2bool %133 %134
%136 = OpAll %bool %135
%137 = OpCompositeExtract %v2float %129 1
%138 = OpCompositeExtract %v2float %132 1
%139 = OpFOrdEqual %v2bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
OpBranch %128
%128 = OpLabel
%142 = OpPhi %bool %false %99 %141 %127
OpStore %ok %142
%143 = OpLoad %mat2v2float %m1
%144 = OpLoad %mat2v2float %m5
%145 = OpCompositeExtract %v2float %143 0
%146 = OpCompositeExtract %v2float %144 0
%147 = OpFAdd %v2float %145 %146
%148 = OpCompositeExtract %v2float %143 1
%149 = OpCompositeExtract %v2float %144 1
%150 = OpFAdd %v2float %148 %149
%151 = OpCompositeConstruct %mat2v2float %147 %150
OpStore %m1 %151
%152 = OpLoad %bool %ok
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %mat2v2float %m1
%160 = OpCompositeConstruct %mat2v2float %158 %159
%161 = OpCompositeExtract %v2float %155 0
%162 = OpCompositeExtract %v2float %160 0
%163 = OpFOrdEqual %v2bool %161 %162
%164 = OpAll %bool %163
%165 = OpCompositeExtract %v2float %155 1
%166 = OpCompositeExtract %v2float %160 1
%167 = OpFOrdEqual %v2bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
OpBranch %154
%154 = OpLabel
%170 = OpPhi %bool %false %128 %169 %153
OpStore %ok %170
%175 = OpCompositeConstruct %mat2v2float %173 %174
OpStore %m7 %175
%176 = OpLoad %bool %ok
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %mat2v2float %m7
%180 = OpCompositeConstruct %mat2v2float %173 %174
%181 = OpCompositeExtract %v2float %179 0
%182 = OpCompositeExtract %v2float %180 0
%183 = OpFOrdEqual %v2bool %181 %182
%184 = OpAll %bool %183
%185 = OpCompositeExtract %v2float %179 1
%186 = OpCompositeExtract %v2float %180 1
%187 = OpFOrdEqual %v2bool %185 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %184 %188
OpBranch %178
%178 = OpLabel
%190 = OpPhi %bool %false %154 %189 %177
OpStore %ok %190
%196 = OpCompositeConstruct %mat3v3float %197 %198 %199
OpStore %m9 %196
%200 = OpLoad %bool %ok
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpLoad %mat3v3float %m9
%204 = OpCompositeConstruct %mat3v3float %197 %198 %199
%206 = OpCompositeExtract %v3float %203 0
%207 = OpCompositeExtract %v3float %204 0
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpCompositeExtract %v3float %203 1
%211 = OpCompositeExtract %v3float %204 1
%212 = OpFOrdEqual %v3bool %210 %211
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %209 %213
%215 = OpCompositeExtract %v3float %203 2
%216 = OpCompositeExtract %v3float %204 2
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
OpBranch %202
%202 = OpLabel
%220 = OpPhi %bool %false %178 %219 %201
OpStore %ok %220
%225 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
OpStore %m10 %225
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat4v4float %m10
%234 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
%236 = OpCompositeExtract %v4float %233 0
%237 = OpCompositeExtract %v4float %234 0
%238 = OpFOrdEqual %v4bool %236 %237
%239 = OpAll %bool %238
%240 = OpCompositeExtract %v4float %233 1
%241 = OpCompositeExtract %v4float %234 1
%242 = OpFOrdEqual %v4bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
%245 = OpCompositeExtract %v4float %233 2
%246 = OpCompositeExtract %v4float %234 2
%247 = OpFOrdEqual %v4bool %245 %246
%248 = OpAll %bool %247
%249 = OpLogicalAnd %bool %244 %248
%250 = OpCompositeExtract %v4float %233 3
%251 = OpCompositeExtract %v4float %234 3
%252 = OpFOrdEqual %v4bool %250 %251
%253 = OpAll %bool %252
%254 = OpLogicalAnd %bool %249 %253
OpBranch %232
%232 = OpLabel
%255 = OpPhi %bool %false %202 %254 %231
OpStore %ok %255
%259 = OpCompositeConstruct %mat4v4float %258 %258 %258 %258
OpStore %m11 %259
%260 = OpLoad %mat4v4float %m11
%261 = OpLoad %mat4v4float %m10
%262 = OpCompositeExtract %v4float %260 0
%263 = OpCompositeExtract %v4float %261 0
%264 = OpFSub %v4float %262 %263
%265 = OpCompositeExtract %v4float %260 1
%266 = OpCompositeExtract %v4float %261 1
%267 = OpFSub %v4float %265 %266
%268 = OpCompositeExtract %v4float %260 2
%269 = OpCompositeExtract %v4float %261 2
%270 = OpFSub %v4float %268 %269
%271 = OpCompositeExtract %v4float %260 3
%272 = OpCompositeExtract %v4float %261 3
%273 = OpFSub %v4float %271 %272
%274 = OpCompositeConstruct %mat4v4float %264 %267 %270 %273
OpStore %m11 %274
%275 = OpLoad %bool %ok
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
%278 = OpLoad %mat4v4float %m11
%283 = OpCompositeConstruct %mat4v4float %279 %280 %281 %282
%284 = OpCompositeExtract %v4float %278 0
%285 = OpCompositeExtract %v4float %283 0
%286 = OpFOrdEqual %v4bool %284 %285
%287 = OpAll %bool %286
%288 = OpCompositeExtract %v4float %278 1
%289 = OpCompositeExtract %v4float %283 1
%290 = OpFOrdEqual %v4bool %288 %289
%291 = OpAll %bool %290
%292 = OpLogicalAnd %bool %287 %291
%293 = OpCompositeExtract %v4float %278 2
%294 = OpCompositeExtract %v4float %283 2
%295 = OpFOrdEqual %v4bool %293 %294
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %292 %296
%298 = OpCompositeExtract %v4float %278 3
%299 = OpCompositeExtract %v4float %283 3
%300 = OpFOrdEqual %v4bool %298 %299
%301 = OpAll %bool %300
%302 = OpLogicalAnd %bool %297 %301
OpBranch %277
%277 = OpLabel
%303 = OpPhi %bool %false %232 %302 %276
OpStore %ok %303
%304 = OpLoad %bool %ok
OpReturnValue %304
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%305 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
%308 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %x %308
%309 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %y %309
%310 = OpLoad %mat2v2float %x
%311 = OpLoad %mat2v2float %y
%312 = OpCompositeExtract %v2float %310 0
%313 = OpCompositeExtract %v2float %311 0
%314 = OpFOrdEqual %v2bool %312 %313
%315 = OpAll %bool %314
%316 = OpCompositeExtract %v2float %310 1
%317 = OpCompositeExtract %v2float %311 1
%318 = OpFOrdEqual %v2bool %316 %317
%319 = OpAll %bool %318
%320 = OpLogicalAnd %bool %315 %319
OpReturnValue %320
OpFunctionEnd
%main = OpFunction %v4float None %321
%322 = OpFunctionParameter %_ptr_Function_v2float
%323 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%557 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%326 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %_1_m1 %326
%327 = OpLoad %bool %_0_ok
OpSelectionMerge %329 None
OpBranchConditional %327 %328 %329
%328 = OpLabel
%330 = OpLoad %mat2v2float %_1_m1
%331 = OpCompositeConstruct %mat2v2float %37 %38
%332 = OpCompositeExtract %v2float %330 0
%333 = OpCompositeExtract %v2float %331 0
%334 = OpFOrdEqual %v2bool %332 %333
%335 = OpAll %bool %334
%336 = OpCompositeExtract %v2float %330 1
%337 = OpCompositeExtract %v2float %331 1
%338 = OpFOrdEqual %v2bool %336 %337
%339 = OpAll %bool %338
%340 = OpLogicalAnd %bool %335 %339
OpBranch %329
%329 = OpLabel
%341 = OpPhi %bool %false %323 %340 %328
OpStore %_0_ok %341
%343 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %343
%344 = OpLoad %bool %_0_ok
OpSelectionMerge %346 None
OpBranchConditional %344 %345 %346
%345 = OpLabel
%347 = OpLoad %mat2v2float %_2_m3
%348 = OpCompositeConstruct %mat2v2float %37 %38
%349 = OpCompositeExtract %v2float %347 0
%350 = OpCompositeExtract %v2float %348 0
%351 = OpFOrdEqual %v2bool %349 %350
%352 = OpAll %bool %351
%353 = OpCompositeExtract %v2float %347 1
%354 = OpCompositeExtract %v2float %348 1
%355 = OpFOrdEqual %v2bool %353 %354
%356 = OpAll %bool %355
%357 = OpLogicalAnd %bool %352 %356
OpBranch %346
%346 = OpLabel
%358 = OpPhi %bool %false %329 %357 %345
OpStore %_0_ok %358
%360 = OpCompositeConstruct %mat2v2float %77 %78
OpStore %_3_m4 %360
%361 = OpLoad %bool %_0_ok
OpSelectionMerge %363 None
OpBranchConditional %361 %362 %363
%362 = OpLabel
%364 = OpLoad %mat2v2float %_3_m4
%365 = OpCompositeConstruct %mat2v2float %77 %78
%366 = OpCompositeExtract %v2float %364 0
%367 = OpCompositeExtract %v2float %365 0
%368 = OpFOrdEqual %v2bool %366 %367
%369 = OpAll %bool %368
%370 = OpCompositeExtract %v2float %364 1
%371 = OpCompositeExtract %v2float %365 1
%372 = OpFOrdEqual %v2bool %370 %371
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %369 %373
OpBranch %363
%363 = OpLabel
%375 = OpPhi %bool %false %346 %374 %362
OpStore %_0_ok %375
%376 = OpLoad %mat2v2float %_2_m3
%377 = OpLoad %mat2v2float %_3_m4
%378 = OpMatrixTimesMatrix %mat2v2float %376 %377
OpStore %_2_m3 %378
%379 = OpLoad %bool %_0_ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %mat2v2float %_2_m3
%383 = OpCompositeConstruct %mat2v2float %104 %105
%384 = OpCompositeExtract %v2float %382 0
%385 = OpCompositeExtract %v2float %383 0
%386 = OpFOrdEqual %v2bool %384 %385
%387 = OpAll %bool %386
%388 = OpCompositeExtract %v2float %382 1
%389 = OpCompositeExtract %v2float %383 1
%390 = OpFOrdEqual %v2bool %388 %389
%391 = OpAll %bool %390
%392 = OpLogicalAnd %bool %387 %391
OpBranch %381
%381 = OpLabel
%393 = OpPhi %bool %false %363 %392 %380
OpStore %_0_ok %393
%395 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%396 = OpLoad %v2float %395
%397 = OpCompositeExtract %float %396 1
%399 = OpCompositeConstruct %v2float %397 %float_0
%400 = OpCompositeConstruct %v2float %float_0 %397
%398 = OpCompositeConstruct %mat2v2float %399 %400
OpStore %_4_m5 %398
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpLoad %mat2v2float %_4_m5
%405 = OpCompositeConstruct %mat2v2float %130 %131
%406 = OpCompositeExtract %v2float %404 0
%407 = OpCompositeExtract %v2float %405 0
%408 = OpFOrdEqual %v2bool %406 %407
%409 = OpAll %bool %408
%410 = OpCompositeExtract %v2float %404 1
%411 = OpCompositeExtract %v2float %405 1
%412 = OpFOrdEqual %v2bool %410 %411
%413 = OpAll %bool %412
%414 = OpLogicalAnd %bool %409 %413
OpBranch %403
%403 = OpLabel
%415 = OpPhi %bool %false %381 %414 %402
OpStore %_0_ok %415
%416 = OpLoad %mat2v2float %_1_m1
%417 = OpLoad %mat2v2float %_4_m5
%418 = OpCompositeExtract %v2float %416 0
%419 = OpCompositeExtract %v2float %417 0
%420 = OpFAdd %v2float %418 %419
%421 = OpCompositeExtract %v2float %416 1
%422 = OpCompositeExtract %v2float %417 1
%423 = OpFAdd %v2float %421 %422
%424 = OpCompositeConstruct %mat2v2float %420 %423
OpStore %_1_m1 %424
%425 = OpLoad %bool %_0_ok
OpSelectionMerge %427 None
OpBranchConditional %425 %426 %427
%426 = OpLabel
%428 = OpLoad %mat2v2float %_1_m1
%429 = OpCompositeConstruct %mat2v2float %158 %159
%430 = OpCompositeExtract %v2float %428 0
%431 = OpCompositeExtract %v2float %429 0
%432 = OpFOrdEqual %v2bool %430 %431
%433 = OpAll %bool %432
%434 = OpCompositeExtract %v2float %428 1
%435 = OpCompositeExtract %v2float %429 1
%436 = OpFOrdEqual %v2bool %434 %435
%437 = OpAll %bool %436
%438 = OpLogicalAnd %bool %433 %437
OpBranch %427
%427 = OpLabel
%439 = OpPhi %bool %false %403 %438 %426
OpStore %_0_ok %439
%441 = OpCompositeConstruct %mat2v2float %173 %174
OpStore %_5_m7 %441
%442 = OpLoad %bool %_0_ok
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpLoad %mat2v2float %_5_m7
%446 = OpCompositeConstruct %mat2v2float %173 %174
%447 = OpCompositeExtract %v2float %445 0
%448 = OpCompositeExtract %v2float %446 0
%449 = OpFOrdEqual %v2bool %447 %448
%450 = OpAll %bool %449
%451 = OpCompositeExtract %v2float %445 1
%452 = OpCompositeExtract %v2float %446 1
%453 = OpFOrdEqual %v2bool %451 %452
%454 = OpAll %bool %453
%455 = OpLogicalAnd %bool %450 %454
OpBranch %444
%444 = OpLabel
%456 = OpPhi %bool %false %427 %455 %443
OpStore %_0_ok %456
%458 = OpCompositeConstruct %mat3v3float %197 %198 %199
OpStore %_6_m9 %458
%459 = OpLoad %bool %_0_ok
OpSelectionMerge %461 None
OpBranchConditional %459 %460 %461
%460 = OpLabel
%462 = OpLoad %mat3v3float %_6_m9
%463 = OpCompositeConstruct %mat3v3float %197 %198 %199
%464 = OpCompositeExtract %v3float %462 0
%465 = OpCompositeExtract %v3float %463 0
%466 = OpFOrdEqual %v3bool %464 %465
%467 = OpAll %bool %466
%468 = OpCompositeExtract %v3float %462 1
%469 = OpCompositeExtract %v3float %463 1
%470 = OpFOrdEqual %v3bool %468 %469
%471 = OpAll %bool %470
%472 = OpLogicalAnd %bool %467 %471
%473 = OpCompositeExtract %v3float %462 2
%474 = OpCompositeExtract %v3float %463 2
%475 = OpFOrdEqual %v3bool %473 %474
%476 = OpAll %bool %475
%477 = OpLogicalAnd %bool %472 %476
OpBranch %461
%461 = OpLabel
%478 = OpPhi %bool %false %444 %477 %460
OpStore %_0_ok %478
%480 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
OpStore %_7_m10 %480
%481 = OpLoad %bool %_0_ok
OpSelectionMerge %483 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%484 = OpLoad %mat4v4float %_7_m10
%485 = OpCompositeConstruct %mat4v4float %226 %227 %228 %229
%486 = OpCompositeExtract %v4float %484 0
%487 = OpCompositeExtract %v4float %485 0
%488 = OpFOrdEqual %v4bool %486 %487
%489 = OpAll %bool %488
%490 = OpCompositeExtract %v4float %484 1
%491 = OpCompositeExtract %v4float %485 1
%492 = OpFOrdEqual %v4bool %490 %491
%493 = OpAll %bool %492
%494 = OpLogicalAnd %bool %489 %493
%495 = OpCompositeExtract %v4float %484 2
%496 = OpCompositeExtract %v4float %485 2
%497 = OpFOrdEqual %v4bool %495 %496
%498 = OpAll %bool %497
%499 = OpLogicalAnd %bool %494 %498
%500 = OpCompositeExtract %v4float %484 3
%501 = OpCompositeExtract %v4float %485 3
%502 = OpFOrdEqual %v4bool %500 %501
%503 = OpAll %bool %502
%504 = OpLogicalAnd %bool %499 %503
OpBranch %483
%483 = OpLabel
%505 = OpPhi %bool %false %461 %504 %482
OpStore %_0_ok %505
%507 = OpCompositeConstruct %mat4v4float %258 %258 %258 %258
OpStore %_8_m11 %507
%508 = OpLoad %mat4v4float %_8_m11
%509 = OpLoad %mat4v4float %_7_m10
%510 = OpCompositeExtract %v4float %508 0
%511 = OpCompositeExtract %v4float %509 0
%512 = OpFSub %v4float %510 %511
%513 = OpCompositeExtract %v4float %508 1
%514 = OpCompositeExtract %v4float %509 1
%515 = OpFSub %v4float %513 %514
%516 = OpCompositeExtract %v4float %508 2
%517 = OpCompositeExtract %v4float %509 2
%518 = OpFSub %v4float %516 %517
%519 = OpCompositeExtract %v4float %508 3
%520 = OpCompositeExtract %v4float %509 3
%521 = OpFSub %v4float %519 %520
%522 = OpCompositeConstruct %mat4v4float %512 %515 %518 %521
OpStore %_8_m11 %522
%523 = OpLoad %bool %_0_ok
OpSelectionMerge %525 None
OpBranchConditional %523 %524 %525
%524 = OpLabel
%526 = OpLoad %mat4v4float %_8_m11
%527 = OpCompositeConstruct %mat4v4float %279 %280 %281 %282
%528 = OpCompositeExtract %v4float %526 0
%529 = OpCompositeExtract %v4float %527 0
%530 = OpFOrdEqual %v4bool %528 %529
%531 = OpAll %bool %530
%532 = OpCompositeExtract %v4float %526 1
%533 = OpCompositeExtract %v4float %527 1
%534 = OpFOrdEqual %v4bool %532 %533
%535 = OpAll %bool %534
%536 = OpLogicalAnd %bool %531 %535
%537 = OpCompositeExtract %v4float %526 2
%538 = OpCompositeExtract %v4float %527 2
%539 = OpFOrdEqual %v4bool %537 %538
%540 = OpAll %bool %539
%541 = OpLogicalAnd %bool %536 %540
%542 = OpCompositeExtract %v4float %526 3
%543 = OpCompositeExtract %v4float %527 3
%544 = OpFOrdEqual %v4bool %542 %543
%545 = OpAll %bool %544
%546 = OpLogicalAnd %bool %541 %545
OpBranch %525
%525 = OpLabel
%547 = OpPhi %bool %false %483 %546 %524
OpStore %_0_ok %547
%548 = OpLoad %bool %_0_ok
OpSelectionMerge %550 None
OpBranchConditional %548 %549 %550
%549 = OpLabel
%551 = OpFunctionCall %bool %test_half_b
OpBranch %550
%550 = OpLabel
%552 = OpPhi %bool %false %525 %551 %549
OpSelectionMerge %554 None
OpBranchConditional %552 %553 %554
%553 = OpLabel
%555 = OpFunctionCall %bool %test_comma_b
OpBranch %554
%554 = OpLabel
%556 = OpPhi %bool %false %550 %555 %553
OpSelectionMerge %561 None
OpBranchConditional %556 %559 %560
%559 = OpLabel
%562 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%565 = OpLoad %v4float %562
OpStore %557 %565
OpBranch %561
%560 = OpLabel
%566 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%567 = OpLoad %v4float %566
OpStore %557 %567
OpBranch %561
%561 = OpLabel
%568 = OpLoad %v4float %557
OpReturnValue %568
OpFunctionEnd
