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
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
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
OpDecorate %m23 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
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
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
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
OpDecorate %349 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %511 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %661 RelaxedPrecision
OpDecorate %678 RelaxedPrecision
OpDecorate %692 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %696 RelaxedPrecision
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
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%368 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%42 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%43 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%44 = OpCompositeConstruct %mat2v3float %42 %43
%46 = OpCompositeExtract %v3float %41 0
%47 = OpCompositeExtract %v3float %44 0
%48 = OpFOrdEqual %v3bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v3float %41 1
%51 = OpCompositeExtract %v3float %44 1
%52 = OpFOrdEqual %v3bool %50 %51
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %49 %53
OpBranch %40
%40 = OpLabel
%55 = OpPhi %bool %false %25 %54 %39
OpStore %ok %55
%61 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%62 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%60 = OpCompositeConstruct %mat2v4float %61 %62
OpStore %m24 %60
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v4float %m24
%67 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%68 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%69 = OpCompositeConstruct %mat2v4float %67 %68
%71 = OpCompositeExtract %v4float %66 0
%72 = OpCompositeExtract %v4float %69 0
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v4float %66 1
%76 = OpCompositeExtract %v4float %69 1
%77 = OpFOrdEqual %v4bool %75 %76
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %74 %78
OpBranch %65
%65 = OpLabel
%80 = OpPhi %bool %false %40 %79 %64
OpStore %ok %80
%86 = OpCompositeConstruct %v2float %float_4 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_4
%88 = OpCompositeConstruct %v2float %float_0 %float_0
%85 = OpCompositeConstruct %mat3v2float %86 %87 %88
OpStore %m32 %85
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %mat3v2float %m32
%93 = OpCompositeConstruct %v2float %float_4 %float_0
%94 = OpCompositeConstruct %v2float %float_0 %float_4
%95 = OpCompositeConstruct %v2float %float_0 %float_0
%96 = OpCompositeConstruct %mat3v2float %93 %94 %95
%98 = OpCompositeExtract %v2float %92 0
%99 = OpCompositeExtract %v2float %96 0
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpCompositeExtract %v2float %92 1
%103 = OpCompositeExtract %v2float %96 1
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %101 %105
%107 = OpCompositeExtract %v2float %92 2
%108 = OpCompositeExtract %v2float %96 2
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %106 %110
OpBranch %91
%91 = OpLabel
%112 = OpPhi %bool %false %65 %111 %90
OpStore %ok %112
%118 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%117 = OpCompositeConstruct %mat3v4float %118 %119 %120
OpStore %m34 %117
%121 = OpLoad %bool %ok
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %mat3v4float %m34
%125 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%126 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%128 = OpCompositeConstruct %mat3v4float %125 %126 %127
%129 = OpCompositeExtract %v4float %124 0
%130 = OpCompositeExtract %v4float %128 0
%131 = OpFOrdEqual %v4bool %129 %130
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v4float %124 1
%134 = OpCompositeExtract %v4float %128 1
%135 = OpFOrdEqual %v4bool %133 %134
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %132 %136
%138 = OpCompositeExtract %v4float %124 2
%139 = OpCompositeExtract %v4float %128 2
%140 = OpFOrdEqual %v4bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
OpBranch %123
%123 = OpLabel
%143 = OpPhi %bool %false %91 %142 %122
OpStore %ok %143
%149 = OpCompositeConstruct %v2float %float_6 %float_0
%150 = OpCompositeConstruct %v2float %float_0 %float_6
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%152 = OpCompositeConstruct %v2float %float_0 %float_0
%148 = OpCompositeConstruct %mat4v2float %149 %150 %151 %152
OpStore %m42 %148
%153 = OpLoad %bool %ok
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %mat4v2float %m42
%157 = OpCompositeConstruct %v2float %float_6 %float_0
%158 = OpCompositeConstruct %v2float %float_0 %float_6
%159 = OpCompositeConstruct %v2float %float_0 %float_0
%160 = OpCompositeConstruct %v2float %float_0 %float_0
%161 = OpCompositeConstruct %mat4v2float %157 %158 %159 %160
%162 = OpCompositeExtract %v2float %156 0
%163 = OpCompositeExtract %v2float %161 0
%164 = OpFOrdEqual %v2bool %162 %163
%165 = OpAll %bool %164
%166 = OpCompositeExtract %v2float %156 1
%167 = OpCompositeExtract %v2float %161 1
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
%171 = OpCompositeExtract %v2float %156 2
%172 = OpCompositeExtract %v2float %161 2
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v2float %156 3
%177 = OpCompositeExtract %v2float %161 3
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
OpBranch %155
%155 = OpLabel
%181 = OpPhi %bool %false %123 %180 %154
OpStore %ok %181
%187 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%188 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%186 = OpCompositeConstruct %mat4v3float %187 %188 %189 %190
OpStore %m43 %186
%191 = OpLoad %bool %ok
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpLoad %mat4v3float %m43
%195 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%196 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%198 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%199 = OpCompositeConstruct %mat4v3float %195 %196 %197 %198
%200 = OpCompositeExtract %v3float %194 0
%201 = OpCompositeExtract %v3float %199 0
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpCompositeExtract %v3float %194 1
%205 = OpCompositeExtract %v3float %199 1
%206 = OpFOrdEqual %v3bool %204 %205
%207 = OpAll %bool %206
%208 = OpLogicalAnd %bool %203 %207
%209 = OpCompositeExtract %v3float %194 2
%210 = OpCompositeExtract %v3float %199 2
%211 = OpFOrdEqual %v3bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
%214 = OpCompositeExtract %v3float %194 3
%215 = OpCompositeExtract %v3float %199 3
%216 = OpFOrdEqual %v3bool %214 %215
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %213 %217
OpBranch %193
%193 = OpLabel
%219 = OpPhi %bool %false %155 %218 %192
OpStore %ok %219
%223 = OpLoad %mat3v2float %m32
%224 = OpLoad %mat2v3float %m23
%225 = OpMatrixTimesMatrix %mat2v2float %223 %224
OpStore %m22 %225
%226 = OpLoad %bool %ok
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %mat2v2float %m22
%232 = OpCompositeConstruct %v2float %float_8 %float_0
%233 = OpCompositeConstruct %v2float %float_0 %float_8
%231 = OpCompositeConstruct %mat2v2float %232 %233
%234 = OpCompositeExtract %v2float %229 0
%235 = OpCompositeExtract %v2float %231 0
%236 = OpFOrdEqual %v2bool %234 %235
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v2float %229 1
%239 = OpCompositeExtract %v2float %231 1
%240 = OpFOrdEqual %v2bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
OpBranch %228
%228 = OpLabel
%243 = OpPhi %bool %false %193 %242 %227
OpStore %ok %243
%247 = OpLoad %mat4v3float %m43
%248 = OpLoad %mat3v4float %m34
%249 = OpMatrixTimesMatrix %mat3v3float %247 %248
OpStore %m33 %249
%250 = OpLoad %bool %ok
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %mat3v3float %m33
%256 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%257 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%258 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%255 = OpCompositeConstruct %mat3v3float %256 %257 %258
%259 = OpCompositeExtract %v3float %253 0
%260 = OpCompositeExtract %v3float %255 0
%261 = OpFOrdEqual %v3bool %259 %260
%262 = OpAll %bool %261
%263 = OpCompositeExtract %v3float %253 1
%264 = OpCompositeExtract %v3float %255 1
%265 = OpFOrdEqual %v3bool %263 %264
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %262 %266
%268 = OpCompositeExtract %v3float %253 2
%269 = OpCompositeExtract %v3float %255 2
%270 = OpFOrdEqual %v3bool %268 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %267 %271
OpBranch %252
%252 = OpLabel
%273 = OpPhi %bool %false %228 %272 %251
OpStore %ok %273
%274 = OpLoad %mat2v3float %m23
%276 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%277 = OpCompositeConstruct %mat2v3float %276 %276
%278 = OpCompositeExtract %v3float %274 0
%279 = OpCompositeExtract %v3float %277 0
%280 = OpFAdd %v3float %278 %279
%281 = OpCompositeExtract %v3float %274 1
%282 = OpCompositeExtract %v3float %277 1
%283 = OpFAdd %v3float %281 %282
%284 = OpCompositeConstruct %mat2v3float %280 %283
OpStore %m23 %284
%285 = OpLoad %bool %ok
OpSelectionMerge %287 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%288 = OpLoad %mat2v3float %m23
%289 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%290 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%291 = OpCompositeConstruct %mat2v3float %289 %290
%292 = OpCompositeExtract %v3float %288 0
%293 = OpCompositeExtract %v3float %291 0
%294 = OpFOrdEqual %v3bool %292 %293
%295 = OpAll %bool %294
%296 = OpCompositeExtract %v3float %288 1
%297 = OpCompositeExtract %v3float %291 1
%298 = OpFOrdEqual %v3bool %296 %297
%299 = OpAll %bool %298
%300 = OpLogicalAnd %bool %295 %299
OpBranch %287
%287 = OpLabel
%301 = OpPhi %bool %false %252 %300 %286
OpStore %ok %301
%302 = OpLoad %mat3v2float %m32
%303 = OpCompositeConstruct %v2float %float_2 %float_2
%304 = OpCompositeConstruct %mat3v2float %303 %303 %303
%305 = OpCompositeExtract %v2float %302 0
%306 = OpCompositeExtract %v2float %304 0
%307 = OpFSub %v2float %305 %306
%308 = OpCompositeExtract %v2float %302 1
%309 = OpCompositeExtract %v2float %304 1
%310 = OpFSub %v2float %308 %309
%311 = OpCompositeExtract %v2float %302 2
%312 = OpCompositeExtract %v2float %304 2
%313 = OpFSub %v2float %311 %312
%314 = OpCompositeConstruct %mat3v2float %307 %310 %313
OpStore %m32 %314
%315 = OpLoad %bool %ok
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%318 = OpLoad %mat3v2float %m32
%320 = OpCompositeConstruct %v2float %float_2 %float_n2
%321 = OpCompositeConstruct %v2float %float_n2 %float_2
%322 = OpCompositeConstruct %v2float %float_n2 %float_n2
%323 = OpCompositeConstruct %mat3v2float %320 %321 %322
%324 = OpCompositeExtract %v2float %318 0
%325 = OpCompositeExtract %v2float %323 0
%326 = OpFOrdEqual %v2bool %324 %325
%327 = OpAll %bool %326
%328 = OpCompositeExtract %v2float %318 1
%329 = OpCompositeExtract %v2float %323 1
%330 = OpFOrdEqual %v2bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
%333 = OpCompositeExtract %v2float %318 2
%334 = OpCompositeExtract %v2float %323 2
%335 = OpFOrdEqual %v2bool %333 %334
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %332 %336
OpBranch %317
%317 = OpLabel
%338 = OpPhi %bool %false %287 %337 %316
OpStore %ok %338
%339 = OpLoad %mat2v4float %m24
%340 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%341 = OpCompositeConstruct %mat2v4float %340 %340
%342 = OpCompositeExtract %v4float %339 0
%343 = OpCompositeExtract %v4float %341 0
%344 = OpFDiv %v4float %342 %343
%345 = OpCompositeExtract %v4float %339 1
%346 = OpCompositeExtract %v4float %341 1
%347 = OpFDiv %v4float %345 %346
%348 = OpCompositeConstruct %mat2v4float %344 %347
OpStore %m24 %348
%349 = OpLoad %bool %ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpLoad %mat2v4float %m24
%354 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%355 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%356 = OpCompositeConstruct %mat2v4float %354 %355
%357 = OpCompositeExtract %v4float %352 0
%358 = OpCompositeExtract %v4float %356 0
%359 = OpFOrdEqual %v4bool %357 %358
%360 = OpAll %bool %359
%361 = OpCompositeExtract %v4float %352 1
%362 = OpCompositeExtract %v4float %356 1
%363 = OpFOrdEqual %v4bool %361 %362
%364 = OpAll %bool %363
%365 = OpLogicalAnd %bool %360 %364
OpBranch %351
%351 = OpLabel
%366 = OpPhi %bool %false %317 %365 %350
OpStore %ok %366
%367 = OpLoad %bool %ok
OpReturnValue %367
OpFunctionEnd
%main = OpFunction %v4float None %368
%369 = OpFunctionParameter %_ptr_Function_v2float
%370 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%683 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%374 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%375 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%373 = OpCompositeConstruct %mat2v3float %374 %375
OpStore %_1_m23 %373
%376 = OpLoad %bool %_0_ok
OpSelectionMerge %378 None
OpBranchConditional %376 %377 %378
%377 = OpLabel
%379 = OpLoad %mat2v3float %_1_m23
%380 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%381 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%382 = OpCompositeConstruct %mat2v3float %380 %381
%383 = OpCompositeExtract %v3float %379 0
%384 = OpCompositeExtract %v3float %382 0
%385 = OpFOrdEqual %v3bool %383 %384
%386 = OpAll %bool %385
%387 = OpCompositeExtract %v3float %379 1
%388 = OpCompositeExtract %v3float %382 1
%389 = OpFOrdEqual %v3bool %387 %388
%390 = OpAll %bool %389
%391 = OpLogicalAnd %bool %386 %390
OpBranch %378
%378 = OpLabel
%392 = OpPhi %bool %false %370 %391 %377
OpStore %_0_ok %392
%395 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%396 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%394 = OpCompositeConstruct %mat2v4float %395 %396
OpStore %_2_m24 %394
%397 = OpLoad %bool %_0_ok
OpSelectionMerge %399 None
OpBranchConditional %397 %398 %399
%398 = OpLabel
%400 = OpLoad %mat2v4float %_2_m24
%401 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%402 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%403 = OpCompositeConstruct %mat2v4float %401 %402
%404 = OpCompositeExtract %v4float %400 0
%405 = OpCompositeExtract %v4float %403 0
%406 = OpFOrdEqual %v4bool %404 %405
%407 = OpAll %bool %406
%408 = OpCompositeExtract %v4float %400 1
%409 = OpCompositeExtract %v4float %403 1
%410 = OpFOrdEqual %v4bool %408 %409
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %407 %411
OpBranch %399
%399 = OpLabel
%413 = OpPhi %bool %false %378 %412 %398
OpStore %_0_ok %413
%416 = OpCompositeConstruct %v2float %float_4 %float_0
%417 = OpCompositeConstruct %v2float %float_0 %float_4
%418 = OpCompositeConstruct %v2float %float_0 %float_0
%415 = OpCompositeConstruct %mat3v2float %416 %417 %418
OpStore %_3_m32 %415
%419 = OpLoad %bool %_0_ok
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%422 = OpLoad %mat3v2float %_3_m32
%423 = OpCompositeConstruct %v2float %float_4 %float_0
%424 = OpCompositeConstruct %v2float %float_0 %float_4
%425 = OpCompositeConstruct %v2float %float_0 %float_0
%426 = OpCompositeConstruct %mat3v2float %423 %424 %425
%427 = OpCompositeExtract %v2float %422 0
%428 = OpCompositeExtract %v2float %426 0
%429 = OpFOrdEqual %v2bool %427 %428
%430 = OpAll %bool %429
%431 = OpCompositeExtract %v2float %422 1
%432 = OpCompositeExtract %v2float %426 1
%433 = OpFOrdEqual %v2bool %431 %432
%434 = OpAll %bool %433
%435 = OpLogicalAnd %bool %430 %434
%436 = OpCompositeExtract %v2float %422 2
%437 = OpCompositeExtract %v2float %426 2
%438 = OpFOrdEqual %v2bool %436 %437
%439 = OpAll %bool %438
%440 = OpLogicalAnd %bool %435 %439
OpBranch %421
%421 = OpLabel
%441 = OpPhi %bool %false %399 %440 %420
OpStore %_0_ok %441
%444 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%445 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%446 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%443 = OpCompositeConstruct %mat3v4float %444 %445 %446
OpStore %_4_m34 %443
%447 = OpLoad %bool %_0_ok
OpSelectionMerge %449 None
OpBranchConditional %447 %448 %449
%448 = OpLabel
%450 = OpLoad %mat3v4float %_4_m34
%451 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%452 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%453 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%454 = OpCompositeConstruct %mat3v4float %451 %452 %453
%455 = OpCompositeExtract %v4float %450 0
%456 = OpCompositeExtract %v4float %454 0
%457 = OpFOrdEqual %v4bool %455 %456
%458 = OpAll %bool %457
%459 = OpCompositeExtract %v4float %450 1
%460 = OpCompositeExtract %v4float %454 1
%461 = OpFOrdEqual %v4bool %459 %460
%462 = OpAll %bool %461
%463 = OpLogicalAnd %bool %458 %462
%464 = OpCompositeExtract %v4float %450 2
%465 = OpCompositeExtract %v4float %454 2
%466 = OpFOrdEqual %v4bool %464 %465
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %463 %467
OpBranch %449
%449 = OpLabel
%469 = OpPhi %bool %false %421 %468 %448
OpStore %_0_ok %469
%472 = OpCompositeConstruct %v2float %float_6 %float_0
%473 = OpCompositeConstruct %v2float %float_0 %float_6
%474 = OpCompositeConstruct %v2float %float_0 %float_0
%475 = OpCompositeConstruct %v2float %float_0 %float_0
%471 = OpCompositeConstruct %mat4v2float %472 %473 %474 %475
OpStore %_5_m42 %471
%476 = OpLoad %bool %_0_ok
OpSelectionMerge %478 None
OpBranchConditional %476 %477 %478
%477 = OpLabel
%479 = OpLoad %mat4v2float %_5_m42
%480 = OpCompositeConstruct %v2float %float_6 %float_0
%481 = OpCompositeConstruct %v2float %float_0 %float_6
%482 = OpCompositeConstruct %v2float %float_0 %float_0
%483 = OpCompositeConstruct %v2float %float_0 %float_0
%484 = OpCompositeConstruct %mat4v2float %480 %481 %482 %483
%485 = OpCompositeExtract %v2float %479 0
%486 = OpCompositeExtract %v2float %484 0
%487 = OpFOrdEqual %v2bool %485 %486
%488 = OpAll %bool %487
%489 = OpCompositeExtract %v2float %479 1
%490 = OpCompositeExtract %v2float %484 1
%491 = OpFOrdEqual %v2bool %489 %490
%492 = OpAll %bool %491
%493 = OpLogicalAnd %bool %488 %492
%494 = OpCompositeExtract %v2float %479 2
%495 = OpCompositeExtract %v2float %484 2
%496 = OpFOrdEqual %v2bool %494 %495
%497 = OpAll %bool %496
%498 = OpLogicalAnd %bool %493 %497
%499 = OpCompositeExtract %v2float %479 3
%500 = OpCompositeExtract %v2float %484 3
%501 = OpFOrdEqual %v2bool %499 %500
%502 = OpAll %bool %501
%503 = OpLogicalAnd %bool %498 %502
OpBranch %478
%478 = OpLabel
%504 = OpPhi %bool %false %449 %503 %477
OpStore %_0_ok %504
%507 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%508 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%509 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%510 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%506 = OpCompositeConstruct %mat4v3float %507 %508 %509 %510
OpStore %_6_m43 %506
%511 = OpLoad %bool %_0_ok
OpSelectionMerge %513 None
OpBranchConditional %511 %512 %513
%512 = OpLabel
%514 = OpLoad %mat4v3float %_6_m43
%515 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%516 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%517 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%518 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%519 = OpCompositeConstruct %mat4v3float %515 %516 %517 %518
%520 = OpCompositeExtract %v3float %514 0
%521 = OpCompositeExtract %v3float %519 0
%522 = OpFOrdEqual %v3bool %520 %521
%523 = OpAll %bool %522
%524 = OpCompositeExtract %v3float %514 1
%525 = OpCompositeExtract %v3float %519 1
%526 = OpFOrdEqual %v3bool %524 %525
%527 = OpAll %bool %526
%528 = OpLogicalAnd %bool %523 %527
%529 = OpCompositeExtract %v3float %514 2
%530 = OpCompositeExtract %v3float %519 2
%531 = OpFOrdEqual %v3bool %529 %530
%532 = OpAll %bool %531
%533 = OpLogicalAnd %bool %528 %532
%534 = OpCompositeExtract %v3float %514 3
%535 = OpCompositeExtract %v3float %519 3
%536 = OpFOrdEqual %v3bool %534 %535
%537 = OpAll %bool %536
%538 = OpLogicalAnd %bool %533 %537
OpBranch %513
%513 = OpLabel
%539 = OpPhi %bool %false %478 %538 %512
OpStore %_0_ok %539
%541 = OpLoad %mat3v2float %_3_m32
%542 = OpLoad %mat2v3float %_1_m23
%543 = OpMatrixTimesMatrix %mat2v2float %541 %542
OpStore %_7_m22 %543
%544 = OpLoad %bool %_0_ok
OpSelectionMerge %546 None
OpBranchConditional %544 %545 %546
%545 = OpLabel
%547 = OpLoad %mat2v2float %_7_m22
%549 = OpCompositeConstruct %v2float %float_8 %float_0
%550 = OpCompositeConstruct %v2float %float_0 %float_8
%548 = OpCompositeConstruct %mat2v2float %549 %550
%551 = OpCompositeExtract %v2float %547 0
%552 = OpCompositeExtract %v2float %548 0
%553 = OpFOrdEqual %v2bool %551 %552
%554 = OpAll %bool %553
%555 = OpCompositeExtract %v2float %547 1
%556 = OpCompositeExtract %v2float %548 1
%557 = OpFOrdEqual %v2bool %555 %556
%558 = OpAll %bool %557
%559 = OpLogicalAnd %bool %554 %558
OpBranch %546
%546 = OpLabel
%560 = OpPhi %bool %false %513 %559 %545
OpStore %_0_ok %560
%562 = OpLoad %mat4v3float %_6_m43
%563 = OpLoad %mat3v4float %_4_m34
%564 = OpMatrixTimesMatrix %mat3v3float %562 %563
OpStore %_8_m33 %564
%565 = OpLoad %bool %_0_ok
OpSelectionMerge %567 None
OpBranchConditional %565 %566 %567
%566 = OpLabel
%568 = OpLoad %mat3v3float %_8_m33
%570 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%571 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%572 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%569 = OpCompositeConstruct %mat3v3float %570 %571 %572
%573 = OpCompositeExtract %v3float %568 0
%574 = OpCompositeExtract %v3float %569 0
%575 = OpFOrdEqual %v3bool %573 %574
%576 = OpAll %bool %575
%577 = OpCompositeExtract %v3float %568 1
%578 = OpCompositeExtract %v3float %569 1
%579 = OpFOrdEqual %v3bool %577 %578
%580 = OpAll %bool %579
%581 = OpLogicalAnd %bool %576 %580
%582 = OpCompositeExtract %v3float %568 2
%583 = OpCompositeExtract %v3float %569 2
%584 = OpFOrdEqual %v3bool %582 %583
%585 = OpAll %bool %584
%586 = OpLogicalAnd %bool %581 %585
OpBranch %567
%567 = OpLabel
%587 = OpPhi %bool %false %546 %586 %566
OpStore %_0_ok %587
%588 = OpLoad %mat2v3float %_1_m23
%589 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%590 = OpCompositeConstruct %mat2v3float %589 %589
%591 = OpCompositeExtract %v3float %588 0
%592 = OpCompositeExtract %v3float %590 0
%593 = OpFAdd %v3float %591 %592
%594 = OpCompositeExtract %v3float %588 1
%595 = OpCompositeExtract %v3float %590 1
%596 = OpFAdd %v3float %594 %595
%597 = OpCompositeConstruct %mat2v3float %593 %596
OpStore %_1_m23 %597
%598 = OpLoad %bool %_0_ok
OpSelectionMerge %600 None
OpBranchConditional %598 %599 %600
%599 = OpLabel
%601 = OpLoad %mat2v3float %_1_m23
%602 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%603 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%604 = OpCompositeConstruct %mat2v3float %602 %603
%605 = OpCompositeExtract %v3float %601 0
%606 = OpCompositeExtract %v3float %604 0
%607 = OpFOrdEqual %v3bool %605 %606
%608 = OpAll %bool %607
%609 = OpCompositeExtract %v3float %601 1
%610 = OpCompositeExtract %v3float %604 1
%611 = OpFOrdEqual %v3bool %609 %610
%612 = OpAll %bool %611
%613 = OpLogicalAnd %bool %608 %612
OpBranch %600
%600 = OpLabel
%614 = OpPhi %bool %false %567 %613 %599
OpStore %_0_ok %614
%615 = OpLoad %mat3v2float %_3_m32
%616 = OpCompositeConstruct %v2float %float_2 %float_2
%617 = OpCompositeConstruct %mat3v2float %616 %616 %616
%618 = OpCompositeExtract %v2float %615 0
%619 = OpCompositeExtract %v2float %617 0
%620 = OpFSub %v2float %618 %619
%621 = OpCompositeExtract %v2float %615 1
%622 = OpCompositeExtract %v2float %617 1
%623 = OpFSub %v2float %621 %622
%624 = OpCompositeExtract %v2float %615 2
%625 = OpCompositeExtract %v2float %617 2
%626 = OpFSub %v2float %624 %625
%627 = OpCompositeConstruct %mat3v2float %620 %623 %626
OpStore %_3_m32 %627
%628 = OpLoad %bool %_0_ok
OpSelectionMerge %630 None
OpBranchConditional %628 %629 %630
%629 = OpLabel
%631 = OpLoad %mat3v2float %_3_m32
%632 = OpCompositeConstruct %v2float %float_2 %float_n2
%633 = OpCompositeConstruct %v2float %float_n2 %float_2
%634 = OpCompositeConstruct %v2float %float_n2 %float_n2
%635 = OpCompositeConstruct %mat3v2float %632 %633 %634
%636 = OpCompositeExtract %v2float %631 0
%637 = OpCompositeExtract %v2float %635 0
%638 = OpFOrdEqual %v2bool %636 %637
%639 = OpAll %bool %638
%640 = OpCompositeExtract %v2float %631 1
%641 = OpCompositeExtract %v2float %635 1
%642 = OpFOrdEqual %v2bool %640 %641
%643 = OpAll %bool %642
%644 = OpLogicalAnd %bool %639 %643
%645 = OpCompositeExtract %v2float %631 2
%646 = OpCompositeExtract %v2float %635 2
%647 = OpFOrdEqual %v2bool %645 %646
%648 = OpAll %bool %647
%649 = OpLogicalAnd %bool %644 %648
OpBranch %630
%630 = OpLabel
%650 = OpPhi %bool %false %600 %649 %629
OpStore %_0_ok %650
%651 = OpLoad %mat2v4float %_2_m24
%652 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%653 = OpCompositeConstruct %mat2v4float %652 %652
%654 = OpCompositeExtract %v4float %651 0
%655 = OpCompositeExtract %v4float %653 0
%656 = OpFDiv %v4float %654 %655
%657 = OpCompositeExtract %v4float %651 1
%658 = OpCompositeExtract %v4float %653 1
%659 = OpFDiv %v4float %657 %658
%660 = OpCompositeConstruct %mat2v4float %656 %659
OpStore %_2_m24 %660
%661 = OpLoad %bool %_0_ok
OpSelectionMerge %663 None
OpBranchConditional %661 %662 %663
%662 = OpLabel
%664 = OpLoad %mat2v4float %_2_m24
%665 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%666 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%667 = OpCompositeConstruct %mat2v4float %665 %666
%668 = OpCompositeExtract %v4float %664 0
%669 = OpCompositeExtract %v4float %667 0
%670 = OpFOrdEqual %v4bool %668 %669
%671 = OpAll %bool %670
%672 = OpCompositeExtract %v4float %664 1
%673 = OpCompositeExtract %v4float %667 1
%674 = OpFOrdEqual %v4bool %672 %673
%675 = OpAll %bool %674
%676 = OpLogicalAnd %bool %671 %675
OpBranch %663
%663 = OpLabel
%677 = OpPhi %bool %false %630 %676 %662
OpStore %_0_ok %677
%678 = OpLoad %bool %_0_ok
OpSelectionMerge %680 None
OpBranchConditional %678 %679 %680
%679 = OpLabel
%681 = OpFunctionCall %bool %test_half_b
OpBranch %680
%680 = OpLabel
%682 = OpPhi %bool %false %663 %681 %679
OpSelectionMerge %687 None
OpBranchConditional %682 %685 %686
%685 = OpLabel
%688 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%692 = OpLoad %v4float %688
OpStore %683 %692
OpBranch %687
%686 = OpLabel
%693 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%695 = OpLoad %v4float %693
OpStore %683 %695
OpBranch %687
%687 = OpLabel
%696 = OpLoad %v4float %683
OpReturnValue %696
OpFunctionEnd
