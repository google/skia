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
OpName %v1 "v1"
OpName %v2 "v2"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_v1 "_1_v1"
OpName %_2_v2 "_2_v2"
OpName %_3_m1 "_3_m1"
OpName %_4_m2 "_4_m2"
OpName %_5_m3 "_5_m3"
OpName %_6_m4 "_6_m4"
OpName %_7_m5 "_7_m5"
OpName %_8_m7 "_8_m7"
OpName %_9_m9 "_9_m9"
OpName %_10_m10 "_10_m10"
OpName %_11_m11 "_11_m11"
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
OpDecorate %v1 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %v2 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %m1 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %m2 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %669 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %713 RelaxedPrecision
OpDecorate %714 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_2 = OpConstant %float 2
%mat3v3float = OpTypeMatrix %v3float 3
%float_3 = OpConstant %float 3
%39 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%false = OpConstantFalse %bool
%float_6 = OpConstant %float 6
%47 = OpConstantComposite %v3float %float_6 %float_6 %float_6
%v3bool = OpTypeVector %bool 3
%float_9 = OpConstant %float 9
%63 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%float_5 = OpConstant %float 5
%95 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_8 = OpConstant %float 8
%float_7 = OpConstant %float 7
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%378 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%v1 = OpVariable %_ptr_Function_v3float Function
%v2 = OpVariable %_ptr_Function_v3float Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%35 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%33 = OpCompositeConstruct %mat3v3float %34 %35 %36
%40 = OpMatrixTimesVector %v3float %33 %39
OpStore %v1 %40
%42 = OpLoad %bool %ok
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%45 = OpLoad %v3float %v1
%48 = OpFOrdEqual %v3bool %45 %47
%50 = OpAll %bool %48
OpBranch %44
%44 = OpLabel
%51 = OpPhi %bool %false %25 %50 %43
OpStore %ok %51
%54 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%55 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%56 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%53 = OpCompositeConstruct %mat3v3float %54 %55 %56
%57 = OpVectorTimesMatrix %v3float %39 %53
OpStore %v2 %57
%58 = OpLoad %bool %ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %v3float %v2
%64 = OpFOrdEqual %v3bool %61 %63
%65 = OpAll %bool %64
OpBranch %60
%60 = OpLabel
%66 = OpPhi %bool %false %44 %65 %59
OpStore %ok %66
%73 = OpCompositeConstruct %v2float %float_1 %float_2
%74 = OpCompositeConstruct %v2float %float_3 %float_4
%72 = OpCompositeConstruct %mat2v2float %73 %74
OpStore %m1 %72
%75 = OpLoad %bool %ok
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %mat2v2float %m1
%80 = OpCompositeConstruct %v2float %float_1 %float_2
%81 = OpCompositeConstruct %v2float %float_3 %float_4
%79 = OpCompositeConstruct %mat2v2float %80 %81
%83 = OpCompositeExtract %v2float %78 0
%84 = OpCompositeExtract %v2float %79 0
%85 = OpFOrdEqual %v2bool %83 %84
%86 = OpAll %bool %85
%87 = OpCompositeExtract %v2float %78 1
%88 = OpCompositeExtract %v2float %79 1
%89 = OpFOrdEqual %v2bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
OpBranch %77
%77 = OpLabel
%92 = OpPhi %bool %false %60 %91 %76
OpStore %ok %92
%97 = OpCompositeExtract %float %95 0
%98 = OpCompositeExtract %float %95 1
%99 = OpCompositeExtract %float %95 2
%100 = OpCompositeExtract %float %95 3
%101 = OpCompositeConstruct %v2float %97 %98
%102 = OpCompositeConstruct %v2float %99 %100
%96 = OpCompositeConstruct %mat2v2float %101 %102
OpStore %m2 %96
%103 = OpLoad %bool %ok
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %mat2v2float %m2
%108 = OpCompositeConstruct %v2float %float_5 %float_5
%109 = OpCompositeConstruct %v2float %float_5 %float_5
%107 = OpCompositeConstruct %mat2v2float %108 %109
%110 = OpCompositeExtract %v2float %106 0
%111 = OpCompositeExtract %v2float %107 0
%112 = OpFOrdEqual %v2bool %110 %111
%113 = OpAll %bool %112
%114 = OpCompositeExtract %v2float %106 1
%115 = OpCompositeExtract %v2float %107 1
%116 = OpFOrdEqual %v2bool %114 %115
%117 = OpAll %bool %116
%118 = OpLogicalAnd %bool %113 %117
OpBranch %105
%105 = OpLabel
%119 = OpPhi %bool %false %77 %118 %104
OpStore %ok %119
%121 = OpLoad %mat2v2float %m1
OpStore %m3 %121
%122 = OpLoad %bool %ok
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %mat2v2float %m3
%127 = OpCompositeConstruct %v2float %float_1 %float_2
%128 = OpCompositeConstruct %v2float %float_3 %float_4
%126 = OpCompositeConstruct %mat2v2float %127 %128
%129 = OpCompositeExtract %v2float %125 0
%130 = OpCompositeExtract %v2float %126 0
%131 = OpFOrdEqual %v2bool %129 %130
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v2float %125 1
%134 = OpCompositeExtract %v2float %126 1
%135 = OpFOrdEqual %v2bool %133 %134
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %132 %136
OpBranch %124
%124 = OpLabel
%138 = OpPhi %bool %false %105 %137 %123
OpStore %ok %138
%141 = OpCompositeConstruct %v2float %float_6 %float_0
%142 = OpCompositeConstruct %v2float %float_0 %float_6
%140 = OpCompositeConstruct %mat2v2float %141 %142
OpStore %m4 %140
%143 = OpLoad %bool %ok
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpLoad %mat2v2float %m4
%148 = OpCompositeConstruct %v2float %float_6 %float_0
%149 = OpCompositeConstruct %v2float %float_0 %float_6
%147 = OpCompositeConstruct %mat2v2float %148 %149
%150 = OpCompositeExtract %v2float %146 0
%151 = OpCompositeExtract %v2float %147 0
%152 = OpFOrdEqual %v2bool %150 %151
%153 = OpAll %bool %152
%154 = OpCompositeExtract %v2float %146 1
%155 = OpCompositeExtract %v2float %147 1
%156 = OpFOrdEqual %v2bool %154 %155
%157 = OpAll %bool %156
%158 = OpLogicalAnd %bool %153 %157
OpBranch %145
%145 = OpLabel
%159 = OpPhi %bool %false %124 %158 %144
OpStore %ok %159
%160 = OpLoad %mat2v2float %m3
%161 = OpLoad %mat2v2float %m4
%162 = OpMatrixTimesMatrix %mat2v2float %160 %161
OpStore %m3 %162
%163 = OpLoad %bool %ok
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %mat2v2float %m3
%171 = OpCompositeConstruct %v2float %float_6 %float_12
%172 = OpCompositeConstruct %v2float %float_18 %float_24
%170 = OpCompositeConstruct %mat2v2float %171 %172
%173 = OpCompositeExtract %v2float %166 0
%174 = OpCompositeExtract %v2float %170 0
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpCompositeExtract %v2float %166 1
%178 = OpCompositeExtract %v2float %170 1
%179 = OpFOrdEqual %v2bool %177 %178
%180 = OpAll %bool %179
%181 = OpLogicalAnd %bool %176 %180
OpBranch %165
%165 = OpLabel
%182 = OpPhi %bool %false %145 %181 %164
OpStore %ok %182
%186 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%187 = OpLoad %v2float %186
%188 = OpCompositeExtract %float %187 1
%190 = OpCompositeConstruct %v2float %188 %float_0
%191 = OpCompositeConstruct %v2float %float_0 %188
%189 = OpCompositeConstruct %mat2v2float %190 %191
OpStore %m5 %189
%192 = OpLoad %bool %ok
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %mat2v2float %m5
%197 = OpCompositeConstruct %v2float %float_4 %float_0
%198 = OpCompositeConstruct %v2float %float_0 %float_4
%196 = OpCompositeConstruct %mat2v2float %197 %198
%199 = OpCompositeExtract %v2float %195 0
%200 = OpCompositeExtract %v2float %196 0
%201 = OpFOrdEqual %v2bool %199 %200
%202 = OpAll %bool %201
%203 = OpCompositeExtract %v2float %195 1
%204 = OpCompositeExtract %v2float %196 1
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %202 %206
OpBranch %194
%194 = OpLabel
%208 = OpPhi %bool %false %165 %207 %193
OpStore %ok %208
%209 = OpLoad %mat2v2float %m1
%210 = OpLoad %mat2v2float %m5
%211 = OpCompositeExtract %v2float %209 0
%212 = OpCompositeExtract %v2float %210 0
%213 = OpFAdd %v2float %211 %212
%214 = OpCompositeExtract %v2float %209 1
%215 = OpCompositeExtract %v2float %210 1
%216 = OpFAdd %v2float %214 %215
%217 = OpCompositeConstruct %mat2v2float %213 %216
OpStore %m1 %217
%218 = OpLoad %bool %ok
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %mat2v2float %m1
%224 = OpCompositeConstruct %v2float %float_5 %float_2
%225 = OpCompositeConstruct %v2float %float_3 %float_8
%223 = OpCompositeConstruct %mat2v2float %224 %225
%226 = OpCompositeExtract %v2float %221 0
%227 = OpCompositeExtract %v2float %223 0
%228 = OpFOrdEqual %v2bool %226 %227
%229 = OpAll %bool %228
%230 = OpCompositeExtract %v2float %221 1
%231 = OpCompositeExtract %v2float %223 1
%232 = OpFOrdEqual %v2bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
OpBranch %220
%220 = OpLabel
%235 = OpPhi %bool %false %194 %234 %219
OpStore %ok %235
%239 = OpCompositeConstruct %v2float %float_5 %float_6
%240 = OpCompositeConstruct %v2float %float_7 %float_8
%238 = OpCompositeConstruct %mat2v2float %239 %240
OpStore %m7 %238
%241 = OpLoad %bool %ok
OpSelectionMerge %243 None
OpBranchConditional %241 %242 %243
%242 = OpLabel
%244 = OpLoad %mat2v2float %m7
%246 = OpCompositeConstruct %v2float %float_5 %float_6
%247 = OpCompositeConstruct %v2float %float_7 %float_8
%245 = OpCompositeConstruct %mat2v2float %246 %247
%248 = OpCompositeExtract %v2float %244 0
%249 = OpCompositeExtract %v2float %245 0
%250 = OpFOrdEqual %v2bool %248 %249
%251 = OpAll %bool %250
%252 = OpCompositeExtract %v2float %244 1
%253 = OpCompositeExtract %v2float %245 1
%254 = OpFOrdEqual %v2bool %252 %253
%255 = OpAll %bool %254
%256 = OpLogicalAnd %bool %251 %255
OpBranch %243
%243 = OpLabel
%257 = OpPhi %bool %false %220 %256 %242
OpStore %ok %257
%261 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%262 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%263 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%260 = OpCompositeConstruct %mat3v3float %261 %262 %263
OpStore %m9 %260
%264 = OpLoad %bool %ok
OpSelectionMerge %266 None
OpBranchConditional %264 %265 %266
%265 = OpLabel
%267 = OpLoad %mat3v3float %m9
%269 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%270 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%271 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%268 = OpCompositeConstruct %mat3v3float %269 %270 %271
%272 = OpCompositeExtract %v3float %267 0
%273 = OpCompositeExtract %v3float %268 0
%274 = OpFOrdEqual %v3bool %272 %273
%275 = OpAll %bool %274
%276 = OpCompositeExtract %v3float %267 1
%277 = OpCompositeExtract %v3float %268 1
%278 = OpFOrdEqual %v3bool %276 %277
%279 = OpAll %bool %278
%280 = OpLogicalAnd %bool %275 %279
%281 = OpCompositeExtract %v3float %267 2
%282 = OpCompositeExtract %v3float %268 2
%283 = OpFOrdEqual %v3bool %281 %282
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %280 %284
OpBranch %266
%266 = OpLabel
%286 = OpPhi %bool %false %243 %285 %265
OpStore %ok %286
%292 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%293 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%294 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%295 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%291 = OpCompositeConstruct %mat4v4float %292 %293 %294 %295
OpStore %m10 %291
%296 = OpLoad %bool %ok
OpSelectionMerge %298 None
OpBranchConditional %296 %297 %298
%297 = OpLabel
%299 = OpLoad %mat4v4float %m10
%301 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%302 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%303 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%304 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%300 = OpCompositeConstruct %mat4v4float %301 %302 %303 %304
%306 = OpCompositeExtract %v4float %299 0
%307 = OpCompositeExtract %v4float %300 0
%308 = OpFOrdEqual %v4bool %306 %307
%309 = OpAll %bool %308
%310 = OpCompositeExtract %v4float %299 1
%311 = OpCompositeExtract %v4float %300 1
%312 = OpFOrdEqual %v4bool %310 %311
%313 = OpAll %bool %312
%314 = OpLogicalAnd %bool %309 %313
%315 = OpCompositeExtract %v4float %299 2
%316 = OpCompositeExtract %v4float %300 2
%317 = OpFOrdEqual %v4bool %315 %316
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %314 %318
%320 = OpCompositeExtract %v4float %299 3
%321 = OpCompositeExtract %v4float %300 3
%322 = OpFOrdEqual %v4bool %320 %321
%323 = OpAll %bool %322
%324 = OpLogicalAnd %bool %319 %323
OpBranch %298
%298 = OpLabel
%325 = OpPhi %bool %false %266 %324 %297
OpStore %ok %325
%329 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%330 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%331 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%332 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%328 = OpCompositeConstruct %mat4v4float %329 %330 %331 %332
OpStore %m11 %328
%333 = OpLoad %mat4v4float %m11
%334 = OpLoad %mat4v4float %m10
%335 = OpCompositeExtract %v4float %333 0
%336 = OpCompositeExtract %v4float %334 0
%337 = OpFSub %v4float %335 %336
%338 = OpCompositeExtract %v4float %333 1
%339 = OpCompositeExtract %v4float %334 1
%340 = OpFSub %v4float %338 %339
%341 = OpCompositeExtract %v4float %333 2
%342 = OpCompositeExtract %v4float %334 2
%343 = OpFSub %v4float %341 %342
%344 = OpCompositeExtract %v4float %333 3
%345 = OpCompositeExtract %v4float %334 3
%346 = OpFSub %v4float %344 %345
%347 = OpCompositeConstruct %mat4v4float %337 %340 %343 %346
OpStore %m11 %347
%348 = OpLoad %bool %ok
OpSelectionMerge %350 None
OpBranchConditional %348 %349 %350
%349 = OpLabel
%351 = OpLoad %mat4v4float %m11
%353 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%354 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%355 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%356 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%352 = OpCompositeConstruct %mat4v4float %353 %354 %355 %356
%357 = OpCompositeExtract %v4float %351 0
%358 = OpCompositeExtract %v4float %352 0
%359 = OpFOrdEqual %v4bool %357 %358
%360 = OpAll %bool %359
%361 = OpCompositeExtract %v4float %351 1
%362 = OpCompositeExtract %v4float %352 1
%363 = OpFOrdEqual %v4bool %361 %362
%364 = OpAll %bool %363
%365 = OpLogicalAnd %bool %360 %364
%366 = OpCompositeExtract %v4float %351 2
%367 = OpCompositeExtract %v4float %352 2
%368 = OpFOrdEqual %v4bool %366 %367
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %365 %369
%371 = OpCompositeExtract %v4float %351 3
%372 = OpCompositeExtract %v4float %352 3
%373 = OpFOrdEqual %v4bool %371 %372
%374 = OpAll %bool %373
%375 = OpLogicalAnd %bool %370 %374
OpBranch %350
%350 = OpLabel
%376 = OpPhi %bool %false %298 %375 %349
OpStore %ok %376
%377 = OpLoad %bool %ok
OpReturnValue %377
OpFunctionEnd
%main = OpFunction %v4float None %378
%379 = OpFunctionParameter %_ptr_Function_v2float
%380 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_v1 = OpVariable %_ptr_Function_v3float Function
%_2_v2 = OpVariable %_ptr_Function_v3float Function
%_3_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_10_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_11_m11 = OpVariable %_ptr_Function_mat4v4float Function
%703 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%384 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%385 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%386 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%383 = OpCompositeConstruct %mat3v3float %384 %385 %386
%387 = OpMatrixTimesVector %v3float %383 %39
OpStore %_1_v1 %387
%388 = OpLoad %bool %_0_ok
OpSelectionMerge %390 None
OpBranchConditional %388 %389 %390
%389 = OpLabel
%391 = OpLoad %v3float %_1_v1
%392 = OpFOrdEqual %v3bool %391 %47
%393 = OpAll %bool %392
OpBranch %390
%390 = OpLabel
%394 = OpPhi %bool %false %380 %393 %389
OpStore %_0_ok %394
%397 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%398 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%399 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%396 = OpCompositeConstruct %mat3v3float %397 %398 %399
%400 = OpVectorTimesMatrix %v3float %39 %396
OpStore %_2_v2 %400
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpLoad %v3float %_2_v2
%405 = OpFOrdEqual %v3bool %404 %63
%406 = OpAll %bool %405
OpBranch %403
%403 = OpLabel
%407 = OpPhi %bool %false %390 %406 %402
OpStore %_0_ok %407
%410 = OpCompositeConstruct %v2float %float_1 %float_2
%411 = OpCompositeConstruct %v2float %float_3 %float_4
%409 = OpCompositeConstruct %mat2v2float %410 %411
OpStore %_3_m1 %409
%412 = OpLoad %bool %_0_ok
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpLoad %mat2v2float %_3_m1
%417 = OpCompositeConstruct %v2float %float_1 %float_2
%418 = OpCompositeConstruct %v2float %float_3 %float_4
%416 = OpCompositeConstruct %mat2v2float %417 %418
%419 = OpCompositeExtract %v2float %415 0
%420 = OpCompositeExtract %v2float %416 0
%421 = OpFOrdEqual %v2bool %419 %420
%422 = OpAll %bool %421
%423 = OpCompositeExtract %v2float %415 1
%424 = OpCompositeExtract %v2float %416 1
%425 = OpFOrdEqual %v2bool %423 %424
%426 = OpAll %bool %425
%427 = OpLogicalAnd %bool %422 %426
OpBranch %414
%414 = OpLabel
%428 = OpPhi %bool %false %403 %427 %413
OpStore %_0_ok %428
%431 = OpCompositeExtract %float %95 0
%432 = OpCompositeExtract %float %95 1
%433 = OpCompositeExtract %float %95 2
%434 = OpCompositeExtract %float %95 3
%435 = OpCompositeConstruct %v2float %431 %432
%436 = OpCompositeConstruct %v2float %433 %434
%430 = OpCompositeConstruct %mat2v2float %435 %436
OpStore %_4_m2 %430
%437 = OpLoad %bool %_0_ok
OpSelectionMerge %439 None
OpBranchConditional %437 %438 %439
%438 = OpLabel
%440 = OpLoad %mat2v2float %_4_m2
%442 = OpCompositeConstruct %v2float %float_5 %float_5
%443 = OpCompositeConstruct %v2float %float_5 %float_5
%441 = OpCompositeConstruct %mat2v2float %442 %443
%444 = OpCompositeExtract %v2float %440 0
%445 = OpCompositeExtract %v2float %441 0
%446 = OpFOrdEqual %v2bool %444 %445
%447 = OpAll %bool %446
%448 = OpCompositeExtract %v2float %440 1
%449 = OpCompositeExtract %v2float %441 1
%450 = OpFOrdEqual %v2bool %448 %449
%451 = OpAll %bool %450
%452 = OpLogicalAnd %bool %447 %451
OpBranch %439
%439 = OpLabel
%453 = OpPhi %bool %false %414 %452 %438
OpStore %_0_ok %453
%455 = OpLoad %mat2v2float %_3_m1
OpStore %_5_m3 %455
%456 = OpLoad %bool %_0_ok
OpSelectionMerge %458 None
OpBranchConditional %456 %457 %458
%457 = OpLabel
%459 = OpLoad %mat2v2float %_5_m3
%461 = OpCompositeConstruct %v2float %float_1 %float_2
%462 = OpCompositeConstruct %v2float %float_3 %float_4
%460 = OpCompositeConstruct %mat2v2float %461 %462
%463 = OpCompositeExtract %v2float %459 0
%464 = OpCompositeExtract %v2float %460 0
%465 = OpFOrdEqual %v2bool %463 %464
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v2float %459 1
%468 = OpCompositeExtract %v2float %460 1
%469 = OpFOrdEqual %v2bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
OpBranch %458
%458 = OpLabel
%472 = OpPhi %bool %false %439 %471 %457
OpStore %_0_ok %472
%475 = OpCompositeConstruct %v2float %float_6 %float_0
%476 = OpCompositeConstruct %v2float %float_0 %float_6
%474 = OpCompositeConstruct %mat2v2float %475 %476
OpStore %_6_m4 %474
%477 = OpLoad %bool %_0_ok
OpSelectionMerge %479 None
OpBranchConditional %477 %478 %479
%478 = OpLabel
%480 = OpLoad %mat2v2float %_6_m4
%482 = OpCompositeConstruct %v2float %float_6 %float_0
%483 = OpCompositeConstruct %v2float %float_0 %float_6
%481 = OpCompositeConstruct %mat2v2float %482 %483
%484 = OpCompositeExtract %v2float %480 0
%485 = OpCompositeExtract %v2float %481 0
%486 = OpFOrdEqual %v2bool %484 %485
%487 = OpAll %bool %486
%488 = OpCompositeExtract %v2float %480 1
%489 = OpCompositeExtract %v2float %481 1
%490 = OpFOrdEqual %v2bool %488 %489
%491 = OpAll %bool %490
%492 = OpLogicalAnd %bool %487 %491
OpBranch %479
%479 = OpLabel
%493 = OpPhi %bool %false %458 %492 %478
OpStore %_0_ok %493
%494 = OpLoad %mat2v2float %_5_m3
%495 = OpLoad %mat2v2float %_6_m4
%496 = OpMatrixTimesMatrix %mat2v2float %494 %495
OpStore %_5_m3 %496
%497 = OpLoad %bool %_0_ok
OpSelectionMerge %499 None
OpBranchConditional %497 %498 %499
%498 = OpLabel
%500 = OpLoad %mat2v2float %_5_m3
%502 = OpCompositeConstruct %v2float %float_6 %float_12
%503 = OpCompositeConstruct %v2float %float_18 %float_24
%501 = OpCompositeConstruct %mat2v2float %502 %503
%504 = OpCompositeExtract %v2float %500 0
%505 = OpCompositeExtract %v2float %501 0
%506 = OpFOrdEqual %v2bool %504 %505
%507 = OpAll %bool %506
%508 = OpCompositeExtract %v2float %500 1
%509 = OpCompositeExtract %v2float %501 1
%510 = OpFOrdEqual %v2bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
OpBranch %499
%499 = OpLabel
%513 = OpPhi %bool %false %479 %512 %498
OpStore %_0_ok %513
%515 = OpAccessChain %_ptr_Function_v2float %_3_m1 %int_1
%516 = OpLoad %v2float %515
%517 = OpCompositeExtract %float %516 1
%519 = OpCompositeConstruct %v2float %517 %float_0
%520 = OpCompositeConstruct %v2float %float_0 %517
%518 = OpCompositeConstruct %mat2v2float %519 %520
OpStore %_7_m5 %518
%521 = OpLoad %bool %_0_ok
OpSelectionMerge %523 None
OpBranchConditional %521 %522 %523
%522 = OpLabel
%524 = OpLoad %mat2v2float %_7_m5
%526 = OpCompositeConstruct %v2float %float_4 %float_0
%527 = OpCompositeConstruct %v2float %float_0 %float_4
%525 = OpCompositeConstruct %mat2v2float %526 %527
%528 = OpCompositeExtract %v2float %524 0
%529 = OpCompositeExtract %v2float %525 0
%530 = OpFOrdEqual %v2bool %528 %529
%531 = OpAll %bool %530
%532 = OpCompositeExtract %v2float %524 1
%533 = OpCompositeExtract %v2float %525 1
%534 = OpFOrdEqual %v2bool %532 %533
%535 = OpAll %bool %534
%536 = OpLogicalAnd %bool %531 %535
OpBranch %523
%523 = OpLabel
%537 = OpPhi %bool %false %499 %536 %522
OpStore %_0_ok %537
%538 = OpLoad %mat2v2float %_3_m1
%539 = OpLoad %mat2v2float %_7_m5
%540 = OpCompositeExtract %v2float %538 0
%541 = OpCompositeExtract %v2float %539 0
%542 = OpFAdd %v2float %540 %541
%543 = OpCompositeExtract %v2float %538 1
%544 = OpCompositeExtract %v2float %539 1
%545 = OpFAdd %v2float %543 %544
%546 = OpCompositeConstruct %mat2v2float %542 %545
OpStore %_3_m1 %546
%547 = OpLoad %bool %_0_ok
OpSelectionMerge %549 None
OpBranchConditional %547 %548 %549
%548 = OpLabel
%550 = OpLoad %mat2v2float %_3_m1
%552 = OpCompositeConstruct %v2float %float_5 %float_2
%553 = OpCompositeConstruct %v2float %float_3 %float_8
%551 = OpCompositeConstruct %mat2v2float %552 %553
%554 = OpCompositeExtract %v2float %550 0
%555 = OpCompositeExtract %v2float %551 0
%556 = OpFOrdEqual %v2bool %554 %555
%557 = OpAll %bool %556
%558 = OpCompositeExtract %v2float %550 1
%559 = OpCompositeExtract %v2float %551 1
%560 = OpFOrdEqual %v2bool %558 %559
%561 = OpAll %bool %560
%562 = OpLogicalAnd %bool %557 %561
OpBranch %549
%549 = OpLabel
%563 = OpPhi %bool %false %523 %562 %548
OpStore %_0_ok %563
%566 = OpCompositeConstruct %v2float %float_5 %float_6
%567 = OpCompositeConstruct %v2float %float_7 %float_8
%565 = OpCompositeConstruct %mat2v2float %566 %567
OpStore %_8_m7 %565
%568 = OpLoad %bool %_0_ok
OpSelectionMerge %570 None
OpBranchConditional %568 %569 %570
%569 = OpLabel
%571 = OpLoad %mat2v2float %_8_m7
%573 = OpCompositeConstruct %v2float %float_5 %float_6
%574 = OpCompositeConstruct %v2float %float_7 %float_8
%572 = OpCompositeConstruct %mat2v2float %573 %574
%575 = OpCompositeExtract %v2float %571 0
%576 = OpCompositeExtract %v2float %572 0
%577 = OpFOrdEqual %v2bool %575 %576
%578 = OpAll %bool %577
%579 = OpCompositeExtract %v2float %571 1
%580 = OpCompositeExtract %v2float %572 1
%581 = OpFOrdEqual %v2bool %579 %580
%582 = OpAll %bool %581
%583 = OpLogicalAnd %bool %578 %582
OpBranch %570
%570 = OpLabel
%584 = OpPhi %bool %false %549 %583 %569
OpStore %_0_ok %584
%587 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%588 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%589 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%586 = OpCompositeConstruct %mat3v3float %587 %588 %589
OpStore %_9_m9 %586
%590 = OpLoad %bool %_0_ok
OpSelectionMerge %592 None
OpBranchConditional %590 %591 %592
%591 = OpLabel
%593 = OpLoad %mat3v3float %_9_m9
%595 = OpCompositeConstruct %v3float %float_9 %float_0 %float_0
%596 = OpCompositeConstruct %v3float %float_0 %float_9 %float_0
%597 = OpCompositeConstruct %v3float %float_0 %float_0 %float_9
%594 = OpCompositeConstruct %mat3v3float %595 %596 %597
%598 = OpCompositeExtract %v3float %593 0
%599 = OpCompositeExtract %v3float %594 0
%600 = OpFOrdEqual %v3bool %598 %599
%601 = OpAll %bool %600
%602 = OpCompositeExtract %v3float %593 1
%603 = OpCompositeExtract %v3float %594 1
%604 = OpFOrdEqual %v3bool %602 %603
%605 = OpAll %bool %604
%606 = OpLogicalAnd %bool %601 %605
%607 = OpCompositeExtract %v3float %593 2
%608 = OpCompositeExtract %v3float %594 2
%609 = OpFOrdEqual %v3bool %607 %608
%610 = OpAll %bool %609
%611 = OpLogicalAnd %bool %606 %610
OpBranch %592
%592 = OpLabel
%612 = OpPhi %bool %false %570 %611 %591
OpStore %_0_ok %612
%615 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%616 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%617 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%618 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%614 = OpCompositeConstruct %mat4v4float %615 %616 %617 %618
OpStore %_10_m10 %614
%619 = OpLoad %bool %_0_ok
OpSelectionMerge %621 None
OpBranchConditional %619 %620 %621
%620 = OpLabel
%622 = OpLoad %mat4v4float %_10_m10
%624 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%625 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%626 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%627 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%623 = OpCompositeConstruct %mat4v4float %624 %625 %626 %627
%628 = OpCompositeExtract %v4float %622 0
%629 = OpCompositeExtract %v4float %623 0
%630 = OpFOrdEqual %v4bool %628 %629
%631 = OpAll %bool %630
%632 = OpCompositeExtract %v4float %622 1
%633 = OpCompositeExtract %v4float %623 1
%634 = OpFOrdEqual %v4bool %632 %633
%635 = OpAll %bool %634
%636 = OpLogicalAnd %bool %631 %635
%637 = OpCompositeExtract %v4float %622 2
%638 = OpCompositeExtract %v4float %623 2
%639 = OpFOrdEqual %v4bool %637 %638
%640 = OpAll %bool %639
%641 = OpLogicalAnd %bool %636 %640
%642 = OpCompositeExtract %v4float %622 3
%643 = OpCompositeExtract %v4float %623 3
%644 = OpFOrdEqual %v4bool %642 %643
%645 = OpAll %bool %644
%646 = OpLogicalAnd %bool %641 %645
OpBranch %621
%621 = OpLabel
%647 = OpPhi %bool %false %592 %646 %620
OpStore %_0_ok %647
%650 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%651 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%652 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%653 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%649 = OpCompositeConstruct %mat4v4float %650 %651 %652 %653
OpStore %_11_m11 %649
%654 = OpLoad %mat4v4float %_11_m11
%655 = OpLoad %mat4v4float %_10_m10
%656 = OpCompositeExtract %v4float %654 0
%657 = OpCompositeExtract %v4float %655 0
%658 = OpFSub %v4float %656 %657
%659 = OpCompositeExtract %v4float %654 1
%660 = OpCompositeExtract %v4float %655 1
%661 = OpFSub %v4float %659 %660
%662 = OpCompositeExtract %v4float %654 2
%663 = OpCompositeExtract %v4float %655 2
%664 = OpFSub %v4float %662 %663
%665 = OpCompositeExtract %v4float %654 3
%666 = OpCompositeExtract %v4float %655 3
%667 = OpFSub %v4float %665 %666
%668 = OpCompositeConstruct %mat4v4float %658 %661 %664 %667
OpStore %_11_m11 %668
%669 = OpLoad %bool %_0_ok
OpSelectionMerge %671 None
OpBranchConditional %669 %670 %671
%670 = OpLabel
%672 = OpLoad %mat4v4float %_11_m11
%674 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%675 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%676 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%677 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%673 = OpCompositeConstruct %mat4v4float %674 %675 %676 %677
%678 = OpCompositeExtract %v4float %672 0
%679 = OpCompositeExtract %v4float %673 0
%680 = OpFOrdEqual %v4bool %678 %679
%681 = OpAll %bool %680
%682 = OpCompositeExtract %v4float %672 1
%683 = OpCompositeExtract %v4float %673 1
%684 = OpFOrdEqual %v4bool %682 %683
%685 = OpAll %bool %684
%686 = OpLogicalAnd %bool %681 %685
%687 = OpCompositeExtract %v4float %672 2
%688 = OpCompositeExtract %v4float %673 2
%689 = OpFOrdEqual %v4bool %687 %688
%690 = OpAll %bool %689
%691 = OpLogicalAnd %bool %686 %690
%692 = OpCompositeExtract %v4float %672 3
%693 = OpCompositeExtract %v4float %673 3
%694 = OpFOrdEqual %v4bool %692 %693
%695 = OpAll %bool %694
%696 = OpLogicalAnd %bool %691 %695
OpBranch %671
%671 = OpLabel
%697 = OpPhi %bool %false %621 %696 %670
OpStore %_0_ok %697
%698 = OpLoad %bool %_0_ok
OpSelectionMerge %700 None
OpBranchConditional %698 %699 %700
%699 = OpLabel
%701 = OpFunctionCall %bool %test_half_b
OpBranch %700
%700 = OpLabel
%702 = OpPhi %bool %false %671 %701 %699
OpSelectionMerge %707 None
OpBranchConditional %702 %705 %706
%705 = OpLabel
%708 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%711 = OpLoad %v4float %708
OpStore %703 %711
OpBranch %707
%706 = OpLabel
%712 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%713 = OpLoad %v4float %712
OpStore %703 %713
OpBranch %707
%707 = OpLabel
%714 = OpLoad %v4float %703
OpReturnValue %714
OpFunctionEnd
