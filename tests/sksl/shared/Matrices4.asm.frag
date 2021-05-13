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
OpName %m4 "m4"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m4 "_1_m4"
OpName %_2_m1 "_2_m1"
OpName %_3_m2 "_3_m2"
OpName %_4_m3 "_4_m3"
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
OpDecorate %m4 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %m1 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m2 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
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
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%82 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%150 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v2float %float_6 %float_0
%35 = OpCompositeConstruct %v2float %float_0 %float_6
%33 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %m4 %33
%37 = OpLoad %bool %ok
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %mat2v2float %m4
%42 = OpCompositeConstruct %v2float %float_6 %float_0
%43 = OpCompositeConstruct %v2float %float_0 %float_6
%41 = OpCompositeConstruct %mat2v2float %42 %43
%45 = OpCompositeExtract %v2float %40 0
%46 = OpCompositeExtract %v2float %41 0
%47 = OpFOrdEqual %v2bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %40 1
%50 = OpCompositeExtract %v2float %41 1
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpBranch %39
%39 = OpLabel
%54 = OpPhi %bool %false %25 %53 %38
OpStore %ok %54
%61 = OpCompositeConstruct %v2float %float_1 %float_2
%62 = OpCompositeConstruct %v2float %float_3 %float_4
%60 = OpCompositeConstruct %mat2v2float %61 %62
OpStore %m1 %60
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v2float %m1
%68 = OpCompositeConstruct %v2float %float_1 %float_2
%69 = OpCompositeConstruct %v2float %float_3 %float_4
%67 = OpCompositeConstruct %mat2v2float %68 %69
%70 = OpCompositeExtract %v2float %66 0
%71 = OpCompositeExtract %v2float %67 0
%72 = OpFOrdEqual %v2bool %70 %71
%73 = OpAll %bool %72
%74 = OpCompositeExtract %v2float %66 1
%75 = OpCompositeExtract %v2float %67 1
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %73 %77
OpBranch %65
%65 = OpLabel
%79 = OpPhi %bool %false %39 %78 %64
OpStore %ok %79
%84 = OpCompositeExtract %float %82 0
%85 = OpCompositeExtract %float %82 1
%86 = OpCompositeExtract %float %82 2
%87 = OpCompositeExtract %float %82 3
%88 = OpCompositeConstruct %v2float %84 %85
%89 = OpCompositeConstruct %v2float %86 %87
%83 = OpCompositeConstruct %mat2v2float %88 %89
OpStore %m2 %83
%90 = OpLoad %bool %ok
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %mat2v2float %m2
%95 = OpCompositeConstruct %v2float %float_5 %float_5
%96 = OpCompositeConstruct %v2float %float_5 %float_5
%94 = OpCompositeConstruct %mat2v2float %95 %96
%97 = OpCompositeExtract %v2float %93 0
%98 = OpCompositeExtract %v2float %94 0
%99 = OpFOrdEqual %v2bool %97 %98
%100 = OpAll %bool %99
%101 = OpCompositeExtract %v2float %93 1
%102 = OpCompositeExtract %v2float %94 1
%103 = OpFOrdEqual %v2bool %101 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %100 %104
OpBranch %92
%92 = OpLabel
%106 = OpPhi %bool %false %65 %105 %91
OpStore %ok %106
%108 = OpLoad %mat2v2float %m1
OpStore %m3 %108
%109 = OpLoad %bool %ok
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %mat2v2float %m3
%114 = OpCompositeConstruct %v2float %float_1 %float_2
%115 = OpCompositeConstruct %v2float %float_3 %float_4
%113 = OpCompositeConstruct %mat2v2float %114 %115
%116 = OpCompositeExtract %v2float %112 0
%117 = OpCompositeExtract %v2float %113 0
%118 = OpFOrdEqual %v2bool %116 %117
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v2float %112 1
%121 = OpCompositeExtract %v2float %113 1
%122 = OpFOrdEqual %v2bool %120 %121
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %119 %123
OpBranch %111
%111 = OpLabel
%125 = OpPhi %bool %false %92 %124 %110
OpStore %ok %125
%126 = OpLoad %mat2v2float %m3
%127 = OpLoad %mat2v2float %m4
%128 = OpMatrixTimesMatrix %mat2v2float %126 %127
OpStore %m3 %128
%129 = OpLoad %bool %ok
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpLoad %mat2v2float %m3
%137 = OpCompositeConstruct %v2float %float_6 %float_12
%138 = OpCompositeConstruct %v2float %float_18 %float_24
%136 = OpCompositeConstruct %mat2v2float %137 %138
%139 = OpCompositeExtract %v2float %132 0
%140 = OpCompositeExtract %v2float %136 0
%141 = OpFOrdEqual %v2bool %139 %140
%142 = OpAll %bool %141
%143 = OpCompositeExtract %v2float %132 1
%144 = OpCompositeExtract %v2float %136 1
%145 = OpFOrdEqual %v2bool %143 %144
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %142 %146
OpBranch %131
%131 = OpLabel
%148 = OpPhi %bool %false %111 %147 %130
OpStore %ok %148
%149 = OpLoad %bool %ok
OpReturnValue %149
OpFunctionEnd
%main = OpFunction %v4float None %150
%151 = OpFunctionParameter %_ptr_Function_v2float
%152 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m3 = OpVariable %_ptr_Function_mat2v2float Function
%265 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%156 = OpCompositeConstruct %v2float %float_6 %float_0
%157 = OpCompositeConstruct %v2float %float_0 %float_6
%155 = OpCompositeConstruct %mat2v2float %156 %157
OpStore %_1_m4 %155
%158 = OpLoad %bool %_0_ok
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %mat2v2float %_1_m4
%163 = OpCompositeConstruct %v2float %float_6 %float_0
%164 = OpCompositeConstruct %v2float %float_0 %float_6
%162 = OpCompositeConstruct %mat2v2float %163 %164
%165 = OpCompositeExtract %v2float %161 0
%166 = OpCompositeExtract %v2float %162 0
%167 = OpFOrdEqual %v2bool %165 %166
%168 = OpAll %bool %167
%169 = OpCompositeExtract %v2float %161 1
%170 = OpCompositeExtract %v2float %162 1
%171 = OpFOrdEqual %v2bool %169 %170
%172 = OpAll %bool %171
%173 = OpLogicalAnd %bool %168 %172
OpBranch %160
%160 = OpLabel
%174 = OpPhi %bool %false %152 %173 %159
OpStore %_0_ok %174
%177 = OpCompositeConstruct %v2float %float_1 %float_2
%178 = OpCompositeConstruct %v2float %float_3 %float_4
%176 = OpCompositeConstruct %mat2v2float %177 %178
OpStore %_2_m1 %176
%179 = OpLoad %bool %_0_ok
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpLoad %mat2v2float %_2_m1
%184 = OpCompositeConstruct %v2float %float_1 %float_2
%185 = OpCompositeConstruct %v2float %float_3 %float_4
%183 = OpCompositeConstruct %mat2v2float %184 %185
%186 = OpCompositeExtract %v2float %182 0
%187 = OpCompositeExtract %v2float %183 0
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpCompositeExtract %v2float %182 1
%191 = OpCompositeExtract %v2float %183 1
%192 = OpFOrdEqual %v2bool %190 %191
%193 = OpAll %bool %192
%194 = OpLogicalAnd %bool %189 %193
OpBranch %181
%181 = OpLabel
%195 = OpPhi %bool %false %160 %194 %180
OpStore %_0_ok %195
%198 = OpCompositeExtract %float %82 0
%199 = OpCompositeExtract %float %82 1
%200 = OpCompositeExtract %float %82 2
%201 = OpCompositeExtract %float %82 3
%202 = OpCompositeConstruct %v2float %198 %199
%203 = OpCompositeConstruct %v2float %200 %201
%197 = OpCompositeConstruct %mat2v2float %202 %203
OpStore %_3_m2 %197
%204 = OpLoad %bool %_0_ok
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpLoad %mat2v2float %_3_m2
%209 = OpCompositeConstruct %v2float %float_5 %float_5
%210 = OpCompositeConstruct %v2float %float_5 %float_5
%208 = OpCompositeConstruct %mat2v2float %209 %210
%211 = OpCompositeExtract %v2float %207 0
%212 = OpCompositeExtract %v2float %208 0
%213 = OpFOrdEqual %v2bool %211 %212
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v2float %207 1
%216 = OpCompositeExtract %v2float %208 1
%217 = OpFOrdEqual %v2bool %215 %216
%218 = OpAll %bool %217
%219 = OpLogicalAnd %bool %214 %218
OpBranch %206
%206 = OpLabel
%220 = OpPhi %bool %false %181 %219 %205
OpStore %_0_ok %220
%222 = OpLoad %mat2v2float %_2_m1
OpStore %_4_m3 %222
%223 = OpLoad %bool %_0_ok
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %mat2v2float %_4_m3
%228 = OpCompositeConstruct %v2float %float_1 %float_2
%229 = OpCompositeConstruct %v2float %float_3 %float_4
%227 = OpCompositeConstruct %mat2v2float %228 %229
%230 = OpCompositeExtract %v2float %226 0
%231 = OpCompositeExtract %v2float %227 0
%232 = OpFOrdEqual %v2bool %230 %231
%233 = OpAll %bool %232
%234 = OpCompositeExtract %v2float %226 1
%235 = OpCompositeExtract %v2float %227 1
%236 = OpFOrdEqual %v2bool %234 %235
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %233 %237
OpBranch %225
%225 = OpLabel
%239 = OpPhi %bool %false %206 %238 %224
OpStore %_0_ok %239
%240 = OpLoad %mat2v2float %_4_m3
%241 = OpLoad %mat2v2float %_1_m4
%242 = OpMatrixTimesMatrix %mat2v2float %240 %241
OpStore %_4_m3 %242
%243 = OpLoad %bool %_0_ok
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpLoad %mat2v2float %_4_m3
%248 = OpCompositeConstruct %v2float %float_6 %float_12
%249 = OpCompositeConstruct %v2float %float_18 %float_24
%247 = OpCompositeConstruct %mat2v2float %248 %249
%250 = OpCompositeExtract %v2float %246 0
%251 = OpCompositeExtract %v2float %247 0
%252 = OpFOrdEqual %v2bool %250 %251
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v2float %246 1
%255 = OpCompositeExtract %v2float %247 1
%256 = OpFOrdEqual %v2bool %254 %255
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %253 %257
OpBranch %245
%245 = OpLabel
%259 = OpPhi %bool %false %225 %258 %244
OpStore %_0_ok %259
%260 = OpLoad %bool %_0_ok
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpFunctionCall %bool %test_half_b
OpBranch %262
%262 = OpLabel
%264 = OpPhi %bool %false %245 %263 %261
OpSelectionMerge %269 None
OpBranchConditional %264 %267 %268
%267 = OpLabel
%270 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%274 = OpLoad %v4float %270
OpStore %265 %274
OpBranch %269
%268 = OpLabel
%275 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%277 = OpLoad %v4float %275
OpStore %265 %277
OpBranch %269
%269 = OpLabel
%278 = OpLoad %v4float %265
OpReturnValue %278
OpFunctionEnd
