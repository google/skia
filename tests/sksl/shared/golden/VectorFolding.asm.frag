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
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_n5 = OpConstant %float -5
%float_n8 = OpConstant %float -8
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_13 = OpConstant %float 13
%float_n13 = OpConstant %float -13
%float_0 = OpConstant %float 0
%67 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%69 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%70 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%79 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%80 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%85 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%86 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_16 = OpConstant %float 16
%float_17 = OpConstant %float 17
%93 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_19 = OpConstant %float 19
%float_19_5 = OpConstant %float 19.5
%float_20 = OpConstant %float 20
%float_21 = OpConstant %float 21
%float_22 = OpConstant %float 22
%float_23 = OpConstant %float 23
%float_24 = OpConstant %float 24
%116 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%119 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%122 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%125 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_2 = OpConstant %int 2
%_ptr_Function_int = OpTypePointer Function %int
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%int_9 = OpConstant %int 9
%int_11 = OpConstant %int 11
%133 = OpConstantComposite %v4int %int_6 %int_7 %int_9 %int_11
%138 = OpConstantComposite %v4int %int_7 %int_9 %int_9 %int_9
%int_4 = OpConstant %int 4
%int_8 = OpConstant %int 8
%139 = OpConstantComposite %v4int %int_2 %int_4 %int_6 %int_8
%int_12 = OpConstant %int 12
%int_3 = OpConstant %int 3
%142 = OpConstantComposite %v4int %int_12 %int_6 %int_4 %int_3
%162 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%163 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%164 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%177 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%178 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%185 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%186 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%193 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%int_1 = OpConstant %int 1
%216 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%220 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%223 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%226 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%main = OpFunction %void None %11
%12 = OpLabel
%_0_result = OpVariable %_ptr_Function_v4int Function
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_0_5
OpStore %sk_FragColor %18
OpStore %sk_FragColor %23
OpStore %sk_FragColor %24
OpStore %sk_FragColor %28
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %31 %float_6
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %float_1
%35 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %35 %float_n2
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %float_3
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %float_4
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %float_n5
%40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %40 %float_6
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %float_7
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %float_n8
%44 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %44 %float_9
%46 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %46 %float_n10
%48 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %48 %float_n11
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %float_n12
%52 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %52 %float_13
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %53 %float_n11
%54 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %54 %float_n12
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %55 %float_13
%56 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %56 %float_n11
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %float_n12
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %59 %float_n13
%60 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %60 %float_n11
%61 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %61 %float_n12
%62 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %62 %float_n13
%63 = OpExtInst %float %1 Sqrt %float_1
%64 = OpCompositeConstruct %v4float %63 %63 %63 %63
OpStore %sk_FragColor %64
%65 = OpExtInst %float %1 Sqrt %float_2
%66 = OpCompositeConstruct %v4float %65 %65 %65 %65
OpStore %sk_FragColor %66
OpStore %sk_FragColor %67
OpStore %sk_FragColor %69
OpStore %sk_FragColor %70
%71 = OpExtInst %float %1 Sqrt %float_6
%72 = OpCompositeConstruct %v4float %71 %71 %71 %71
OpStore %sk_FragColor %72
%73 = OpExtInst %float %1 Sqrt %float_7
%74 = OpCompositeConstruct %v4float %73 %73 %73 %73
OpStore %sk_FragColor %74
%75 = OpExtInst %float %1 Sqrt %float_8
%76 = OpCompositeConstruct %v4float %75 %75 %75 %75
OpStore %sk_FragColor %76
%77 = OpExtInst %float %1 Sqrt %float_9
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
OpStore %sk_FragColor %78
OpStore %sk_FragColor %79
OpStore %sk_FragColor %80
%81 = OpExtInst %float %1 Sqrt %float_12
%82 = OpCompositeConstruct %v4float %81 %81 %81 %81
OpStore %sk_FragColor %82
%83 = OpExtInst %float %1 Sqrt %float_13
%84 = OpCompositeConstruct %v4float %83 %83 %83 %83
OpStore %sk_FragColor %84
OpStore %sk_FragColor %85
OpStore %sk_FragColor %86
%87 = OpExtInst %float %1 Sqrt %float_16
%89 = OpCompositeConstruct %v4float %87 %87 %87 %87
OpStore %sk_FragColor %89
%90 = OpExtInst %float %1 Sqrt %float_17
%92 = OpCompositeConstruct %v4float %90 %90 %90 %90
OpStore %sk_FragColor %92
OpStore %sk_FragColor %93
%94 = OpExtInst %float %1 Sqrt %float_19
%96 = OpCompositeConstruct %v4float %94 %94 %94 %94
OpStore %sk_FragColor %96
%97 = OpExtInst %float %1 Sqrt %float_19_5
%99 = OpCompositeConstruct %v4float %97 %97 %97 %97
OpStore %sk_FragColor %99
%100 = OpExtInst %float %1 Sqrt %float_20
%102 = OpCompositeConstruct %v4float %100 %100 %100 %100
OpStore %sk_FragColor %102
%103 = OpExtInst %float %1 Sqrt %float_21
%105 = OpCompositeConstruct %v4float %103 %103 %103 %103
OpStore %sk_FragColor %105
%106 = OpExtInst %float %1 Sqrt %float_22
%108 = OpCompositeConstruct %v4float %106 %106 %106 %106
OpStore %sk_FragColor %108
%109 = OpExtInst %float %1 Sqrt %float_23
%111 = OpCompositeConstruct %v4float %109 %109 %109 %109
OpStore %sk_FragColor %111
%112 = OpExtInst %float %1 Sqrt %float_24
%114 = OpCompositeConstruct %v4float %112 %112 %112 %112
OpStore %sk_FragColor %114
%115 = OpLoad %v4float %sk_FragColor
%117 = OpFAdd %v4float %115 %116
OpStore %sk_FragColor %117
%118 = OpLoad %v4float %sk_FragColor
%120 = OpFSub %v4float %118 %119
OpStore %sk_FragColor %120
%121 = OpLoad %v4float %sk_FragColor
%123 = OpFMul %v4float %121 %122
OpStore %sk_FragColor %123
%124 = OpLoad %v4float %sk_FragColor
%126 = OpFDiv %v4float %124 %125
OpStore %sk_FragColor %126
%131 = OpAccessChain %_ptr_Function_int %_0_result %int_0
OpStore %131 %int_2
OpStore %_0_result %133
OpStore %_0_result %138
OpStore %_0_result %139
OpStore %_0_result %142
%145 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %145 %float_6
%146 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %146 %float_1
%147 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %147 %float_n2
%148 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %148 %float_3
%149 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %149 %float_4
%150 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %150 %float_n5
%151 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %151 %float_6
%152 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %152 %float_7
%153 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %153 %float_n8
%154 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %154 %float_9
%155 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %155 %float_n10
%157 = OpExtInst %float %1 Sqrt %float_1
%156 = OpConvertFToS %int %157
%158 = OpCompositeConstruct %v4int %156 %156 %156 %156
OpStore %_0_result %158
%160 = OpExtInst %float %1 Sqrt %float_2
%159 = OpConvertFToS %int %160
%161 = OpCompositeConstruct %v4int %159 %159 %159 %159
OpStore %_0_result %161
OpStore %_0_result %162
OpStore %_0_result %163
OpStore %_0_result %164
%166 = OpExtInst %float %1 Sqrt %float_6
%165 = OpConvertFToS %int %166
%167 = OpCompositeConstruct %v4int %165 %165 %165 %165
OpStore %_0_result %167
%169 = OpExtInst %float %1 Sqrt %float_7
%168 = OpConvertFToS %int %169
%170 = OpCompositeConstruct %v4int %168 %168 %168 %168
OpStore %_0_result %170
%172 = OpExtInst %float %1 Sqrt %float_8
%171 = OpConvertFToS %int %172
%173 = OpCompositeConstruct %v4int %171 %171 %171 %171
OpStore %_0_result %173
%175 = OpExtInst %float %1 Sqrt %float_9
%174 = OpConvertFToS %int %175
%176 = OpCompositeConstruct %v4int %174 %174 %174 %174
OpStore %_0_result %176
OpStore %_0_result %177
OpStore %_0_result %178
%180 = OpExtInst %float %1 Sqrt %float_12
%179 = OpConvertFToS %int %180
%181 = OpCompositeConstruct %v4int %179 %179 %179 %179
OpStore %_0_result %181
%183 = OpExtInst %float %1 Sqrt %float_13
%182 = OpConvertFToS %int %183
%184 = OpCompositeConstruct %v4int %182 %182 %182 %182
OpStore %_0_result %184
OpStore %_0_result %185
OpStore %_0_result %186
%188 = OpExtInst %float %1 Sqrt %float_16
%187 = OpConvertFToS %int %188
%189 = OpCompositeConstruct %v4int %187 %187 %187 %187
OpStore %_0_result %189
%191 = OpExtInst %float %1 Sqrt %float_17
%190 = OpConvertFToS %int %191
%192 = OpCompositeConstruct %v4int %190 %190 %190 %190
OpStore %_0_result %192
OpStore %_0_result %193
%195 = OpExtInst %float %1 Sqrt %float_19
%194 = OpConvertFToS %int %195
%196 = OpCompositeConstruct %v4int %194 %194 %194 %194
OpStore %_0_result %196
%198 = OpExtInst %float %1 Sqrt %float_19_5
%197 = OpConvertFToS %int %198
%199 = OpCompositeConstruct %v4int %197 %197 %197 %197
OpStore %_0_result %199
%201 = OpExtInst %float %1 Sqrt %float_20
%200 = OpConvertFToS %int %201
%202 = OpCompositeConstruct %v4int %200 %200 %200 %200
OpStore %_0_result %202
%204 = OpExtInst %float %1 Sqrt %float_21
%203 = OpConvertFToS %int %204
%205 = OpCompositeConstruct %v4int %203 %203 %203 %203
OpStore %_0_result %205
%207 = OpExtInst %float %1 Sqrt %float_22
%206 = OpConvertFToS %int %207
%208 = OpCompositeConstruct %v4int %206 %206 %206 %206
OpStore %_0_result %208
%210 = OpExtInst %float %1 Sqrt %float_23
%209 = OpConvertFToS %int %210
%211 = OpCompositeConstruct %v4int %209 %209 %209 %209
OpStore %_0_result %211
%213 = OpExtInst %float %1 Sqrt %float_24
%212 = OpConvertFToS %int %213
%214 = OpCompositeConstruct %v4int %212 %212 %212 %212
OpStore %_0_result %214
%215 = OpLoad %v4int %_0_result
%218 = OpIAdd %v4int %215 %216
OpStore %_0_result %218
%219 = OpLoad %v4int %_0_result
%221 = OpISub %v4int %219 %220
OpStore %_0_result %221
%222 = OpLoad %v4int %_0_result
%224 = OpIMul %v4int %222 %223
OpStore %_0_result %224
%225 = OpLoad %v4int %_0_result
%227 = OpSDiv %v4int %225 %226
OpStore %_0_result %227
%228 = OpLoad %v4int %_0_result
%229 = OpCompositeExtract %int %228 0
%230 = OpConvertSToF %float %229
%231 = OpCompositeExtract %int %228 1
%232 = OpConvertSToF %float %231
%233 = OpCompositeExtract %int %228 2
%234 = OpConvertSToF %float %233
%235 = OpCompositeExtract %int %228 3
%236 = OpConvertSToF %float %235
%237 = OpCompositeConstruct %v4float %230 %232 %234 %236
OpStore %sk_FragColor %237
OpReturn
OpFunctionEnd
