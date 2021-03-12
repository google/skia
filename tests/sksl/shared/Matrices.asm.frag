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
OpDecorate %48 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
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
%50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%134 = OpTypeFunction %v4float
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
%47 = OpCompositeConstruct %v2float %float_1 %float_2
%48 = OpCompositeConstruct %v2float %float_3 %float_4
%46 = OpCompositeConstruct %mat2v2float %47 %48
OpStore %m1 %46
%52 = OpCompositeExtract %float %50 0
%53 = OpCompositeExtract %float %50 1
%54 = OpCompositeExtract %float %50 2
%55 = OpCompositeExtract %float %50 3
%56 = OpCompositeConstruct %v2float %52 %53
%57 = OpCompositeConstruct %v2float %54 %55
%51 = OpCompositeConstruct %mat2v2float %56 %57
OpStore %m2 %51
%59 = OpLoad %mat2v2float %m1
OpStore %m3 %59
%62 = OpCompositeConstruct %v2float %float_1 %float_0
%63 = OpCompositeConstruct %v2float %float_0 %float_1
%61 = OpCompositeConstruct %mat2v2float %62 %63
OpStore %m4 %61
%64 = OpLoad %mat2v2float %m3
%65 = OpLoad %mat2v2float %m4
%66 = OpMatrixTimesMatrix %mat2v2float %64 %65
OpStore %m3 %66
%70 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%72 = OpLoad %v2float %70
%73 = OpCompositeExtract %float %72 0
%75 = OpCompositeConstruct %v2float %73 %float_0
%76 = OpCompositeConstruct %v2float %float_0 %73
%74 = OpCompositeConstruct %mat2v2float %75 %76
OpStore %m5 %74
%79 = OpCompositeConstruct %v2float %float_1 %float_2
%80 = OpCompositeConstruct %v2float %float_3 %float_4
%78 = OpCompositeConstruct %mat2v2float %79 %80
OpStore %m6 %78
%81 = OpLoad %mat2v2float %m6
%82 = OpLoad %mat2v2float %m5
%83 = OpCompositeExtract %v2float %81 0
%84 = OpCompositeExtract %v2float %82 0
%85 = OpFAdd %v2float %83 %84
%86 = OpCompositeExtract %v2float %81 1
%87 = OpCompositeExtract %v2float %82 1
%88 = OpFAdd %v2float %86 %87
%89 = OpCompositeConstruct %mat2v2float %85 %88
OpStore %m6 %89
%96 = OpCompositeConstruct %v2float %float_5 %float_6
%97 = OpCompositeConstruct %v2float %float_7 %float_8
%95 = OpCompositeConstruct %mat2v2float %96 %97
OpStore %m7 %95
%101 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%102 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%103 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%100 = OpCompositeConstruct %mat3v3float %101 %102 %103
OpStore %m9 %100
%108 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%109 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%110 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%107 = OpCompositeConstruct %mat4v4float %108 %109 %110 %111
OpStore %m10 %107
%114 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%115 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%116 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%117 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%113 = OpCompositeConstruct %mat4v4float %114 %115 %116 %117
OpStore %m11 %113
%118 = OpLoad %mat4v4float %m11
%119 = OpLoad %mat4v4float %m10
%120 = OpCompositeExtract %v4float %118 0
%121 = OpCompositeExtract %v4float %119 0
%122 = OpFSub %v4float %120 %121
%123 = OpCompositeExtract %v4float %118 1
%124 = OpCompositeExtract %v4float %119 1
%125 = OpFSub %v4float %123 %124
%126 = OpCompositeExtract %v4float %118 2
%127 = OpCompositeExtract %v4float %119 2
%128 = OpFSub %v4float %126 %127
%129 = OpCompositeExtract %v4float %118 3
%130 = OpCompositeExtract %v4float %119 3
%131 = OpFSub %v4float %129 %130
%132 = OpCompositeConstruct %mat4v4float %122 %125 %128 %131
OpStore %m11 %132
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %134
%135 = OpLabel
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
%230 = OpVariable %_ptr_Function_v4float Function
%138 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%139 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%140 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%137 = OpCompositeConstruct %mat3v3float %138 %139 %140
%141 = OpMatrixTimesVector %v3float %137 %32
OpStore %_0_v1 %141
%144 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%145 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%146 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%143 = OpCompositeConstruct %mat3v3float %144 %145 %146
%147 = OpVectorTimesMatrix %v3float %32 %143
OpStore %_1_v2 %147
%150 = OpCompositeConstruct %v2float %float_1 %float_2
%151 = OpCompositeConstruct %v2float %float_3 %float_4
%149 = OpCompositeConstruct %mat2v2float %150 %151
OpStore %_2_m1 %149
%154 = OpCompositeExtract %float %50 0
%155 = OpCompositeExtract %float %50 1
%156 = OpCompositeExtract %float %50 2
%157 = OpCompositeExtract %float %50 3
%158 = OpCompositeConstruct %v2float %154 %155
%159 = OpCompositeConstruct %v2float %156 %157
%153 = OpCompositeConstruct %mat2v2float %158 %159
OpStore %_3_m2 %153
%161 = OpLoad %mat2v2float %_2_m1
OpStore %_4_m3 %161
%164 = OpCompositeConstruct %v2float %float_1 %float_0
%165 = OpCompositeConstruct %v2float %float_0 %float_1
%163 = OpCompositeConstruct %mat2v2float %164 %165
OpStore %_5_m4 %163
%166 = OpLoad %mat2v2float %_4_m3
%167 = OpLoad %mat2v2float %_5_m4
%168 = OpMatrixTimesMatrix %mat2v2float %166 %167
OpStore %_4_m3 %168
%170 = OpAccessChain %_ptr_Function_v2float %_2_m1 %int_0
%171 = OpLoad %v2float %170
%172 = OpCompositeExtract %float %171 0
%174 = OpCompositeConstruct %v2float %172 %float_0
%175 = OpCompositeConstruct %v2float %float_0 %172
%173 = OpCompositeConstruct %mat2v2float %174 %175
OpStore %_6_m5 %173
%178 = OpCompositeConstruct %v2float %float_1 %float_2
%179 = OpCompositeConstruct %v2float %float_3 %float_4
%177 = OpCompositeConstruct %mat2v2float %178 %179
OpStore %_7_m6 %177
%180 = OpLoad %mat2v2float %_7_m6
%181 = OpLoad %mat2v2float %_6_m5
%182 = OpCompositeExtract %v2float %180 0
%183 = OpCompositeExtract %v2float %181 0
%184 = OpFAdd %v2float %182 %183
%185 = OpCompositeExtract %v2float %180 1
%186 = OpCompositeExtract %v2float %181 1
%187 = OpFAdd %v2float %185 %186
%188 = OpCompositeConstruct %mat2v2float %184 %187
OpStore %_7_m6 %188
%191 = OpCompositeConstruct %v2float %float_5 %float_6
%192 = OpCompositeConstruct %v2float %float_7 %float_8
%190 = OpCompositeConstruct %mat2v2float %191 %192
OpStore %_8_m7 %190
%195 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%196 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%194 = OpCompositeConstruct %mat3v3float %195 %196 %197
OpStore %_9_m9 %194
%200 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%201 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%202 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%203 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%199 = OpCompositeConstruct %mat4v4float %200 %201 %202 %203
OpStore %_10_m10 %199
%206 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%207 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%208 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%209 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%205 = OpCompositeConstruct %mat4v4float %206 %207 %208 %209
OpStore %_11_m11 %205
%210 = OpLoad %mat4v4float %_11_m11
%211 = OpLoad %mat4v4float %_10_m10
%212 = OpCompositeExtract %v4float %210 0
%213 = OpCompositeExtract %v4float %211 0
%214 = OpFSub %v4float %212 %213
%215 = OpCompositeExtract %v4float %210 1
%216 = OpCompositeExtract %v4float %211 1
%217 = OpFSub %v4float %215 %216
%218 = OpCompositeExtract %v4float %210 2
%219 = OpCompositeExtract %v4float %211 2
%220 = OpFSub %v4float %218 %219
%221 = OpCompositeExtract %v4float %210 3
%222 = OpCompositeExtract %v4float %211 3
%223 = OpFSub %v4float %221 %222
%224 = OpCompositeConstruct %mat4v4float %214 %217 %220 %223
OpStore %_11_m11 %224
OpSelectionMerge %227 None
OpBranchConditional %true %226 %227
%226 = OpLabel
%228 = OpFunctionCall %bool %test_half
OpBranch %227
%227 = OpLabel
%229 = OpPhi %bool %false %135 %228 %226
OpSelectionMerge %234 None
OpBranchConditional %229 %232 %233
%232 = OpLabel
%235 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%237 = OpLoad %v4float %235
OpStore %230 %237
OpBranch %234
%233 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%240 = OpLoad %v4float %238
OpStore %230 %240
OpBranch %234
%234 = OpLabel
%241 = OpLoad %v4float %230
OpReturnValue %241
OpFunctionEnd
