OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_0_result "_0_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_0_5 = OpConstant %float 0.5
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_9 = OpConstant %float 9
%float_11 = OpConstant %float 11
%18 = OpConstantComposite %v4float %float_6 %float_7 %float_9 %float_11
%23 = OpConstantComposite %v4float %float_7 %float_9 %float_9 %float_9
%float_2 = OpConstant %float 2
%float_4 = OpConstant %float 4
%float_8 = OpConstant %float 8
%24 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
%float_12 = OpConstant %float 12
%float_3 = OpConstant %float 3
%28 = OpConstantComposite %v4float %float_12 %float_6 %float_4 %float_3
%float_6_0 = OpConstant %float 6
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_3_0 = OpConstant %float 3
%float_4_0 = OpConstant %float 4
%float_n5 = OpConstant %float -5
%float_7_0 = OpConstant %float 7
%float_n8 = OpConstant %float -8
%float_9_0 = OpConstant %float 9
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_13 = OpConstant %float 13
%float_n13 = OpConstant %float -13
%float_1_0 = OpConstant %float 1
%float_0 = OpConstant %float 0
%73 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%75 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%85 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%86 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_13_0 = OpConstant %float 13
%92 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%93 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_16 = OpConstant %float 16
%float_17 = OpConstant %float 17
%100 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_19 = OpConstant %float 19
%float_19_5 = OpConstant %float 19.5
%float_20 = OpConstant %float 20
%float_21 = OpConstant %float 21
%float_22 = OpConstant %float 22
%float_23 = OpConstant %float 23
%float_24 = OpConstant %float 24
%123 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%126 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_2_0 = OpConstant %float 2
%129 = OpConstantComposite %v4float %float_2_0 %float_2_0 %float_2_0 %float_2_0
%133 = OpConstantComposite %v4float %float_2_0 %float_2_0 %float_2_0 %float_2_0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_2 = OpConstant %int 2
%_ptr_Function_int = OpTypePointer Function %int
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%int_9 = OpConstant %int 9
%int_11 = OpConstant %int 11
%141 = OpConstantComposite %v4int %int_6 %int_7 %int_9 %int_11
%146 = OpConstantComposite %v4int %int_7 %int_9 %int_9 %int_9
%int_4 = OpConstant %int 4
%int_8 = OpConstant %int 8
%147 = OpConstantComposite %v4int %int_2 %int_4 %int_6 %int_8
%int_12 = OpConstant %int 12
%int_3 = OpConstant %int 3
%150 = OpConstantComposite %v4int %int_12 %int_6 %int_4 %int_3
%170 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%171 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%172 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%185 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%186 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%193 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%194 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%201 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%int_1 = OpConstant %int 1
%224 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%228 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%231 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%234 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%main = OpFunction %void None %11
%12 = OpLabel
%_0_result = OpVariable %_ptr_Function_v4int Function
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_0_5
OpStore %sk_FragColor %18
OpStore %sk_FragColor %23
OpStore %sk_FragColor %24
OpStore %sk_FragColor %28
%32 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %32 %float_6_0
%34 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %34 %float_1
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %float_n2
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %38 %float_3_0
%40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %40 %float_4_0
%42 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %42 %float_n5
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %float_6_0
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %float_7_0
%47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %47 %float_n8
%49 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %49 %float_9_0
%51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %51 %float_n10
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %float_n11
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %55 %float_n12
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %float_13
%58 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %58 %float_n11
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %59 %float_n12
%60 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %60 %float_13
%61 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %61 %float_n11
%62 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %62 %float_n12
%64 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %64 %float_n13
%65 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %65 %float_n11
%66 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %66 %float_n12
%67 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %67 %float_n13
%68 = OpExtInst %float %1 Sqrt %float_1_0
%70 = OpCompositeConstruct %v4float %68 %68 %68 %68
OpStore %sk_FragColor %70
%71 = OpExtInst %float %1 Sqrt %float_2
%72 = OpCompositeConstruct %v4float %71 %71 %71 %71
OpStore %sk_FragColor %72
OpStore %sk_FragColor %73
OpStore %sk_FragColor %75
OpStore %sk_FragColor %76
%77 = OpExtInst %float %1 Sqrt %float_6
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
OpStore %sk_FragColor %78
%79 = OpExtInst %float %1 Sqrt %float_7
%80 = OpCompositeConstruct %v4float %79 %79 %79 %79
OpStore %sk_FragColor %80
%81 = OpExtInst %float %1 Sqrt %float_8
%82 = OpCompositeConstruct %v4float %81 %81 %81 %81
OpStore %sk_FragColor %82
%83 = OpExtInst %float %1 Sqrt %float_9
%84 = OpCompositeConstruct %v4float %83 %83 %83 %83
OpStore %sk_FragColor %84
OpStore %sk_FragColor %85
OpStore %sk_FragColor %86
%87 = OpExtInst %float %1 Sqrt %float_12
%88 = OpCompositeConstruct %v4float %87 %87 %87 %87
OpStore %sk_FragColor %88
%89 = OpExtInst %float %1 Sqrt %float_13_0
%91 = OpCompositeConstruct %v4float %89 %89 %89 %89
OpStore %sk_FragColor %91
OpStore %sk_FragColor %92
OpStore %sk_FragColor %93
%94 = OpExtInst %float %1 Sqrt %float_16
%96 = OpCompositeConstruct %v4float %94 %94 %94 %94
OpStore %sk_FragColor %96
%97 = OpExtInst %float %1 Sqrt %float_17
%99 = OpCompositeConstruct %v4float %97 %97 %97 %97
OpStore %sk_FragColor %99
OpStore %sk_FragColor %100
%101 = OpExtInst %float %1 Sqrt %float_19
%103 = OpCompositeConstruct %v4float %101 %101 %101 %101
OpStore %sk_FragColor %103
%104 = OpExtInst %float %1 Sqrt %float_19_5
%106 = OpCompositeConstruct %v4float %104 %104 %104 %104
OpStore %sk_FragColor %106
%107 = OpExtInst %float %1 Sqrt %float_20
%109 = OpCompositeConstruct %v4float %107 %107 %107 %107
OpStore %sk_FragColor %109
%110 = OpExtInst %float %1 Sqrt %float_21
%112 = OpCompositeConstruct %v4float %110 %110 %110 %110
OpStore %sk_FragColor %112
%113 = OpExtInst %float %1 Sqrt %float_22
%115 = OpCompositeConstruct %v4float %113 %113 %113 %113
OpStore %sk_FragColor %115
%116 = OpExtInst %float %1 Sqrt %float_23
%118 = OpCompositeConstruct %v4float %116 %116 %116 %116
OpStore %sk_FragColor %118
%119 = OpExtInst %float %1 Sqrt %float_24
%121 = OpCompositeConstruct %v4float %119 %119 %119 %119
OpStore %sk_FragColor %121
%122 = OpLoad %v4float %sk_FragColor
%124 = OpFAdd %v4float %122 %123
OpStore %sk_FragColor %124
%125 = OpLoad %v4float %sk_FragColor
%127 = OpFSub %v4float %125 %126
OpStore %sk_FragColor %127
%128 = OpLoad %v4float %sk_FragColor
%131 = OpFMul %v4float %128 %129
OpStore %sk_FragColor %131
%132 = OpLoad %v4float %sk_FragColor
%134 = OpFDiv %v4float %132 %133
OpStore %sk_FragColor %134
%139 = OpAccessChain %_ptr_Function_int %_0_result %int_0
OpStore %139 %int_2
OpStore %_0_result %141
OpStore %_0_result %146
OpStore %_0_result %147
OpStore %_0_result %150
%153 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %153 %float_6_0
%154 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %154 %float_1
%155 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %155 %float_n2
%156 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %156 %float_3_0
%157 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %157 %float_4_0
%158 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %158 %float_n5
%159 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %159 %float_6_0
%160 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %160 %float_7_0
%161 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %161 %float_n8
%162 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %162 %float_9_0
%163 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %163 %float_n10
%165 = OpExtInst %float %1 Sqrt %float_1_0
%164 = OpConvertFToS %int %165
%166 = OpCompositeConstruct %v4int %164 %164 %164 %164
OpStore %_0_result %166
%168 = OpExtInst %float %1 Sqrt %float_2
%167 = OpConvertFToS %int %168
%169 = OpCompositeConstruct %v4int %167 %167 %167 %167
OpStore %_0_result %169
OpStore %_0_result %170
OpStore %_0_result %171
OpStore %_0_result %172
%174 = OpExtInst %float %1 Sqrt %float_6
%173 = OpConvertFToS %int %174
%175 = OpCompositeConstruct %v4int %173 %173 %173 %173
OpStore %_0_result %175
%177 = OpExtInst %float %1 Sqrt %float_7
%176 = OpConvertFToS %int %177
%178 = OpCompositeConstruct %v4int %176 %176 %176 %176
OpStore %_0_result %178
%180 = OpExtInst %float %1 Sqrt %float_8
%179 = OpConvertFToS %int %180
%181 = OpCompositeConstruct %v4int %179 %179 %179 %179
OpStore %_0_result %181
%183 = OpExtInst %float %1 Sqrt %float_9
%182 = OpConvertFToS %int %183
%184 = OpCompositeConstruct %v4int %182 %182 %182 %182
OpStore %_0_result %184
OpStore %_0_result %185
OpStore %_0_result %186
%188 = OpExtInst %float %1 Sqrt %float_12
%187 = OpConvertFToS %int %188
%189 = OpCompositeConstruct %v4int %187 %187 %187 %187
OpStore %_0_result %189
%191 = OpExtInst %float %1 Sqrt %float_13_0
%190 = OpConvertFToS %int %191
%192 = OpCompositeConstruct %v4int %190 %190 %190 %190
OpStore %_0_result %192
OpStore %_0_result %193
OpStore %_0_result %194
%196 = OpExtInst %float %1 Sqrt %float_16
%195 = OpConvertFToS %int %196
%197 = OpCompositeConstruct %v4int %195 %195 %195 %195
OpStore %_0_result %197
%199 = OpExtInst %float %1 Sqrt %float_17
%198 = OpConvertFToS %int %199
%200 = OpCompositeConstruct %v4int %198 %198 %198 %198
OpStore %_0_result %200
OpStore %_0_result %201
%203 = OpExtInst %float %1 Sqrt %float_19
%202 = OpConvertFToS %int %203
%204 = OpCompositeConstruct %v4int %202 %202 %202 %202
OpStore %_0_result %204
%206 = OpExtInst %float %1 Sqrt %float_19_5
%205 = OpConvertFToS %int %206
%207 = OpCompositeConstruct %v4int %205 %205 %205 %205
OpStore %_0_result %207
%209 = OpExtInst %float %1 Sqrt %float_20
%208 = OpConvertFToS %int %209
%210 = OpCompositeConstruct %v4int %208 %208 %208 %208
OpStore %_0_result %210
%212 = OpExtInst %float %1 Sqrt %float_21
%211 = OpConvertFToS %int %212
%213 = OpCompositeConstruct %v4int %211 %211 %211 %211
OpStore %_0_result %213
%215 = OpExtInst %float %1 Sqrt %float_22
%214 = OpConvertFToS %int %215
%216 = OpCompositeConstruct %v4int %214 %214 %214 %214
OpStore %_0_result %216
%218 = OpExtInst %float %1 Sqrt %float_23
%217 = OpConvertFToS %int %218
%219 = OpCompositeConstruct %v4int %217 %217 %217 %217
OpStore %_0_result %219
%221 = OpExtInst %float %1 Sqrt %float_24
%220 = OpConvertFToS %int %221
%222 = OpCompositeConstruct %v4int %220 %220 %220 %220
OpStore %_0_result %222
%223 = OpLoad %v4int %_0_result
%226 = OpIAdd %v4int %223 %224
OpStore %_0_result %226
%227 = OpLoad %v4int %_0_result
%229 = OpISub %v4int %227 %228
OpStore %_0_result %229
%230 = OpLoad %v4int %_0_result
%232 = OpIMul %v4int %230 %231
OpStore %_0_result %232
%233 = OpLoad %v4int %_0_result
%235 = OpSDiv %v4int %233 %234
OpStore %_0_result %235
%236 = OpLoad %v4int %_0_result
%237 = OpCompositeExtract %int %236 0
%238 = OpConvertSToF %float %237
%239 = OpCompositeExtract %int %236 1
%240 = OpConvertSToF %float %239
%241 = OpCompositeExtract %int %236 2
%242 = OpConvertSToF %float %241
%243 = OpCompositeExtract %int %236 3
%244 = OpConvertSToF %float %243
%245 = OpCompositeConstruct %v4float %238 %240 %242 %244
OpStore %sk_FragColor %245
OpReturn
OpFunctionEnd
