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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
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
OpName %h4 "h4"
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
OpName %_10_f4 "_10_f4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
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
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
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
OpDecorate %h4 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %559 RelaxedPrecision
OpDecorate %560 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %602 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %654 RelaxedPrecision
OpDecorate %682 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %833 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %921 RelaxedPrecision
OpDecorate %947 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1156 RelaxedPrecision
OpDecorate %1159 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
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
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%603 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%36 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%35 = OpCompositeConstruct %mat2v3float %36 %37
OpStore %m23 %35
%39 = OpLoad %bool %ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %mat2v3float %m23
%43 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%44 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%45 = OpCompositeConstruct %mat2v3float %43 %44
%47 = OpCompositeExtract %v3float %42 0
%48 = OpCompositeExtract %v3float %45 0
%49 = OpFOrdEqual %v3bool %47 %48
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v3float %42 1
%52 = OpCompositeExtract %v3float %45 1
%53 = OpFOrdEqual %v3bool %51 %52
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %50 %54
OpBranch %41
%41 = OpLabel
%56 = OpPhi %bool %false %26 %55 %40
OpStore %ok %56
%62 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%63 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%61 = OpCompositeConstruct %mat2v4float %62 %63
OpStore %m24 %61
%64 = OpLoad %bool %ok
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpLoad %mat2v4float %m24
%68 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%70 = OpCompositeConstruct %mat2v4float %68 %69
%72 = OpCompositeExtract %v4float %67 0
%73 = OpCompositeExtract %v4float %70 0
%74 = OpFOrdEqual %v4bool %72 %73
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v4float %67 1
%77 = OpCompositeExtract %v4float %70 1
%78 = OpFOrdEqual %v4bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
OpBranch %66
%66 = OpLabel
%81 = OpPhi %bool %false %41 %80 %65
OpStore %ok %81
%87 = OpCompositeConstruct %v2float %float_4 %float_0
%88 = OpCompositeConstruct %v2float %float_0 %float_4
%89 = OpCompositeConstruct %v2float %float_0 %float_0
%86 = OpCompositeConstruct %mat3v2float %87 %88 %89
OpStore %m32 %86
%90 = OpLoad %bool %ok
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %mat3v2float %m32
%94 = OpCompositeConstruct %v2float %float_4 %float_0
%95 = OpCompositeConstruct %v2float %float_0 %float_4
%96 = OpCompositeConstruct %v2float %float_0 %float_0
%97 = OpCompositeConstruct %mat3v2float %94 %95 %96
%99 = OpCompositeExtract %v2float %93 0
%100 = OpCompositeExtract %v2float %97 0
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v2float %93 1
%104 = OpCompositeExtract %v2float %97 1
%105 = OpFOrdEqual %v2bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
%108 = OpCompositeExtract %v2float %93 2
%109 = OpCompositeExtract %v2float %97 2
%110 = OpFOrdEqual %v2bool %108 %109
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %107 %111
OpBranch %92
%92 = OpLabel
%113 = OpPhi %bool %false %66 %112 %91
OpStore %ok %113
%119 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%121 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%118 = OpCompositeConstruct %mat3v4float %119 %120 %121
OpStore %m34 %118
%122 = OpLoad %bool %ok
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %mat3v4float %m34
%126 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%129 = OpCompositeConstruct %mat3v4float %126 %127 %128
%130 = OpCompositeExtract %v4float %125 0
%131 = OpCompositeExtract %v4float %129 0
%132 = OpFOrdEqual %v4bool %130 %131
%133 = OpAll %bool %132
%134 = OpCompositeExtract %v4float %125 1
%135 = OpCompositeExtract %v4float %129 1
%136 = OpFOrdEqual %v4bool %134 %135
%137 = OpAll %bool %136
%138 = OpLogicalAnd %bool %133 %137
%139 = OpCompositeExtract %v4float %125 2
%140 = OpCompositeExtract %v4float %129 2
%141 = OpFOrdEqual %v4bool %139 %140
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %138 %142
OpBranch %124
%124 = OpLabel
%144 = OpPhi %bool %false %92 %143 %123
OpStore %ok %144
%150 = OpCompositeConstruct %v2float %float_6 %float_0
%151 = OpCompositeConstruct %v2float %float_0 %float_6
%152 = OpCompositeConstruct %v2float %float_0 %float_0
%153 = OpCompositeConstruct %v2float %float_0 %float_0
%149 = OpCompositeConstruct %mat4v2float %150 %151 %152 %153
OpStore %m42 %149
%154 = OpLoad %bool %ok
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %mat4v2float %m42
%158 = OpCompositeConstruct %v2float %float_6 %float_0
%159 = OpCompositeConstruct %v2float %float_0 %float_6
%160 = OpCompositeConstruct %v2float %float_0 %float_0
%161 = OpCompositeConstruct %v2float %float_0 %float_0
%162 = OpCompositeConstruct %mat4v2float %158 %159 %160 %161
%163 = OpCompositeExtract %v2float %157 0
%164 = OpCompositeExtract %v2float %162 0
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpCompositeExtract %v2float %157 1
%168 = OpCompositeExtract %v2float %162 1
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %166 %170
%172 = OpCompositeExtract %v2float %157 2
%173 = OpCompositeExtract %v2float %162 2
%174 = OpFOrdEqual %v2bool %172 %173
%175 = OpAll %bool %174
%176 = OpLogicalAnd %bool %171 %175
%177 = OpCompositeExtract %v2float %157 3
%178 = OpCompositeExtract %v2float %162 3
%179 = OpFOrdEqual %v2bool %177 %178
%180 = OpAll %bool %179
%181 = OpLogicalAnd %bool %176 %180
OpBranch %156
%156 = OpLabel
%182 = OpPhi %bool %false %124 %181 %155
OpStore %ok %182
%188 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%191 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%187 = OpCompositeConstruct %mat4v3float %188 %189 %190 %191
OpStore %m43 %187
%192 = OpLoad %bool %ok
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %mat4v3float %m43
%196 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%199 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%200 = OpCompositeConstruct %mat4v3float %196 %197 %198 %199
%201 = OpCompositeExtract %v3float %195 0
%202 = OpCompositeExtract %v3float %200 0
%203 = OpFOrdEqual %v3bool %201 %202
%204 = OpAll %bool %203
%205 = OpCompositeExtract %v3float %195 1
%206 = OpCompositeExtract %v3float %200 1
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
%210 = OpCompositeExtract %v3float %195 2
%211 = OpCompositeExtract %v3float %200 2
%212 = OpFOrdEqual %v3bool %210 %211
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %209 %213
%215 = OpCompositeExtract %v3float %195 3
%216 = OpCompositeExtract %v3float %200 3
%217 = OpFOrdEqual %v3bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
OpBranch %194
%194 = OpLabel
%220 = OpPhi %bool %false %156 %219 %193
OpStore %ok %220
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
%243 = OpPhi %bool %false %194 %242 %227
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
%369 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%373 = OpLoad %mat2v2float %369
%374 = OpCompositeExtract %float %373 0 0
%375 = OpCompositeExtract %float %373 0 1
%376 = OpCompositeExtract %float %373 1 0
%377 = OpCompositeExtract %float %373 1 1
%378 = OpCompositeConstruct %v4float %374 %375 %376 %377
OpStore %h4 %378
%379 = OpLoad %bool %ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %v4float %h4
%383 = OpVectorShuffle %v3float %382 %382 0 1 2
%384 = OpLoad %v4float %h4
%385 = OpCompositeExtract %float %384 3
%386 = OpLoad %v4float %h4
%387 = OpVectorShuffle %v2float %386 %386 0 1
%388 = OpCompositeExtract %float %387 0
%389 = OpCompositeExtract %float %387 1
%390 = OpCompositeConstruct %v3float %385 %388 %389
%391 = OpCompositeConstruct %mat2v3float %383 %390
%392 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%393 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%394 = OpCompositeConstruct %mat2v3float %392 %393
%395 = OpCompositeExtract %v3float %391 0
%396 = OpCompositeExtract %v3float %394 0
%397 = OpFOrdEqual %v3bool %395 %396
%398 = OpAll %bool %397
%399 = OpCompositeExtract %v3float %391 1
%400 = OpCompositeExtract %v3float %394 1
%401 = OpFOrdEqual %v3bool %399 %400
%402 = OpAll %bool %401
%403 = OpLogicalAnd %bool %398 %402
OpBranch %381
%381 = OpLabel
%404 = OpPhi %bool %false %351 %403 %380
OpStore %ok %404
%405 = OpLoad %bool %ok
OpSelectionMerge %407 None
OpBranchConditional %405 %406 %407
%406 = OpLabel
%408 = OpLoad %v4float %h4
%409 = OpVectorShuffle %v3float %408 %408 0 1 2
%410 = OpLoad %v4float %h4
%411 = OpCompositeExtract %float %410 3
%412 = OpLoad %v4float %h4
%413 = OpCompositeExtract %float %412 0
%414 = OpLoad %v4float %h4
%415 = OpVectorShuffle %v3float %414 %414 1 2 3
%416 = OpCompositeExtract %float %409 0
%417 = OpCompositeExtract %float %409 1
%418 = OpCompositeExtract %float %409 2
%419 = OpCompositeConstruct %v4float %416 %417 %418 %411
%420 = OpCompositeExtract %float %415 0
%421 = OpCompositeExtract %float %415 1
%422 = OpCompositeExtract %float %415 2
%423 = OpCompositeConstruct %v4float %413 %420 %421 %422
%424 = OpCompositeConstruct %mat2v4float %419 %423
%425 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%426 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%427 = OpCompositeConstruct %mat2v4float %425 %426
%428 = OpCompositeExtract %v4float %424 0
%429 = OpCompositeExtract %v4float %427 0
%430 = OpFOrdEqual %v4bool %428 %429
%431 = OpAll %bool %430
%432 = OpCompositeExtract %v4float %424 1
%433 = OpCompositeExtract %v4float %427 1
%434 = OpFOrdEqual %v4bool %432 %433
%435 = OpAll %bool %434
%436 = OpLogicalAnd %bool %431 %435
OpBranch %407
%407 = OpLabel
%437 = OpPhi %bool %false %381 %436 %406
OpStore %ok %437
%438 = OpLoad %bool %ok
OpSelectionMerge %440 None
OpBranchConditional %438 %439 %440
%439 = OpLabel
%441 = OpLoad %v4float %h4
%442 = OpVectorShuffle %v2float %441 %441 0 1
%443 = OpLoad %v4float %h4
%444 = OpVectorShuffle %v2float %443 %443 2 3
%445 = OpLoad %v4float %h4
%446 = OpCompositeExtract %float %445 0
%447 = OpLoad %v4float %h4
%448 = OpCompositeExtract %float %447 1
%449 = OpCompositeConstruct %v2float %446 %448
%450 = OpCompositeConstruct %mat3v2float %442 %444 %449
%451 = OpCompositeConstruct %v2float %float_1 %float_2
%452 = OpCompositeConstruct %v2float %float_3 %float_4
%453 = OpCompositeConstruct %v2float %float_1 %float_2
%454 = OpCompositeConstruct %mat3v2float %451 %452 %453
%455 = OpCompositeExtract %v2float %450 0
%456 = OpCompositeExtract %v2float %454 0
%457 = OpFOrdEqual %v2bool %455 %456
%458 = OpAll %bool %457
%459 = OpCompositeExtract %v2float %450 1
%460 = OpCompositeExtract %v2float %454 1
%461 = OpFOrdEqual %v2bool %459 %460
%462 = OpAll %bool %461
%463 = OpLogicalAnd %bool %458 %462
%464 = OpCompositeExtract %v2float %450 2
%465 = OpCompositeExtract %v2float %454 2
%466 = OpFOrdEqual %v2bool %464 %465
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %463 %467
OpBranch %440
%440 = OpLabel
%469 = OpPhi %bool %false %407 %468 %439
OpStore %ok %469
%470 = OpLoad %bool %ok
OpSelectionMerge %472 None
OpBranchConditional %470 %471 %472
%471 = OpLabel
%473 = OpLoad %v4float %h4
%474 = OpVectorShuffle %v2float %473 %473 0 1
%475 = OpLoad %v4float %h4
%476 = OpVectorShuffle %v2float %475 %475 2 3
%477 = OpLoad %v4float %h4
%478 = OpLoad %v4float %h4
%479 = OpCompositeExtract %float %478 0
%480 = OpLoad %v4float %h4
%481 = OpVectorShuffle %v2float %480 %480 1 2
%482 = OpLoad %v4float %h4
%483 = OpCompositeExtract %float %482 3
%484 = OpCompositeExtract %float %474 0
%485 = OpCompositeExtract %float %474 1
%486 = OpCompositeExtract %float %476 0
%487 = OpCompositeExtract %float %476 1
%488 = OpCompositeConstruct %v4float %484 %485 %486 %487
%489 = OpCompositeExtract %float %481 0
%490 = OpCompositeExtract %float %481 1
%491 = OpCompositeConstruct %v4float %479 %489 %490 %483
%492 = OpCompositeConstruct %mat3v4float %488 %477 %491
%493 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%494 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%495 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%496 = OpCompositeConstruct %mat3v4float %493 %494 %495
%497 = OpCompositeExtract %v4float %492 0
%498 = OpCompositeExtract %v4float %496 0
%499 = OpFOrdEqual %v4bool %497 %498
%500 = OpAll %bool %499
%501 = OpCompositeExtract %v4float %492 1
%502 = OpCompositeExtract %v4float %496 1
%503 = OpFOrdEqual %v4bool %501 %502
%504 = OpAll %bool %503
%505 = OpLogicalAnd %bool %500 %504
%506 = OpCompositeExtract %v4float %492 2
%507 = OpCompositeExtract %v4float %496 2
%508 = OpFOrdEqual %v4bool %506 %507
%509 = OpAll %bool %508
%510 = OpLogicalAnd %bool %505 %509
OpBranch %472
%472 = OpLabel
%511 = OpPhi %bool %false %440 %510 %471
OpStore %ok %511
%512 = OpLoad %bool %ok
OpSelectionMerge %514 None
OpBranchConditional %512 %513 %514
%513 = OpLabel
%515 = OpLoad %v4float %h4
%516 = OpVectorShuffle %v2float %515 %515 0 1
%517 = OpLoad %v4float %h4
%518 = OpCompositeExtract %float %517 2
%519 = OpLoad %v4float %h4
%520 = OpCompositeExtract %float %519 3
%521 = OpLoad %v4float %h4
%522 = OpVectorShuffle %v2float %521 %521 0 1
%523 = OpLoad %v4float %h4
%524 = OpCompositeExtract %float %523 2
%525 = OpLoad %v4float %h4
%526 = OpCompositeExtract %float %525 3
%527 = OpCompositeConstruct %v2float %518 %520
%528 = OpCompositeConstruct %v2float %524 %526
%529 = OpCompositeConstruct %mat4v2float %516 %527 %522 %528
%530 = OpCompositeConstruct %v2float %float_1 %float_2
%531 = OpCompositeConstruct %v2float %float_3 %float_4
%532 = OpCompositeConstruct %v2float %float_1 %float_2
%533 = OpCompositeConstruct %v2float %float_3 %float_4
%534 = OpCompositeConstruct %mat4v2float %530 %531 %532 %533
%535 = OpCompositeExtract %v2float %529 0
%536 = OpCompositeExtract %v2float %534 0
%537 = OpFOrdEqual %v2bool %535 %536
%538 = OpAll %bool %537
%539 = OpCompositeExtract %v2float %529 1
%540 = OpCompositeExtract %v2float %534 1
%541 = OpFOrdEqual %v2bool %539 %540
%542 = OpAll %bool %541
%543 = OpLogicalAnd %bool %538 %542
%544 = OpCompositeExtract %v2float %529 2
%545 = OpCompositeExtract %v2float %534 2
%546 = OpFOrdEqual %v2bool %544 %545
%547 = OpAll %bool %546
%548 = OpLogicalAnd %bool %543 %547
%549 = OpCompositeExtract %v2float %529 3
%550 = OpCompositeExtract %v2float %534 3
%551 = OpFOrdEqual %v2bool %549 %550
%552 = OpAll %bool %551
%553 = OpLogicalAnd %bool %548 %552
OpBranch %514
%514 = OpLabel
%554 = OpPhi %bool %false %472 %553 %513
OpStore %ok %554
%555 = OpLoad %bool %ok
OpSelectionMerge %557 None
OpBranchConditional %555 %556 %557
%556 = OpLabel
%558 = OpLoad %v4float %h4
%559 = OpCompositeExtract %float %558 0
%560 = OpLoad %v4float %h4
%561 = OpVectorShuffle %v2float %560 %560 1 2
%562 = OpLoad %v4float %h4
%563 = OpVectorShuffle %v2float %562 %562 3 0
%564 = OpLoad %v4float %h4
%565 = OpCompositeExtract %float %564 1
%566 = OpLoad %v4float %h4
%567 = OpVectorShuffle %v3float %566 %566 2 3 0
%568 = OpLoad %v4float %h4
%569 = OpVectorShuffle %v3float %568 %568 1 2 3
%570 = OpCompositeExtract %float %561 0
%571 = OpCompositeExtract %float %561 1
%572 = OpCompositeConstruct %v3float %559 %570 %571
%573 = OpCompositeExtract %float %563 0
%574 = OpCompositeExtract %float %563 1
%575 = OpCompositeConstruct %v3float %573 %574 %565
%576 = OpCompositeConstruct %mat4v3float %572 %575 %567 %569
%577 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%578 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%579 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%580 = OpCompositeConstruct %v3float %float_2 %float_3 %float_4
%581 = OpCompositeConstruct %mat4v3float %577 %578 %579 %580
%582 = OpCompositeExtract %v3float %576 0
%583 = OpCompositeExtract %v3float %581 0
%584 = OpFOrdEqual %v3bool %582 %583
%585 = OpAll %bool %584
%586 = OpCompositeExtract %v3float %576 1
%587 = OpCompositeExtract %v3float %581 1
%588 = OpFOrdEqual %v3bool %586 %587
%589 = OpAll %bool %588
%590 = OpLogicalAnd %bool %585 %589
%591 = OpCompositeExtract %v3float %576 2
%592 = OpCompositeExtract %v3float %581 2
%593 = OpFOrdEqual %v3bool %591 %592
%594 = OpAll %bool %593
%595 = OpLogicalAnd %bool %590 %594
%596 = OpCompositeExtract %v3float %576 3
%597 = OpCompositeExtract %v3float %581 3
%598 = OpFOrdEqual %v3bool %596 %597
%599 = OpAll %bool %598
%600 = OpLogicalAnd %bool %595 %599
OpBranch %557
%557 = OpLabel
%601 = OpPhi %bool %false %514 %600 %556
OpStore %ok %601
%602 = OpLoad %bool %ok
OpReturnValue %602
OpFunctionEnd
%main = OpFunction %v4float None %603
%604 = OpFunctionParameter %_ptr_Function_v2float
%605 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_10_f4 = OpVariable %_ptr_Function_v4float Function
%1149 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%609 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%610 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%608 = OpCompositeConstruct %mat2v3float %609 %610
OpStore %_1_m23 %608
%611 = OpLoad %bool %_0_ok
OpSelectionMerge %613 None
OpBranchConditional %611 %612 %613
%612 = OpLabel
%614 = OpLoad %mat2v3float %_1_m23
%615 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%616 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%617 = OpCompositeConstruct %mat2v3float %615 %616
%618 = OpCompositeExtract %v3float %614 0
%619 = OpCompositeExtract %v3float %617 0
%620 = OpFOrdEqual %v3bool %618 %619
%621 = OpAll %bool %620
%622 = OpCompositeExtract %v3float %614 1
%623 = OpCompositeExtract %v3float %617 1
%624 = OpFOrdEqual %v3bool %622 %623
%625 = OpAll %bool %624
%626 = OpLogicalAnd %bool %621 %625
OpBranch %613
%613 = OpLabel
%627 = OpPhi %bool %false %605 %626 %612
OpStore %_0_ok %627
%630 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%631 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%629 = OpCompositeConstruct %mat2v4float %630 %631
OpStore %_2_m24 %629
%632 = OpLoad %bool %_0_ok
OpSelectionMerge %634 None
OpBranchConditional %632 %633 %634
%633 = OpLabel
%635 = OpLoad %mat2v4float %_2_m24
%636 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%637 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%638 = OpCompositeConstruct %mat2v4float %636 %637
%639 = OpCompositeExtract %v4float %635 0
%640 = OpCompositeExtract %v4float %638 0
%641 = OpFOrdEqual %v4bool %639 %640
%642 = OpAll %bool %641
%643 = OpCompositeExtract %v4float %635 1
%644 = OpCompositeExtract %v4float %638 1
%645 = OpFOrdEqual %v4bool %643 %644
%646 = OpAll %bool %645
%647 = OpLogicalAnd %bool %642 %646
OpBranch %634
%634 = OpLabel
%648 = OpPhi %bool %false %613 %647 %633
OpStore %_0_ok %648
%651 = OpCompositeConstruct %v2float %float_4 %float_0
%652 = OpCompositeConstruct %v2float %float_0 %float_4
%653 = OpCompositeConstruct %v2float %float_0 %float_0
%650 = OpCompositeConstruct %mat3v2float %651 %652 %653
OpStore %_3_m32 %650
%654 = OpLoad %bool %_0_ok
OpSelectionMerge %656 None
OpBranchConditional %654 %655 %656
%655 = OpLabel
%657 = OpLoad %mat3v2float %_3_m32
%658 = OpCompositeConstruct %v2float %float_4 %float_0
%659 = OpCompositeConstruct %v2float %float_0 %float_4
%660 = OpCompositeConstruct %v2float %float_0 %float_0
%661 = OpCompositeConstruct %mat3v2float %658 %659 %660
%662 = OpCompositeExtract %v2float %657 0
%663 = OpCompositeExtract %v2float %661 0
%664 = OpFOrdEqual %v2bool %662 %663
%665 = OpAll %bool %664
%666 = OpCompositeExtract %v2float %657 1
%667 = OpCompositeExtract %v2float %661 1
%668 = OpFOrdEqual %v2bool %666 %667
%669 = OpAll %bool %668
%670 = OpLogicalAnd %bool %665 %669
%671 = OpCompositeExtract %v2float %657 2
%672 = OpCompositeExtract %v2float %661 2
%673 = OpFOrdEqual %v2bool %671 %672
%674 = OpAll %bool %673
%675 = OpLogicalAnd %bool %670 %674
OpBranch %656
%656 = OpLabel
%676 = OpPhi %bool %false %634 %675 %655
OpStore %_0_ok %676
%679 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%680 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%681 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%678 = OpCompositeConstruct %mat3v4float %679 %680 %681
OpStore %_4_m34 %678
%682 = OpLoad %bool %_0_ok
OpSelectionMerge %684 None
OpBranchConditional %682 %683 %684
%683 = OpLabel
%685 = OpLoad %mat3v4float %_4_m34
%686 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%687 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%688 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%689 = OpCompositeConstruct %mat3v4float %686 %687 %688
%690 = OpCompositeExtract %v4float %685 0
%691 = OpCompositeExtract %v4float %689 0
%692 = OpFOrdEqual %v4bool %690 %691
%693 = OpAll %bool %692
%694 = OpCompositeExtract %v4float %685 1
%695 = OpCompositeExtract %v4float %689 1
%696 = OpFOrdEqual %v4bool %694 %695
%697 = OpAll %bool %696
%698 = OpLogicalAnd %bool %693 %697
%699 = OpCompositeExtract %v4float %685 2
%700 = OpCompositeExtract %v4float %689 2
%701 = OpFOrdEqual %v4bool %699 %700
%702 = OpAll %bool %701
%703 = OpLogicalAnd %bool %698 %702
OpBranch %684
%684 = OpLabel
%704 = OpPhi %bool %false %656 %703 %683
OpStore %_0_ok %704
%707 = OpCompositeConstruct %v2float %float_6 %float_0
%708 = OpCompositeConstruct %v2float %float_0 %float_6
%709 = OpCompositeConstruct %v2float %float_0 %float_0
%710 = OpCompositeConstruct %v2float %float_0 %float_0
%706 = OpCompositeConstruct %mat4v2float %707 %708 %709 %710
OpStore %_5_m42 %706
%711 = OpLoad %bool %_0_ok
OpSelectionMerge %713 None
OpBranchConditional %711 %712 %713
%712 = OpLabel
%714 = OpLoad %mat4v2float %_5_m42
%715 = OpCompositeConstruct %v2float %float_6 %float_0
%716 = OpCompositeConstruct %v2float %float_0 %float_6
%717 = OpCompositeConstruct %v2float %float_0 %float_0
%718 = OpCompositeConstruct %v2float %float_0 %float_0
%719 = OpCompositeConstruct %mat4v2float %715 %716 %717 %718
%720 = OpCompositeExtract %v2float %714 0
%721 = OpCompositeExtract %v2float %719 0
%722 = OpFOrdEqual %v2bool %720 %721
%723 = OpAll %bool %722
%724 = OpCompositeExtract %v2float %714 1
%725 = OpCompositeExtract %v2float %719 1
%726 = OpFOrdEqual %v2bool %724 %725
%727 = OpAll %bool %726
%728 = OpLogicalAnd %bool %723 %727
%729 = OpCompositeExtract %v2float %714 2
%730 = OpCompositeExtract %v2float %719 2
%731 = OpFOrdEqual %v2bool %729 %730
%732 = OpAll %bool %731
%733 = OpLogicalAnd %bool %728 %732
%734 = OpCompositeExtract %v2float %714 3
%735 = OpCompositeExtract %v2float %719 3
%736 = OpFOrdEqual %v2bool %734 %735
%737 = OpAll %bool %736
%738 = OpLogicalAnd %bool %733 %737
OpBranch %713
%713 = OpLabel
%739 = OpPhi %bool %false %684 %738 %712
OpStore %_0_ok %739
%742 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%743 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%744 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%745 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%741 = OpCompositeConstruct %mat4v3float %742 %743 %744 %745
OpStore %_6_m43 %741
%746 = OpLoad %bool %_0_ok
OpSelectionMerge %748 None
OpBranchConditional %746 %747 %748
%747 = OpLabel
%749 = OpLoad %mat4v3float %_6_m43
%750 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%751 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%752 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%753 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%754 = OpCompositeConstruct %mat4v3float %750 %751 %752 %753
%755 = OpCompositeExtract %v3float %749 0
%756 = OpCompositeExtract %v3float %754 0
%757 = OpFOrdEqual %v3bool %755 %756
%758 = OpAll %bool %757
%759 = OpCompositeExtract %v3float %749 1
%760 = OpCompositeExtract %v3float %754 1
%761 = OpFOrdEqual %v3bool %759 %760
%762 = OpAll %bool %761
%763 = OpLogicalAnd %bool %758 %762
%764 = OpCompositeExtract %v3float %749 2
%765 = OpCompositeExtract %v3float %754 2
%766 = OpFOrdEqual %v3bool %764 %765
%767 = OpAll %bool %766
%768 = OpLogicalAnd %bool %763 %767
%769 = OpCompositeExtract %v3float %749 3
%770 = OpCompositeExtract %v3float %754 3
%771 = OpFOrdEqual %v3bool %769 %770
%772 = OpAll %bool %771
%773 = OpLogicalAnd %bool %768 %772
OpBranch %748
%748 = OpLabel
%774 = OpPhi %bool %false %713 %773 %747
OpStore %_0_ok %774
%776 = OpLoad %mat3v2float %_3_m32
%777 = OpLoad %mat2v3float %_1_m23
%778 = OpMatrixTimesMatrix %mat2v2float %776 %777
OpStore %_7_m22 %778
%779 = OpLoad %bool %_0_ok
OpSelectionMerge %781 None
OpBranchConditional %779 %780 %781
%780 = OpLabel
%782 = OpLoad %mat2v2float %_7_m22
%784 = OpCompositeConstruct %v2float %float_8 %float_0
%785 = OpCompositeConstruct %v2float %float_0 %float_8
%783 = OpCompositeConstruct %mat2v2float %784 %785
%786 = OpCompositeExtract %v2float %782 0
%787 = OpCompositeExtract %v2float %783 0
%788 = OpFOrdEqual %v2bool %786 %787
%789 = OpAll %bool %788
%790 = OpCompositeExtract %v2float %782 1
%791 = OpCompositeExtract %v2float %783 1
%792 = OpFOrdEqual %v2bool %790 %791
%793 = OpAll %bool %792
%794 = OpLogicalAnd %bool %789 %793
OpBranch %781
%781 = OpLabel
%795 = OpPhi %bool %false %748 %794 %780
OpStore %_0_ok %795
%797 = OpLoad %mat4v3float %_6_m43
%798 = OpLoad %mat3v4float %_4_m34
%799 = OpMatrixTimesMatrix %mat3v3float %797 %798
OpStore %_8_m33 %799
%800 = OpLoad %bool %_0_ok
OpSelectionMerge %802 None
OpBranchConditional %800 %801 %802
%801 = OpLabel
%803 = OpLoad %mat3v3float %_8_m33
%805 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%806 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%807 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%804 = OpCompositeConstruct %mat3v3float %805 %806 %807
%808 = OpCompositeExtract %v3float %803 0
%809 = OpCompositeExtract %v3float %804 0
%810 = OpFOrdEqual %v3bool %808 %809
%811 = OpAll %bool %810
%812 = OpCompositeExtract %v3float %803 1
%813 = OpCompositeExtract %v3float %804 1
%814 = OpFOrdEqual %v3bool %812 %813
%815 = OpAll %bool %814
%816 = OpLogicalAnd %bool %811 %815
%817 = OpCompositeExtract %v3float %803 2
%818 = OpCompositeExtract %v3float %804 2
%819 = OpFOrdEqual %v3bool %817 %818
%820 = OpAll %bool %819
%821 = OpLogicalAnd %bool %816 %820
OpBranch %802
%802 = OpLabel
%822 = OpPhi %bool %false %781 %821 %801
OpStore %_0_ok %822
%823 = OpLoad %mat2v3float %_1_m23
%824 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%825 = OpCompositeConstruct %mat2v3float %824 %824
%826 = OpCompositeExtract %v3float %823 0
%827 = OpCompositeExtract %v3float %825 0
%828 = OpFAdd %v3float %826 %827
%829 = OpCompositeExtract %v3float %823 1
%830 = OpCompositeExtract %v3float %825 1
%831 = OpFAdd %v3float %829 %830
%832 = OpCompositeConstruct %mat2v3float %828 %831
OpStore %_1_m23 %832
%833 = OpLoad %bool %_0_ok
OpSelectionMerge %835 None
OpBranchConditional %833 %834 %835
%834 = OpLabel
%836 = OpLoad %mat2v3float %_1_m23
%837 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%838 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%839 = OpCompositeConstruct %mat2v3float %837 %838
%840 = OpCompositeExtract %v3float %836 0
%841 = OpCompositeExtract %v3float %839 0
%842 = OpFOrdEqual %v3bool %840 %841
%843 = OpAll %bool %842
%844 = OpCompositeExtract %v3float %836 1
%845 = OpCompositeExtract %v3float %839 1
%846 = OpFOrdEqual %v3bool %844 %845
%847 = OpAll %bool %846
%848 = OpLogicalAnd %bool %843 %847
OpBranch %835
%835 = OpLabel
%849 = OpPhi %bool %false %802 %848 %834
OpStore %_0_ok %849
%850 = OpLoad %mat3v2float %_3_m32
%851 = OpCompositeConstruct %v2float %float_2 %float_2
%852 = OpCompositeConstruct %mat3v2float %851 %851 %851
%853 = OpCompositeExtract %v2float %850 0
%854 = OpCompositeExtract %v2float %852 0
%855 = OpFSub %v2float %853 %854
%856 = OpCompositeExtract %v2float %850 1
%857 = OpCompositeExtract %v2float %852 1
%858 = OpFSub %v2float %856 %857
%859 = OpCompositeExtract %v2float %850 2
%860 = OpCompositeExtract %v2float %852 2
%861 = OpFSub %v2float %859 %860
%862 = OpCompositeConstruct %mat3v2float %855 %858 %861
OpStore %_3_m32 %862
%863 = OpLoad %bool %_0_ok
OpSelectionMerge %865 None
OpBranchConditional %863 %864 %865
%864 = OpLabel
%866 = OpLoad %mat3v2float %_3_m32
%867 = OpCompositeConstruct %v2float %float_2 %float_n2
%868 = OpCompositeConstruct %v2float %float_n2 %float_2
%869 = OpCompositeConstruct %v2float %float_n2 %float_n2
%870 = OpCompositeConstruct %mat3v2float %867 %868 %869
%871 = OpCompositeExtract %v2float %866 0
%872 = OpCompositeExtract %v2float %870 0
%873 = OpFOrdEqual %v2bool %871 %872
%874 = OpAll %bool %873
%875 = OpCompositeExtract %v2float %866 1
%876 = OpCompositeExtract %v2float %870 1
%877 = OpFOrdEqual %v2bool %875 %876
%878 = OpAll %bool %877
%879 = OpLogicalAnd %bool %874 %878
%880 = OpCompositeExtract %v2float %866 2
%881 = OpCompositeExtract %v2float %870 2
%882 = OpFOrdEqual %v2bool %880 %881
%883 = OpAll %bool %882
%884 = OpLogicalAnd %bool %879 %883
OpBranch %865
%865 = OpLabel
%885 = OpPhi %bool %false %835 %884 %864
OpStore %_0_ok %885
%886 = OpLoad %mat2v4float %_2_m24
%887 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%888 = OpCompositeConstruct %mat2v4float %887 %887
%889 = OpCompositeExtract %v4float %886 0
%890 = OpCompositeExtract %v4float %888 0
%891 = OpFDiv %v4float %889 %890
%892 = OpCompositeExtract %v4float %886 1
%893 = OpCompositeExtract %v4float %888 1
%894 = OpFDiv %v4float %892 %893
%895 = OpCompositeConstruct %mat2v4float %891 %894
OpStore %_2_m24 %895
%896 = OpLoad %bool %_0_ok
OpSelectionMerge %898 None
OpBranchConditional %896 %897 %898
%897 = OpLabel
%899 = OpLoad %mat2v4float %_2_m24
%900 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%901 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%902 = OpCompositeConstruct %mat2v4float %900 %901
%903 = OpCompositeExtract %v4float %899 0
%904 = OpCompositeExtract %v4float %902 0
%905 = OpFOrdEqual %v4bool %903 %904
%906 = OpAll %bool %905
%907 = OpCompositeExtract %v4float %899 1
%908 = OpCompositeExtract %v4float %902 1
%909 = OpFOrdEqual %v4bool %907 %908
%910 = OpAll %bool %909
%911 = OpLogicalAnd %bool %906 %910
OpBranch %898
%898 = OpLabel
%912 = OpPhi %bool %false %865 %911 %897
OpStore %_0_ok %912
%914 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%915 = OpLoad %mat2v2float %914
%916 = OpCompositeExtract %float %915 0 0
%917 = OpCompositeExtract %float %915 0 1
%918 = OpCompositeExtract %float %915 1 0
%919 = OpCompositeExtract %float %915 1 1
%920 = OpCompositeConstruct %v4float %916 %917 %918 %919
OpStore %_10_f4 %920
%921 = OpLoad %bool %_0_ok
OpSelectionMerge %923 None
OpBranchConditional %921 %922 %923
%922 = OpLabel
%924 = OpLoad %v4float %_10_f4
%925 = OpVectorShuffle %v3float %924 %924 0 1 2
%926 = OpLoad %v4float %_10_f4
%927 = OpCompositeExtract %float %926 3
%928 = OpLoad %v4float %_10_f4
%929 = OpVectorShuffle %v2float %928 %928 0 1
%930 = OpCompositeExtract %float %929 0
%931 = OpCompositeExtract %float %929 1
%932 = OpCompositeConstruct %v3float %927 %930 %931
%933 = OpCompositeConstruct %mat2v3float %925 %932
%934 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%935 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%936 = OpCompositeConstruct %mat2v3float %934 %935
%937 = OpCompositeExtract %v3float %933 0
%938 = OpCompositeExtract %v3float %936 0
%939 = OpFOrdEqual %v3bool %937 %938
%940 = OpAll %bool %939
%941 = OpCompositeExtract %v3float %933 1
%942 = OpCompositeExtract %v3float %936 1
%943 = OpFOrdEqual %v3bool %941 %942
%944 = OpAll %bool %943
%945 = OpLogicalAnd %bool %940 %944
OpBranch %923
%923 = OpLabel
%946 = OpPhi %bool %false %898 %945 %922
OpStore %_0_ok %946
%947 = OpLoad %bool %_0_ok
OpSelectionMerge %949 None
OpBranchConditional %947 %948 %949
%948 = OpLabel
%950 = OpLoad %v4float %_10_f4
%951 = OpVectorShuffle %v3float %950 %950 0 1 2
%952 = OpLoad %v4float %_10_f4
%953 = OpCompositeExtract %float %952 3
%954 = OpLoad %v4float %_10_f4
%955 = OpCompositeExtract %float %954 0
%956 = OpLoad %v4float %_10_f4
%957 = OpVectorShuffle %v3float %956 %956 1 2 3
%958 = OpCompositeExtract %float %951 0
%959 = OpCompositeExtract %float %951 1
%960 = OpCompositeExtract %float %951 2
%961 = OpCompositeConstruct %v4float %958 %959 %960 %953
%962 = OpCompositeExtract %float %957 0
%963 = OpCompositeExtract %float %957 1
%964 = OpCompositeExtract %float %957 2
%965 = OpCompositeConstruct %v4float %955 %962 %963 %964
%966 = OpCompositeConstruct %mat2v4float %961 %965
%967 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%968 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%969 = OpCompositeConstruct %mat2v4float %967 %968
%970 = OpCompositeExtract %v4float %966 0
%971 = OpCompositeExtract %v4float %969 0
%972 = OpFOrdEqual %v4bool %970 %971
%973 = OpAll %bool %972
%974 = OpCompositeExtract %v4float %966 1
%975 = OpCompositeExtract %v4float %969 1
%976 = OpFOrdEqual %v4bool %974 %975
%977 = OpAll %bool %976
%978 = OpLogicalAnd %bool %973 %977
OpBranch %949
%949 = OpLabel
%979 = OpPhi %bool %false %923 %978 %948
OpStore %_0_ok %979
%980 = OpLoad %bool %_0_ok
OpSelectionMerge %982 None
OpBranchConditional %980 %981 %982
%981 = OpLabel
%983 = OpLoad %v4float %_10_f4
%984 = OpVectorShuffle %v2float %983 %983 0 1
%985 = OpLoad %v4float %_10_f4
%986 = OpVectorShuffle %v2float %985 %985 2 3
%987 = OpLoad %v4float %_10_f4
%988 = OpCompositeExtract %float %987 0
%989 = OpLoad %v4float %_10_f4
%990 = OpCompositeExtract %float %989 1
%991 = OpCompositeConstruct %v2float %988 %990
%992 = OpCompositeConstruct %mat3v2float %984 %986 %991
%993 = OpCompositeConstruct %v2float %float_1 %float_2
%994 = OpCompositeConstruct %v2float %float_3 %float_4
%995 = OpCompositeConstruct %v2float %float_1 %float_2
%996 = OpCompositeConstruct %mat3v2float %993 %994 %995
%997 = OpCompositeExtract %v2float %992 0
%998 = OpCompositeExtract %v2float %996 0
%999 = OpFOrdEqual %v2bool %997 %998
%1000 = OpAll %bool %999
%1001 = OpCompositeExtract %v2float %992 1
%1002 = OpCompositeExtract %v2float %996 1
%1003 = OpFOrdEqual %v2bool %1001 %1002
%1004 = OpAll %bool %1003
%1005 = OpLogicalAnd %bool %1000 %1004
%1006 = OpCompositeExtract %v2float %992 2
%1007 = OpCompositeExtract %v2float %996 2
%1008 = OpFOrdEqual %v2bool %1006 %1007
%1009 = OpAll %bool %1008
%1010 = OpLogicalAnd %bool %1005 %1009
OpBranch %982
%982 = OpLabel
%1011 = OpPhi %bool %false %949 %1010 %981
OpStore %_0_ok %1011
%1012 = OpLoad %bool %_0_ok
OpSelectionMerge %1014 None
OpBranchConditional %1012 %1013 %1014
%1013 = OpLabel
%1015 = OpLoad %v4float %_10_f4
%1016 = OpVectorShuffle %v2float %1015 %1015 0 1
%1017 = OpLoad %v4float %_10_f4
%1018 = OpVectorShuffle %v2float %1017 %1017 2 3
%1019 = OpLoad %v4float %_10_f4
%1020 = OpLoad %v4float %_10_f4
%1021 = OpCompositeExtract %float %1020 0
%1022 = OpLoad %v4float %_10_f4
%1023 = OpVectorShuffle %v2float %1022 %1022 1 2
%1024 = OpLoad %v4float %_10_f4
%1025 = OpCompositeExtract %float %1024 3
%1026 = OpCompositeExtract %float %1016 0
%1027 = OpCompositeExtract %float %1016 1
%1028 = OpCompositeExtract %float %1018 0
%1029 = OpCompositeExtract %float %1018 1
%1030 = OpCompositeConstruct %v4float %1026 %1027 %1028 %1029
%1031 = OpCompositeExtract %float %1023 0
%1032 = OpCompositeExtract %float %1023 1
%1033 = OpCompositeConstruct %v4float %1021 %1031 %1032 %1025
%1034 = OpCompositeConstruct %mat3v4float %1030 %1019 %1033
%1035 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%1036 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%1037 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%1038 = OpCompositeConstruct %mat3v4float %1035 %1036 %1037
%1039 = OpCompositeExtract %v4float %1034 0
%1040 = OpCompositeExtract %v4float %1038 0
%1041 = OpFOrdEqual %v4bool %1039 %1040
%1042 = OpAll %bool %1041
%1043 = OpCompositeExtract %v4float %1034 1
%1044 = OpCompositeExtract %v4float %1038 1
%1045 = OpFOrdEqual %v4bool %1043 %1044
%1046 = OpAll %bool %1045
%1047 = OpLogicalAnd %bool %1042 %1046
%1048 = OpCompositeExtract %v4float %1034 2
%1049 = OpCompositeExtract %v4float %1038 2
%1050 = OpFOrdEqual %v4bool %1048 %1049
%1051 = OpAll %bool %1050
%1052 = OpLogicalAnd %bool %1047 %1051
OpBranch %1014
%1014 = OpLabel
%1053 = OpPhi %bool %false %982 %1052 %1013
OpStore %_0_ok %1053
%1054 = OpLoad %bool %_0_ok
OpSelectionMerge %1056 None
OpBranchConditional %1054 %1055 %1056
%1055 = OpLabel
%1057 = OpLoad %v4float %_10_f4
%1058 = OpVectorShuffle %v2float %1057 %1057 0 1
%1059 = OpLoad %v4float %_10_f4
%1060 = OpCompositeExtract %float %1059 2
%1061 = OpLoad %v4float %_10_f4
%1062 = OpCompositeExtract %float %1061 3
%1063 = OpLoad %v4float %_10_f4
%1064 = OpVectorShuffle %v2float %1063 %1063 0 1
%1065 = OpLoad %v4float %_10_f4
%1066 = OpCompositeExtract %float %1065 2
%1067 = OpLoad %v4float %_10_f4
%1068 = OpCompositeExtract %float %1067 3
%1069 = OpCompositeConstruct %v2float %1060 %1062
%1070 = OpCompositeConstruct %v2float %1066 %1068
%1071 = OpCompositeConstruct %mat4v2float %1058 %1069 %1064 %1070
%1072 = OpCompositeConstruct %v2float %float_1 %float_2
%1073 = OpCompositeConstruct %v2float %float_3 %float_4
%1074 = OpCompositeConstruct %v2float %float_1 %float_2
%1075 = OpCompositeConstruct %v2float %float_3 %float_4
%1076 = OpCompositeConstruct %mat4v2float %1072 %1073 %1074 %1075
%1077 = OpCompositeExtract %v2float %1071 0
%1078 = OpCompositeExtract %v2float %1076 0
%1079 = OpFOrdEqual %v2bool %1077 %1078
%1080 = OpAll %bool %1079
%1081 = OpCompositeExtract %v2float %1071 1
%1082 = OpCompositeExtract %v2float %1076 1
%1083 = OpFOrdEqual %v2bool %1081 %1082
%1084 = OpAll %bool %1083
%1085 = OpLogicalAnd %bool %1080 %1084
%1086 = OpCompositeExtract %v2float %1071 2
%1087 = OpCompositeExtract %v2float %1076 2
%1088 = OpFOrdEqual %v2bool %1086 %1087
%1089 = OpAll %bool %1088
%1090 = OpLogicalAnd %bool %1085 %1089
%1091 = OpCompositeExtract %v2float %1071 3
%1092 = OpCompositeExtract %v2float %1076 3
%1093 = OpFOrdEqual %v2bool %1091 %1092
%1094 = OpAll %bool %1093
%1095 = OpLogicalAnd %bool %1090 %1094
OpBranch %1056
%1056 = OpLabel
%1096 = OpPhi %bool %false %1014 %1095 %1055
OpStore %_0_ok %1096
%1097 = OpLoad %bool %_0_ok
OpSelectionMerge %1099 None
OpBranchConditional %1097 %1098 %1099
%1098 = OpLabel
%1100 = OpLoad %v4float %_10_f4
%1101 = OpCompositeExtract %float %1100 0
%1102 = OpLoad %v4float %_10_f4
%1103 = OpVectorShuffle %v2float %1102 %1102 1 2
%1104 = OpLoad %v4float %_10_f4
%1105 = OpVectorShuffle %v2float %1104 %1104 3 0
%1106 = OpLoad %v4float %_10_f4
%1107 = OpCompositeExtract %float %1106 1
%1108 = OpLoad %v4float %_10_f4
%1109 = OpVectorShuffle %v3float %1108 %1108 2 3 0
%1110 = OpLoad %v4float %_10_f4
%1111 = OpVectorShuffle %v3float %1110 %1110 1 2 3
%1112 = OpCompositeExtract %float %1103 0
%1113 = OpCompositeExtract %float %1103 1
%1114 = OpCompositeConstruct %v3float %1101 %1112 %1113
%1115 = OpCompositeExtract %float %1105 0
%1116 = OpCompositeExtract %float %1105 1
%1117 = OpCompositeConstruct %v3float %1115 %1116 %1107
%1118 = OpCompositeConstruct %mat4v3float %1114 %1117 %1109 %1111
%1119 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%1120 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%1121 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%1122 = OpCompositeConstruct %v3float %float_2 %float_3 %float_4
%1123 = OpCompositeConstruct %mat4v3float %1119 %1120 %1121 %1122
%1124 = OpCompositeExtract %v3float %1118 0
%1125 = OpCompositeExtract %v3float %1123 0
%1126 = OpFOrdEqual %v3bool %1124 %1125
%1127 = OpAll %bool %1126
%1128 = OpCompositeExtract %v3float %1118 1
%1129 = OpCompositeExtract %v3float %1123 1
%1130 = OpFOrdEqual %v3bool %1128 %1129
%1131 = OpAll %bool %1130
%1132 = OpLogicalAnd %bool %1127 %1131
%1133 = OpCompositeExtract %v3float %1118 2
%1134 = OpCompositeExtract %v3float %1123 2
%1135 = OpFOrdEqual %v3bool %1133 %1134
%1136 = OpAll %bool %1135
%1137 = OpLogicalAnd %bool %1132 %1136
%1138 = OpCompositeExtract %v3float %1118 3
%1139 = OpCompositeExtract %v3float %1123 3
%1140 = OpFOrdEqual %v3bool %1138 %1139
%1141 = OpAll %bool %1140
%1142 = OpLogicalAnd %bool %1137 %1141
OpBranch %1099
%1099 = OpLabel
%1143 = OpPhi %bool %false %1056 %1142 %1098
OpStore %_0_ok %1143
%1144 = OpLoad %bool %_0_ok
OpSelectionMerge %1146 None
OpBranchConditional %1144 %1145 %1146
%1145 = OpLabel
%1147 = OpFunctionCall %bool %test_half_b
OpBranch %1146
%1146 = OpLabel
%1148 = OpPhi %bool %false %1099 %1147 %1145
OpSelectionMerge %1152 None
OpBranchConditional %1148 %1150 %1151
%1150 = OpLabel
%1153 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%1156 = OpLoad %v4float %1153
OpStore %1149 %1156
OpBranch %1152
%1151 = OpLabel
%1157 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%1159 = OpLoad %v4float %1157
OpStore %1149 %1159
OpBranch %1152
%1152 = OpLabel
%1160 = OpLoad %v4float %1149
OpReturnValue %1160
OpFunctionEnd
