OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %test_half "test_half"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m6 "m6"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_0_v1 "_0_v1"
OpName %_1_v2 "_1_v2"
OpName %_2_m1 "_2_m1"
OpName %_3_m2 "_3_m2"
OpName %_4_m3 "_4_m3"
OpName %_5_m4 "_5_m4"
OpName %_6_m5 "_6_m5"
OpName %_7_m6 "_7_m6"
OpName %_8_m7 "_8_m7"
OpName %_9_m9 "_9_m9"
OpName %_10_m10 "_10_m10"
OpName %_11_m11 "_11_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
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
%19 = OpTypeFunction %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%float_2 = OpConstant %float 2
%32 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%46 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%55 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%100 = OpConstantComposite %v3float %float_6 %float_7 %float_8
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%143 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_half = OpFunction %bool None %19
%20 = OpLabel
%v1 = OpVariable %_ptr_Function_v3float Function
%v2 = OpVariable %_ptr_Function_v3float Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m6 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%27 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%28 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%29 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%25 = OpCompositeConstruct %mat3v3float %27 %28 %29
%33 = OpMatrixTimesVector %v3float %25 %32
OpStore %v1 %33
%36 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%35 = OpCompositeConstruct %mat3v3float %36 %37 %38
%39 = OpVectorTimesMatrix %v3float %32 %35
OpStore %v2 %39
%48 = OpCompositeExtract %float %46 0
%49 = OpCompositeExtract %float %46 1
%50 = OpCompositeExtract %float %46 2
%51 = OpCompositeExtract %float %46 3
%52 = OpCompositeConstruct %v2float %48 %49
%53 = OpCompositeConstruct %v2float %50 %51
%47 = OpCompositeConstruct %mat2v2float %52 %53
OpStore %m1 %47
%57 = OpCompositeExtract %float %55 0
%58 = OpCompositeExtract %float %55 1
%59 = OpCompositeExtract %float %55 2
%60 = OpCompositeExtract %float %55 3
%61 = OpCompositeConstruct %v2float %57 %58
%62 = OpCompositeConstruct %v2float %59 %60
%56 = OpCompositeConstruct %mat2v2float %61 %62
OpStore %m2 %56
%64 = OpLoad %mat2v2float %m1
OpStore %m3 %64
%67 = OpCompositeConstruct %v2float %float_1 %float_0
%68 = OpCompositeConstruct %v2float %float_0 %float_1
%66 = OpCompositeConstruct %mat2v2float %67 %68
OpStore %m4 %66
%69 = OpLoad %mat2v2float %m3
%70 = OpLoad %mat2v2float %m4
%71 = OpMatrixTimesMatrix %mat2v2float %69 %70
OpStore %m3 %71
%75 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%77 = OpLoad %v2float %75
%78 = OpCompositeExtract %float %77 0
%80 = OpCompositeConstruct %v2float %78 %float_0
%81 = OpCompositeConstruct %v2float %float_0 %78
%79 = OpCompositeConstruct %mat2v2float %80 %81
OpStore %m5 %79
%84 = OpCompositeConstruct %v2float %float_1 %float_2
%85 = OpCompositeConstruct %v2float %float_3 %float_4
%83 = OpCompositeConstruct %mat2v2float %84 %85
OpStore %m6 %83
%86 = OpLoad %mat2v2float %m6
%87 = OpLoad %mat2v2float %m5
%88 = OpCompositeExtract %v2float %86 0
%89 = OpCompositeExtract %v2float %87 0
%90 = OpFAdd %v2float %88 %89
%91 = OpCompositeExtract %v2float %86 1
%92 = OpCompositeExtract %v2float %87 1
%93 = OpFAdd %v2float %91 %92
%94 = OpCompositeConstruct %mat2v2float %90 %93
OpStore %m6 %94
%102 = OpCompositeExtract %float %100 0
%103 = OpCompositeConstruct %v2float %float_5 %102
%104 = OpCompositeExtract %float %100 1
%105 = OpCompositeExtract %float %100 2
%106 = OpCompositeConstruct %v2float %104 %105
%101 = OpCompositeConstruct %mat2v2float %103 %106
OpStore %m7 %101
%110 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%111 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%112 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%109 = OpCompositeConstruct %mat3v3float %110 %111 %112
OpStore %m9 %109
%117 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%118 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%116 = OpCompositeConstruct %mat4v4float %117 %118 %119 %120
OpStore %m10 %116
%123 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%124 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%125 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%126 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%122 = OpCompositeConstruct %mat4v4float %123 %124 %125 %126
OpStore %m11 %122
%127 = OpLoad %mat4v4float %m11
%128 = OpLoad %mat4v4float %m10
%129 = OpCompositeExtract %v4float %127 0
%130 = OpCompositeExtract %v4float %128 0
%131 = OpFSub %v4float %129 %130
%132 = OpCompositeExtract %v4float %127 1
%133 = OpCompositeExtract %v4float %128 1
%134 = OpFSub %v4float %132 %133
%135 = OpCompositeExtract %v4float %127 2
%136 = OpCompositeExtract %v4float %128 2
%137 = OpFSub %v4float %135 %136
%138 = OpCompositeExtract %v4float %127 3
%139 = OpCompositeExtract %v4float %128 3
%140 = OpFSub %v4float %138 %139
%141 = OpCompositeConstruct %mat4v4float %131 %134 %137 %140
OpStore %m11 %141
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %143
%144 = OpLabel
%_0_v1 = OpVariable %_ptr_Function_v3float Function
%_1_v2 = OpVariable %_ptr_Function_v3float Function
%_2_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m2 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_10_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_11_m11 = OpVariable %_ptr_Function_mat4v4float Function
%246 = OpVariable %_ptr_Function_v4float Function
%147 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%148 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%149 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%146 = OpCompositeConstruct %mat3v3float %147 %148 %149
%150 = OpMatrixTimesVector %v3float %146 %32
OpStore %_0_v1 %150
%153 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
%156 = OpVectorTimesMatrix %v3float %32 %152
OpStore %_1_v2 %156
%159 = OpCompositeExtract %float %46 0
%160 = OpCompositeExtract %float %46 1
%161 = OpCompositeExtract %float %46 2
%162 = OpCompositeExtract %float %46 3
%163 = OpCompositeConstruct %v2float %159 %160
%164 = OpCompositeConstruct %v2float %161 %162
%158 = OpCompositeConstruct %mat2v2float %163 %164
OpStore %_2_m1 %158
%167 = OpCompositeExtract %float %55 0
%168 = OpCompositeExtract %float %55 1
%169 = OpCompositeExtract %float %55 2
%170 = OpCompositeExtract %float %55 3
%171 = OpCompositeConstruct %v2float %167 %168
%172 = OpCompositeConstruct %v2float %169 %170
%166 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %_3_m2 %166
%174 = OpLoad %mat2v2float %_2_m1
OpStore %_4_m3 %174
%177 = OpCompositeConstruct %v2float %float_1 %float_0
%178 = OpCompositeConstruct %v2float %float_0 %float_1
%176 = OpCompositeConstruct %mat2v2float %177 %178
OpStore %_5_m4 %176
%179 = OpLoad %mat2v2float %_4_m3
%180 = OpLoad %mat2v2float %_5_m4
%181 = OpMatrixTimesMatrix %mat2v2float %179 %180
OpStore %_4_m3 %181
%183 = OpAccessChain %_ptr_Function_v2float %_2_m1 %int_0
%184 = OpLoad %v2float %183
%185 = OpCompositeExtract %float %184 0
%187 = OpCompositeConstruct %v2float %185 %float_0
%188 = OpCompositeConstruct %v2float %float_0 %185
%186 = OpCompositeConstruct %mat2v2float %187 %188
OpStore %_6_m5 %186
%191 = OpCompositeConstruct %v2float %float_1 %float_2
%192 = OpCompositeConstruct %v2float %float_3 %float_4
%190 = OpCompositeConstruct %mat2v2float %191 %192
OpStore %_7_m6 %190
%193 = OpLoad %mat2v2float %_7_m6
%194 = OpLoad %mat2v2float %_6_m5
%195 = OpCompositeExtract %v2float %193 0
%196 = OpCompositeExtract %v2float %194 0
%197 = OpFAdd %v2float %195 %196
%198 = OpCompositeExtract %v2float %193 1
%199 = OpCompositeExtract %v2float %194 1
%200 = OpFAdd %v2float %198 %199
%201 = OpCompositeConstruct %mat2v2float %197 %200
OpStore %_7_m6 %201
%204 = OpCompositeExtract %float %100 0
%205 = OpCompositeConstruct %v2float %float_5 %204
%206 = OpCompositeExtract %float %100 1
%207 = OpCompositeExtract %float %100 2
%208 = OpCompositeConstruct %v2float %206 %207
%203 = OpCompositeConstruct %mat2v2float %205 %208
OpStore %_8_m7 %203
%211 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%212 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%210 = OpCompositeConstruct %mat3v3float %211 %212 %213
OpStore %_9_m9 %210
%216 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%217 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%218 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%219 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%215 = OpCompositeConstruct %mat4v4float %216 %217 %218 %219
OpStore %_10_m10 %215
%222 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%223 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%224 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%225 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%221 = OpCompositeConstruct %mat4v4float %222 %223 %224 %225
OpStore %_11_m11 %221
%226 = OpLoad %mat4v4float %_11_m11
%227 = OpLoad %mat4v4float %_10_m10
%228 = OpCompositeExtract %v4float %226 0
%229 = OpCompositeExtract %v4float %227 0
%230 = OpFSub %v4float %228 %229
%231 = OpCompositeExtract %v4float %226 1
%232 = OpCompositeExtract %v4float %227 1
%233 = OpFSub %v4float %231 %232
%234 = OpCompositeExtract %v4float %226 2
%235 = OpCompositeExtract %v4float %227 2
%236 = OpFSub %v4float %234 %235
%237 = OpCompositeExtract %v4float %226 3
%238 = OpCompositeExtract %v4float %227 3
%239 = OpFSub %v4float %237 %238
%240 = OpCompositeConstruct %mat4v4float %230 %233 %236 %239
OpStore %_11_m11 %240
OpSelectionMerge %243 None
OpBranchConditional %true %242 %243
%242 = OpLabel
%244 = OpFunctionCall %bool %test_half
OpBranch %243
%243 = OpLabel
%245 = OpPhi %bool %false %144 %244 %242
OpSelectionMerge %250 None
OpBranchConditional %245 %248 %249
%248 = OpLabel
%251 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%253 = OpLoad %v4float %251
OpStore %246 %253
OpBranch %250
%249 = OpLabel
%254 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%256 = OpLoad %v4float %254
OpStore %246 %256
OpBranch %250
%250 = OpLabel
%257 = OpLoad %v4float %246
OpReturnValue %257
OpFunctionEnd
