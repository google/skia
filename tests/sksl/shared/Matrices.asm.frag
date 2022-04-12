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
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
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
OpDecorate %275 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %549 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%39 = OpConstantComposite %mat2v2float %37 %38
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%75 = OpConstantComposite %v2float %float_6 %float_0
%76 = OpConstantComposite %v2float %float_0 %float_6
%81 = OpConstantComposite %mat2v2float %75 %76
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%102 = OpConstantComposite %v2float %float_6 %float_12
%103 = OpConstantComposite %v2float %float_18 %float_24
%104 = OpConstantComposite %mat2v2float %102 %103
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%128 = OpConstantComposite %v2float %float_4 %float_0
%129 = OpConstantComposite %v2float %float_0 %float_4
%130 = OpConstantComposite %mat2v2float %128 %129
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%156 = OpConstantComposite %v2float %float_5 %float_2
%157 = OpConstantComposite %v2float %float_3 %float_8
%158 = OpConstantComposite %mat2v2float %156 %157
%float_7 = OpConstant %float 7
%171 = OpConstantComposite %v2float %float_5 %float_6
%172 = OpConstantComposite %v2float %float_7 %float_8
%173 = OpConstantComposite %mat2v2float %171 %172
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%194 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%195 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%196 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%201 = OpConstantComposite %mat3v3float %194 %195 %196
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%223 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%224 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%225 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%226 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%231 = OpConstantComposite %mat4v4float %223 %224 %225 %226
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%255 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%256 = OpConstantComposite %mat4v4float %255 %255 %255 %255
%276 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%277 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%278 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%279 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%280 = OpConstantComposite %mat4v4float %276 %277 %278 %279
%316 = OpTypeFunction %v4float %_ptr_Function_v2float
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
OpStore %m1 %39
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%46 = OpCompositeExtract %v2float %44 0
%47 = OpCompositeExtract %v2float %39 0
%48 = OpFOrdEqual %v2bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v2float %44 1
%51 = OpCompositeExtract %v2float %39 1
%52 = OpFOrdEqual %v2bool %50 %51
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %49 %53
OpBranch %43
%43 = OpLabel
%55 = OpPhi %bool %false %26 %54 %42
OpStore %ok %55
%57 = OpLoad %mat2v2float %m1
OpStore %m3 %57
%58 = OpLoad %bool %ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %mat2v2float %m3
%62 = OpCompositeExtract %v2float %61 0
%63 = OpCompositeExtract %v2float %39 0
%64 = OpFOrdEqual %v2bool %62 %63
%65 = OpAll %bool %64
%66 = OpCompositeExtract %v2float %61 1
%67 = OpCompositeExtract %v2float %39 1
%68 = OpFOrdEqual %v2bool %66 %67
%69 = OpAll %bool %68
%70 = OpLogicalAnd %bool %65 %69
OpBranch %60
%60 = OpLabel
%71 = OpPhi %bool %false %43 %70 %59
OpStore %ok %71
%74 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %m4 %74
%77 = OpLoad %bool %ok
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %mat2v2float %m4
%82 = OpCompositeExtract %v2float %80 0
%83 = OpCompositeExtract %v2float %81 0
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpCompositeExtract %v2float %80 1
%87 = OpCompositeExtract %v2float %81 1
%88 = OpFOrdEqual %v2bool %86 %87
%89 = OpAll %bool %88
%90 = OpLogicalAnd %bool %85 %89
OpBranch %79
%79 = OpLabel
%91 = OpPhi %bool %false %60 %90 %78
OpStore %ok %91
%92 = OpLoad %mat2v2float %m3
%93 = OpLoad %mat2v2float %m4
%94 = OpMatrixTimesMatrix %mat2v2float %92 %93
OpStore %m3 %94
%95 = OpLoad %bool %ok
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %mat2v2float %m3
%105 = OpCompositeExtract %v2float %98 0
%106 = OpCompositeExtract %v2float %104 0
%107 = OpFOrdEqual %v2bool %105 %106
%108 = OpAll %bool %107
%109 = OpCompositeExtract %v2float %98 1
%110 = OpCompositeExtract %v2float %104 1
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
OpBranch %97
%97 = OpLabel
%114 = OpPhi %bool %false %79 %113 %96
OpStore %ok %114
%118 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 1
%122 = OpCompositeConstruct %v2float %120 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %120
%121 = OpCompositeConstruct %mat2v2float %122 %123
OpStore %m5 %121
%124 = OpLoad %bool %ok
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %mat2v2float %m5
%131 = OpCompositeExtract %v2float %127 0
%132 = OpCompositeExtract %v2float %130 0
%133 = OpFOrdEqual %v2bool %131 %132
%134 = OpAll %bool %133
%135 = OpCompositeExtract %v2float %127 1
%136 = OpCompositeExtract %v2float %130 1
%137 = OpFOrdEqual %v2bool %135 %136
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %134 %138
OpBranch %126
%126 = OpLabel
%140 = OpPhi %bool %false %97 %139 %125
OpStore %ok %140
%141 = OpLoad %mat2v2float %m1
%142 = OpLoad %mat2v2float %m5
%143 = OpCompositeExtract %v2float %141 0
%144 = OpCompositeExtract %v2float %142 0
%145 = OpFAdd %v2float %143 %144
%146 = OpCompositeExtract %v2float %141 1
%147 = OpCompositeExtract %v2float %142 1
%148 = OpFAdd %v2float %146 %147
%149 = OpCompositeConstruct %mat2v2float %145 %148
OpStore %m1 %149
%150 = OpLoad %bool %ok
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpLoad %mat2v2float %m1
%159 = OpCompositeExtract %v2float %153 0
%160 = OpCompositeExtract %v2float %158 0
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpCompositeExtract %v2float %153 1
%164 = OpCompositeExtract %v2float %158 1
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
OpBranch %152
%152 = OpLabel
%168 = OpPhi %bool %false %126 %167 %151
OpStore %ok %168
OpStore %m7 %173
%174 = OpLoad %bool %ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %mat2v2float %m7
%178 = OpCompositeExtract %v2float %177 0
%179 = OpCompositeExtract %v2float %173 0
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpCompositeExtract %v2float %177 1
%183 = OpCompositeExtract %v2float %173 1
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %181 %185
OpBranch %176
%176 = OpLabel
%187 = OpPhi %bool %false %152 %186 %175
OpStore %ok %187
%193 = OpCompositeConstruct %mat3v3float %194 %195 %196
OpStore %m9 %193
%197 = OpLoad %bool %ok
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %mat3v3float %m9
%203 = OpCompositeExtract %v3float %200 0
%204 = OpCompositeExtract %v3float %201 0
%205 = OpFOrdEqual %v3bool %203 %204
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v3float %200 1
%208 = OpCompositeExtract %v3float %201 1
%209 = OpFOrdEqual %v3bool %207 %208
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %206 %210
%212 = OpCompositeExtract %v3float %200 2
%213 = OpCompositeExtract %v3float %201 2
%214 = OpFOrdEqual %v3bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
OpBranch %199
%199 = OpLabel
%217 = OpPhi %bool %false %176 %216 %198
OpStore %ok %217
%222 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
OpStore %m10 %222
%227 = OpLoad %bool %ok
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
%230 = OpLoad %mat4v4float %m10
%233 = OpCompositeExtract %v4float %230 0
%234 = OpCompositeExtract %v4float %231 0
%235 = OpFOrdEqual %v4bool %233 %234
%236 = OpAll %bool %235
%237 = OpCompositeExtract %v4float %230 1
%238 = OpCompositeExtract %v4float %231 1
%239 = OpFOrdEqual %v4bool %237 %238
%240 = OpAll %bool %239
%241 = OpLogicalAnd %bool %236 %240
%242 = OpCompositeExtract %v4float %230 2
%243 = OpCompositeExtract %v4float %231 2
%244 = OpFOrdEqual %v4bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
%247 = OpCompositeExtract %v4float %230 3
%248 = OpCompositeExtract %v4float %231 3
%249 = OpFOrdEqual %v4bool %247 %248
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %246 %250
OpBranch %229
%229 = OpLabel
%252 = OpPhi %bool %false %199 %251 %228
OpStore %ok %252
OpStore %m11 %256
%257 = OpLoad %mat4v4float %m11
%258 = OpLoad %mat4v4float %m10
%259 = OpCompositeExtract %v4float %257 0
%260 = OpCompositeExtract %v4float %258 0
%261 = OpFSub %v4float %259 %260
%262 = OpCompositeExtract %v4float %257 1
%263 = OpCompositeExtract %v4float %258 1
%264 = OpFSub %v4float %262 %263
%265 = OpCompositeExtract %v4float %257 2
%266 = OpCompositeExtract %v4float %258 2
%267 = OpFSub %v4float %265 %266
%268 = OpCompositeExtract %v4float %257 3
%269 = OpCompositeExtract %v4float %258 3
%270 = OpFSub %v4float %268 %269
%271 = OpCompositeConstruct %mat4v4float %261 %264 %267 %270
OpStore %m11 %271
%272 = OpLoad %bool %ok
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%275 = OpLoad %mat4v4float %m11
%281 = OpCompositeExtract %v4float %275 0
%282 = OpCompositeExtract %v4float %280 0
%283 = OpFOrdEqual %v4bool %281 %282
%284 = OpAll %bool %283
%285 = OpCompositeExtract %v4float %275 1
%286 = OpCompositeExtract %v4float %280 1
%287 = OpFOrdEqual %v4bool %285 %286
%288 = OpAll %bool %287
%289 = OpLogicalAnd %bool %284 %288
%290 = OpCompositeExtract %v4float %275 2
%291 = OpCompositeExtract %v4float %280 2
%292 = OpFOrdEqual %v4bool %290 %291
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %289 %293
%295 = OpCompositeExtract %v4float %275 3
%296 = OpCompositeExtract %v4float %280 3
%297 = OpFOrdEqual %v4bool %295 %296
%298 = OpAll %bool %297
%299 = OpLogicalAnd %bool %294 %298
OpBranch %274
%274 = OpLabel
%300 = OpPhi %bool %false %229 %299 %273
OpStore %ok %300
%301 = OpLoad %bool %ok
OpReturnValue %301
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%302 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%305 = OpLoad %mat2v2float %x
%306 = OpLoad %mat2v2float %y
%307 = OpCompositeExtract %v2float %305 0
%308 = OpCompositeExtract %v2float %306 0
%309 = OpFOrdEqual %v2bool %307 %308
%310 = OpAll %bool %309
%311 = OpCompositeExtract %v2float %305 1
%312 = OpCompositeExtract %v2float %306 1
%313 = OpFOrdEqual %v2bool %311 %312
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %310 %314
OpReturnValue %315
OpFunctionEnd
%main = OpFunction %v4float None %316
%317 = OpFunctionParameter %_ptr_Function_v2float
%318 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%539 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
%321 = OpLoad %bool %_0_ok
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %mat2v2float %_1_m1
%325 = OpCompositeExtract %v2float %324 0
%326 = OpCompositeExtract %v2float %39 0
%327 = OpFOrdEqual %v2bool %325 %326
%328 = OpAll %bool %327
%329 = OpCompositeExtract %v2float %324 1
%330 = OpCompositeExtract %v2float %39 1
%331 = OpFOrdEqual %v2bool %329 %330
%332 = OpAll %bool %331
%333 = OpLogicalAnd %bool %328 %332
OpBranch %323
%323 = OpLabel
%334 = OpPhi %bool %false %318 %333 %322
OpStore %_0_ok %334
%336 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %336
%337 = OpLoad %bool %_0_ok
OpSelectionMerge %339 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%340 = OpLoad %mat2v2float %_2_m3
%341 = OpCompositeExtract %v2float %340 0
%342 = OpCompositeExtract %v2float %39 0
%343 = OpFOrdEqual %v2bool %341 %342
%344 = OpAll %bool %343
%345 = OpCompositeExtract %v2float %340 1
%346 = OpCompositeExtract %v2float %39 1
%347 = OpFOrdEqual %v2bool %345 %346
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %344 %348
OpBranch %339
%339 = OpLabel
%350 = OpPhi %bool %false %323 %349 %338
OpStore %_0_ok %350
%352 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %_3_m4 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v2float %_3_m4
%357 = OpCompositeExtract %v2float %356 0
%358 = OpCompositeExtract %v2float %81 0
%359 = OpFOrdEqual %v2bool %357 %358
%360 = OpAll %bool %359
%361 = OpCompositeExtract %v2float %356 1
%362 = OpCompositeExtract %v2float %81 1
%363 = OpFOrdEqual %v2bool %361 %362
%364 = OpAll %bool %363
%365 = OpLogicalAnd %bool %360 %364
OpBranch %355
%355 = OpLabel
%366 = OpPhi %bool %false %339 %365 %354
OpStore %_0_ok %366
%367 = OpLoad %mat2v2float %_2_m3
%368 = OpLoad %mat2v2float %_3_m4
%369 = OpMatrixTimesMatrix %mat2v2float %367 %368
OpStore %_2_m3 %369
%370 = OpLoad %bool %_0_ok
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %mat2v2float %_2_m3
%374 = OpCompositeExtract %v2float %373 0
%375 = OpCompositeExtract %v2float %104 0
%376 = OpFOrdEqual %v2bool %374 %375
%377 = OpAll %bool %376
%378 = OpCompositeExtract %v2float %373 1
%379 = OpCompositeExtract %v2float %104 1
%380 = OpFOrdEqual %v2bool %378 %379
%381 = OpAll %bool %380
%382 = OpLogicalAnd %bool %377 %381
OpBranch %372
%372 = OpLabel
%383 = OpPhi %bool %false %355 %382 %371
OpStore %_0_ok %383
%385 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%386 = OpLoad %v2float %385
%387 = OpCompositeExtract %float %386 1
%389 = OpCompositeConstruct %v2float %387 %float_0
%390 = OpCompositeConstruct %v2float %float_0 %387
%388 = OpCompositeConstruct %mat2v2float %389 %390
OpStore %_4_m5 %388
%391 = OpLoad %bool %_0_ok
OpSelectionMerge %393 None
OpBranchConditional %391 %392 %393
%392 = OpLabel
%394 = OpLoad %mat2v2float %_4_m5
%395 = OpCompositeExtract %v2float %394 0
%396 = OpCompositeExtract %v2float %130 0
%397 = OpFOrdEqual %v2bool %395 %396
%398 = OpAll %bool %397
%399 = OpCompositeExtract %v2float %394 1
%400 = OpCompositeExtract %v2float %130 1
%401 = OpFOrdEqual %v2bool %399 %400
%402 = OpAll %bool %401
%403 = OpLogicalAnd %bool %398 %402
OpBranch %393
%393 = OpLabel
%404 = OpPhi %bool %false %372 %403 %392
OpStore %_0_ok %404
%405 = OpLoad %mat2v2float %_1_m1
%406 = OpLoad %mat2v2float %_4_m5
%407 = OpCompositeExtract %v2float %405 0
%408 = OpCompositeExtract %v2float %406 0
%409 = OpFAdd %v2float %407 %408
%410 = OpCompositeExtract %v2float %405 1
%411 = OpCompositeExtract %v2float %406 1
%412 = OpFAdd %v2float %410 %411
%413 = OpCompositeConstruct %mat2v2float %409 %412
OpStore %_1_m1 %413
%414 = OpLoad %bool %_0_ok
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%417 = OpLoad %mat2v2float %_1_m1
%418 = OpCompositeExtract %v2float %417 0
%419 = OpCompositeExtract %v2float %158 0
%420 = OpFOrdEqual %v2bool %418 %419
%421 = OpAll %bool %420
%422 = OpCompositeExtract %v2float %417 1
%423 = OpCompositeExtract %v2float %158 1
%424 = OpFOrdEqual %v2bool %422 %423
%425 = OpAll %bool %424
%426 = OpLogicalAnd %bool %421 %425
OpBranch %416
%416 = OpLabel
%427 = OpPhi %bool %false %393 %426 %415
OpStore %_0_ok %427
OpStore %_5_m7 %173
%429 = OpLoad %bool %_0_ok
OpSelectionMerge %431 None
OpBranchConditional %429 %430 %431
%430 = OpLabel
%432 = OpLoad %mat2v2float %_5_m7
%433 = OpCompositeExtract %v2float %432 0
%434 = OpCompositeExtract %v2float %173 0
%435 = OpFOrdEqual %v2bool %433 %434
%436 = OpAll %bool %435
%437 = OpCompositeExtract %v2float %432 1
%438 = OpCompositeExtract %v2float %173 1
%439 = OpFOrdEqual %v2bool %437 %438
%440 = OpAll %bool %439
%441 = OpLogicalAnd %bool %436 %440
OpBranch %431
%431 = OpLabel
%442 = OpPhi %bool %false %416 %441 %430
OpStore %_0_ok %442
%444 = OpCompositeConstruct %mat3v3float %194 %195 %196
OpStore %_6_m9 %444
%445 = OpLoad %bool %_0_ok
OpSelectionMerge %447 None
OpBranchConditional %445 %446 %447
%446 = OpLabel
%448 = OpLoad %mat3v3float %_6_m9
%449 = OpCompositeExtract %v3float %448 0
%450 = OpCompositeExtract %v3float %201 0
%451 = OpFOrdEqual %v3bool %449 %450
%452 = OpAll %bool %451
%453 = OpCompositeExtract %v3float %448 1
%454 = OpCompositeExtract %v3float %201 1
%455 = OpFOrdEqual %v3bool %453 %454
%456 = OpAll %bool %455
%457 = OpLogicalAnd %bool %452 %456
%458 = OpCompositeExtract %v3float %448 2
%459 = OpCompositeExtract %v3float %201 2
%460 = OpFOrdEqual %v3bool %458 %459
%461 = OpAll %bool %460
%462 = OpLogicalAnd %bool %457 %461
OpBranch %447
%447 = OpLabel
%463 = OpPhi %bool %false %431 %462 %446
OpStore %_0_ok %463
%465 = OpCompositeConstruct %mat4v4float %223 %224 %225 %226
OpStore %_7_m10 %465
%466 = OpLoad %bool %_0_ok
OpSelectionMerge %468 None
OpBranchConditional %466 %467 %468
%467 = OpLabel
%469 = OpLoad %mat4v4float %_7_m10
%470 = OpCompositeExtract %v4float %469 0
%471 = OpCompositeExtract %v4float %231 0
%472 = OpFOrdEqual %v4bool %470 %471
%473 = OpAll %bool %472
%474 = OpCompositeExtract %v4float %469 1
%475 = OpCompositeExtract %v4float %231 1
%476 = OpFOrdEqual %v4bool %474 %475
%477 = OpAll %bool %476
%478 = OpLogicalAnd %bool %473 %477
%479 = OpCompositeExtract %v4float %469 2
%480 = OpCompositeExtract %v4float %231 2
%481 = OpFOrdEqual %v4bool %479 %480
%482 = OpAll %bool %481
%483 = OpLogicalAnd %bool %478 %482
%484 = OpCompositeExtract %v4float %469 3
%485 = OpCompositeExtract %v4float %231 3
%486 = OpFOrdEqual %v4bool %484 %485
%487 = OpAll %bool %486
%488 = OpLogicalAnd %bool %483 %487
OpBranch %468
%468 = OpLabel
%489 = OpPhi %bool %false %447 %488 %467
OpStore %_0_ok %489
OpStore %_8_m11 %256
%491 = OpLoad %mat4v4float %_8_m11
%492 = OpLoad %mat4v4float %_7_m10
%493 = OpCompositeExtract %v4float %491 0
%494 = OpCompositeExtract %v4float %492 0
%495 = OpFSub %v4float %493 %494
%496 = OpCompositeExtract %v4float %491 1
%497 = OpCompositeExtract %v4float %492 1
%498 = OpFSub %v4float %496 %497
%499 = OpCompositeExtract %v4float %491 2
%500 = OpCompositeExtract %v4float %492 2
%501 = OpFSub %v4float %499 %500
%502 = OpCompositeExtract %v4float %491 3
%503 = OpCompositeExtract %v4float %492 3
%504 = OpFSub %v4float %502 %503
%505 = OpCompositeConstruct %mat4v4float %495 %498 %501 %504
OpStore %_8_m11 %505
%506 = OpLoad %bool %_0_ok
OpSelectionMerge %508 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%509 = OpLoad %mat4v4float %_8_m11
%510 = OpCompositeExtract %v4float %509 0
%511 = OpCompositeExtract %v4float %280 0
%512 = OpFOrdEqual %v4bool %510 %511
%513 = OpAll %bool %512
%514 = OpCompositeExtract %v4float %509 1
%515 = OpCompositeExtract %v4float %280 1
%516 = OpFOrdEqual %v4bool %514 %515
%517 = OpAll %bool %516
%518 = OpLogicalAnd %bool %513 %517
%519 = OpCompositeExtract %v4float %509 2
%520 = OpCompositeExtract %v4float %280 2
%521 = OpFOrdEqual %v4bool %519 %520
%522 = OpAll %bool %521
%523 = OpLogicalAnd %bool %518 %522
%524 = OpCompositeExtract %v4float %509 3
%525 = OpCompositeExtract %v4float %280 3
%526 = OpFOrdEqual %v4bool %524 %525
%527 = OpAll %bool %526
%528 = OpLogicalAnd %bool %523 %527
OpBranch %508
%508 = OpLabel
%529 = OpPhi %bool %false %468 %528 %507
OpStore %_0_ok %529
%530 = OpLoad %bool %_0_ok
OpSelectionMerge %532 None
OpBranchConditional %530 %531 %532
%531 = OpLabel
%533 = OpFunctionCall %bool %test_half_b
OpBranch %532
%532 = OpLabel
%534 = OpPhi %bool %false %508 %533 %531
OpSelectionMerge %536 None
OpBranchConditional %534 %535 %536
%535 = OpLabel
%537 = OpFunctionCall %bool %test_comma_b
OpBranch %536
%536 = OpLabel
%538 = OpPhi %bool %false %532 %537 %535
OpSelectionMerge %543 None
OpBranchConditional %538 %541 %542
%541 = OpLabel
%544 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%547 = OpLoad %v4float %544
OpStore %539 %547
OpBranch %543
%542 = OpLabel
%548 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%549 = OpLoad %v4float %548
OpStore %539 %549
OpBranch %543
%543 = OpLabel
%550 = OpLoad %v4float %539
OpReturnValue %550
OpFunctionEnd
