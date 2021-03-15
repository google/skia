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
OpName %test_float "test_float"
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
OpName %test_half "test_half"
OpName %v1_0 "v1"
OpName %v2_0 "v2"
OpName %m1_0 "m1"
OpName %m2_0 "m2"
OpName %m3_0 "m3"
OpName %m4_0 "m4"
OpName %m5_0 "m5"
OpName %m6_0 "m6"
OpName %m7_0 "m7"
OpName %m9_0 "m9"
OpName %m10_0 "m10"
OpName %m11_0 "m11"
OpName %main "main"
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
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
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
%20 = OpTypeFunction %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%float_2 = OpConstant %float 2
%33 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%51 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
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
%225 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%test_float = OpFunction %bool None %20
%21 = OpLabel
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
%28 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%29 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%30 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%26 = OpCompositeConstruct %mat3v3float %28 %29 %30
%34 = OpMatrixTimesVector %v3float %26 %33
OpStore %v1 %34
%37 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%39 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%36 = OpCompositeConstruct %mat3v3float %37 %38 %39
%40 = OpVectorTimesMatrix %v3float %33 %36
OpStore %v2 %40
%48 = OpCompositeConstruct %v2float %float_1 %float_2
%49 = OpCompositeConstruct %v2float %float_3 %float_4
%47 = OpCompositeConstruct %mat2v2float %48 %49
OpStore %m1 %47
%53 = OpCompositeExtract %float %51 0
%54 = OpCompositeExtract %float %51 1
%55 = OpCompositeExtract %float %51 2
%56 = OpCompositeExtract %float %51 3
%57 = OpCompositeConstruct %v2float %53 %54
%58 = OpCompositeConstruct %v2float %55 %56
%52 = OpCompositeConstruct %mat2v2float %57 %58
OpStore %m2 %52
%60 = OpLoad %mat2v2float %m1
OpStore %m3 %60
%63 = OpCompositeConstruct %v2float %float_1 %float_0
%64 = OpCompositeConstruct %v2float %float_0 %float_1
%62 = OpCompositeConstruct %mat2v2float %63 %64
OpStore %m4 %62
%65 = OpLoad %mat2v2float %m3
%66 = OpLoad %mat2v2float %m4
%67 = OpMatrixTimesMatrix %mat2v2float %65 %66
OpStore %m3 %67
%71 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%73 = OpLoad %v2float %71
%74 = OpCompositeExtract %float %73 0
%76 = OpCompositeConstruct %v2float %74 %float_0
%77 = OpCompositeConstruct %v2float %float_0 %74
%75 = OpCompositeConstruct %mat2v2float %76 %77
OpStore %m5 %75
%80 = OpCompositeConstruct %v2float %float_1 %float_2
%81 = OpCompositeConstruct %v2float %float_3 %float_4
%79 = OpCompositeConstruct %mat2v2float %80 %81
OpStore %m6 %79
%82 = OpLoad %mat2v2float %m6
%83 = OpLoad %mat2v2float %m5
%84 = OpCompositeExtract %v2float %82 0
%85 = OpCompositeExtract %v2float %83 0
%86 = OpFAdd %v2float %84 %85
%87 = OpCompositeExtract %v2float %82 1
%88 = OpCompositeExtract %v2float %83 1
%89 = OpFAdd %v2float %87 %88
%90 = OpCompositeConstruct %mat2v2float %86 %89
OpStore %m6 %90
%97 = OpCompositeConstruct %v2float %float_5 %float_6
%98 = OpCompositeConstruct %v2float %float_7 %float_8
%96 = OpCompositeConstruct %mat2v2float %97 %98
OpStore %m7 %96
%102 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%103 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%104 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%101 = OpCompositeConstruct %mat3v3float %102 %103 %104
OpStore %m9 %101
%109 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%110 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%112 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%108 = OpCompositeConstruct %mat4v4float %109 %110 %111 %112
OpStore %m10 %108
%115 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%116 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%117 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%118 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%114 = OpCompositeConstruct %mat4v4float %115 %116 %117 %118
OpStore %m11 %114
%119 = OpLoad %mat4v4float %m11
%120 = OpLoad %mat4v4float %m10
%121 = OpCompositeExtract %v4float %119 0
%122 = OpCompositeExtract %v4float %120 0
%123 = OpFSub %v4float %121 %122
%124 = OpCompositeExtract %v4float %119 1
%125 = OpCompositeExtract %v4float %120 1
%126 = OpFSub %v4float %124 %125
%127 = OpCompositeExtract %v4float %119 2
%128 = OpCompositeExtract %v4float %120 2
%129 = OpFSub %v4float %127 %128
%130 = OpCompositeExtract %v4float %119 3
%131 = OpCompositeExtract %v4float %120 3
%132 = OpFSub %v4float %130 %131
%133 = OpCompositeConstruct %mat4v4float %123 %126 %129 %132
OpStore %m11 %133
OpReturnValue %true
OpFunctionEnd
%test_half = OpFunction %bool None %20
%135 = OpLabel
%v1_0 = OpVariable %_ptr_Function_v3float Function
%v2_0 = OpVariable %_ptr_Function_v3float Function
%m1_0 = OpVariable %_ptr_Function_mat2v2float Function
%m2_0 = OpVariable %_ptr_Function_mat2v2float Function
%m3_0 = OpVariable %_ptr_Function_mat2v2float Function
%m4_0 = OpVariable %_ptr_Function_mat2v2float Function
%m5_0 = OpVariable %_ptr_Function_mat2v2float Function
%m6_0 = OpVariable %_ptr_Function_mat2v2float Function
%m7_0 = OpVariable %_ptr_Function_mat2v2float Function
%m9_0 = OpVariable %_ptr_Function_mat3v3float Function
%m10_0 = OpVariable %_ptr_Function_mat4v4float Function
%m11_0 = OpVariable %_ptr_Function_mat4v4float Function
%138 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%139 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%140 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%137 = OpCompositeConstruct %mat3v3float %138 %139 %140
%141 = OpMatrixTimesVector %v3float %137 %33
OpStore %v1_0 %141
%144 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%145 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%146 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%143 = OpCompositeConstruct %mat3v3float %144 %145 %146
%147 = OpVectorTimesMatrix %v3float %33 %143
OpStore %v2_0 %147
%150 = OpCompositeConstruct %v2float %float_1 %float_2
%151 = OpCompositeConstruct %v2float %float_3 %float_4
%149 = OpCompositeConstruct %mat2v2float %150 %151
OpStore %m1_0 %149
%154 = OpCompositeExtract %float %51 0
%155 = OpCompositeExtract %float %51 1
%156 = OpCompositeExtract %float %51 2
%157 = OpCompositeExtract %float %51 3
%158 = OpCompositeConstruct %v2float %154 %155
%159 = OpCompositeConstruct %v2float %156 %157
%153 = OpCompositeConstruct %mat2v2float %158 %159
OpStore %m2_0 %153
%161 = OpLoad %mat2v2float %m1_0
OpStore %m3_0 %161
%164 = OpCompositeConstruct %v2float %float_1 %float_0
%165 = OpCompositeConstruct %v2float %float_0 %float_1
%163 = OpCompositeConstruct %mat2v2float %164 %165
OpStore %m4_0 %163
%166 = OpLoad %mat2v2float %m3_0
%167 = OpLoad %mat2v2float %m4_0
%168 = OpMatrixTimesMatrix %mat2v2float %166 %167
OpStore %m3_0 %168
%170 = OpAccessChain %_ptr_Function_v2float %m1_0 %int_0
%171 = OpLoad %v2float %170
%172 = OpCompositeExtract %float %171 0
%174 = OpCompositeConstruct %v2float %172 %float_0
%175 = OpCompositeConstruct %v2float %float_0 %172
%173 = OpCompositeConstruct %mat2v2float %174 %175
OpStore %m5_0 %173
%178 = OpCompositeConstruct %v2float %float_1 %float_2
%179 = OpCompositeConstruct %v2float %float_3 %float_4
%177 = OpCompositeConstruct %mat2v2float %178 %179
OpStore %m6_0 %177
%180 = OpLoad %mat2v2float %m6_0
%181 = OpLoad %mat2v2float %m5_0
%182 = OpCompositeExtract %v2float %180 0
%183 = OpCompositeExtract %v2float %181 0
%184 = OpFAdd %v2float %182 %183
%185 = OpCompositeExtract %v2float %180 1
%186 = OpCompositeExtract %v2float %181 1
%187 = OpFAdd %v2float %185 %186
%188 = OpCompositeConstruct %mat2v2float %184 %187
OpStore %m6_0 %188
%191 = OpCompositeConstruct %v2float %float_5 %float_6
%192 = OpCompositeConstruct %v2float %float_7 %float_8
%190 = OpCompositeConstruct %mat2v2float %191 %192
OpStore %m7_0 %190
%195 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%196 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%194 = OpCompositeConstruct %mat3v3float %195 %196 %197
OpStore %m9_0 %194
%200 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%201 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%202 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%203 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%199 = OpCompositeConstruct %mat4v4float %200 %201 %202 %203
OpStore %m10_0 %199
%206 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%207 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%208 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%209 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%205 = OpCompositeConstruct %mat4v4float %206 %207 %208 %209
OpStore %m11_0 %205
%210 = OpLoad %mat4v4float %m11_0
%211 = OpLoad %mat4v4float %m10_0
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
OpStore %m11_0 %224
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %225
%226 = OpLabel
%233 = OpVariable %_ptr_Function_v4float Function
%228 = OpFunctionCall %bool %test_float
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpFunctionCall %bool %test_half
OpBranch %230
%230 = OpLabel
%232 = OpPhi %bool %false %226 %231 %229
OpSelectionMerge %237 None
OpBranchConditional %232 %235 %236
%235 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%240 = OpLoad %v4float %238
OpStore %233 %240
OpBranch %237
%236 = OpLabel
%241 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%243 = OpLoad %v4float %241
OpStore %233 %243
OpBranch %237
%237 = OpLabel
%244 = OpLoad %v4float %233
OpReturnValue %244
OpFunctionEnd
