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
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%22 = OpConstantComposite %v4float %float_6 %float_7 %float_9 %float_11
%23 = OpConstantComposite %v4float %float_7 %float_9 %float_9 %float_9
%float_2 = OpConstant %float 2
%float_4 = OpConstant %float 4
%float_8 = OpConstant %float 8
%27 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
%float_12 = OpConstant %float 12
%float_3 = OpConstant %float 3
%30 = OpConstantComposite %v4float %float_12 %float_6 %float_4 %float_3
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
%68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_16 = OpConstant %float 16
%float_17 = OpConstant %float 17
%float_19 = OpConstant %float 19
%float_19_5 = OpConstant %float 19.5
%float_20 = OpConstant %float 20
%float_21 = OpConstant %float 21
%float_22 = OpConstant %float 22
%float_23 = OpConstant %float 23
%float_24 = OpConstant %float 24
%109 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%114 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_2 = OpConstant %int 2
%_ptr_Function_int = OpTypePointer Function %int
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%int_9 = OpConstant %int 9
%int_11 = OpConstant %int 11
%128 = OpConstantComposite %v4int %int_6 %int_7 %int_9 %int_11
%129 = OpConstantComposite %v4int %int_7 %int_9 %int_9 %int_9
%int_4 = OpConstant %int 4
%int_8 = OpConstant %int 8
%132 = OpConstantComposite %v4int %int_2 %int_4 %int_6 %int_8
%int_12 = OpConstant %int 12
%int_3 = OpConstant %int 3
%135 = OpConstantComposite %v4int %int_12 %int_6 %int_4 %int_3
%153 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
%int_1 = OpConstant %int 1
%201 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%206 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%main = OpFunction %void None %11
%12 = OpLabel
%_0_result = OpVariable %_ptr_Function_v4int Function
%14 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %14 %float_0_5
OpStore %sk_FragColor %22
OpStore %sk_FragColor %23
OpStore %sk_FragColor %27
OpStore %sk_FragColor %30
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
OpStore %sk_FragColor %68
OpStore %sk_FragColor %68
OpStore %sk_FragColor %68
%69 = OpExtInst %float %1 Sqrt %float_6
%70 = OpCompositeConstruct %v4float %69 %69 %69 %69
OpStore %sk_FragColor %70
%71 = OpExtInst %float %1 Sqrt %float_7
%72 = OpCompositeConstruct %v4float %71 %71 %71 %71
OpStore %sk_FragColor %72
%73 = OpExtInst %float %1 Sqrt %float_8
%74 = OpCompositeConstruct %v4float %73 %73 %73 %73
OpStore %sk_FragColor %74
%75 = OpExtInst %float %1 Sqrt %float_9
%76 = OpCompositeConstruct %v4float %75 %75 %75 %75
OpStore %sk_FragColor %76
OpStore %sk_FragColor %68
OpStore %sk_FragColor %68
%77 = OpExtInst %float %1 Sqrt %float_12
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
OpStore %sk_FragColor %78
%79 = OpExtInst %float %1 Sqrt %float_13
%80 = OpCompositeConstruct %v4float %79 %79 %79 %79
OpStore %sk_FragColor %80
OpStore %sk_FragColor %68
OpStore %sk_FragColor %68
%81 = OpExtInst %float %1 Sqrt %float_16
%83 = OpCompositeConstruct %v4float %81 %81 %81 %81
OpStore %sk_FragColor %83
%84 = OpExtInst %float %1 Sqrt %float_17
%86 = OpCompositeConstruct %v4float %84 %84 %84 %84
OpStore %sk_FragColor %86
OpStore %sk_FragColor %68
%87 = OpExtInst %float %1 Sqrt %float_19
%89 = OpCompositeConstruct %v4float %87 %87 %87 %87
OpStore %sk_FragColor %89
%90 = OpExtInst %float %1 Sqrt %float_19_5
%92 = OpCompositeConstruct %v4float %90 %90 %90 %90
OpStore %sk_FragColor %92
%93 = OpExtInst %float %1 Sqrt %float_20
%95 = OpCompositeConstruct %v4float %93 %93 %93 %93
OpStore %sk_FragColor %95
%96 = OpExtInst %float %1 Sqrt %float_21
%98 = OpCompositeConstruct %v4float %96 %96 %96 %96
OpStore %sk_FragColor %98
%99 = OpExtInst %float %1 Sqrt %float_22
%101 = OpCompositeConstruct %v4float %99 %99 %99 %99
OpStore %sk_FragColor %101
%102 = OpExtInst %float %1 Sqrt %float_23
%104 = OpCompositeConstruct %v4float %102 %102 %102 %102
OpStore %sk_FragColor %104
%105 = OpExtInst %float %1 Sqrt %float_24
%107 = OpCompositeConstruct %v4float %105 %105 %105 %105
OpStore %sk_FragColor %107
%108 = OpLoad %v4float %sk_FragColor
%110 = OpFAdd %v4float %108 %109
OpStore %sk_FragColor %110
%111 = OpLoad %v4float %sk_FragColor
%112 = OpFSub %v4float %111 %109
OpStore %sk_FragColor %112
%113 = OpLoad %v4float %sk_FragColor
%115 = OpFMul %v4float %113 %114
OpStore %sk_FragColor %115
%116 = OpLoad %v4float %sk_FragColor
%117 = OpFDiv %v4float %116 %114
OpStore %sk_FragColor %117
%122 = OpAccessChain %_ptr_Function_int %_0_result %int_0
OpStore %122 %int_2
OpStore %_0_result %128
OpStore %_0_result %129
OpStore %_0_result %132
OpStore %_0_result %135
%136 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %136 %float_6
%137 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %137 %float_1
%138 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %138 %float_n2
%139 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %139 %float_3
%140 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %140 %float_4
%141 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %141 %float_n5
%142 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %142 %float_6
%143 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %143 %float_7
%144 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %144 %float_n8
%145 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %145 %float_9
%146 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %146 %float_n10
%148 = OpExtInst %float %1 Sqrt %float_1
%147 = OpConvertFToS %int %148
%149 = OpCompositeConstruct %v4int %147 %147 %147 %147
OpStore %_0_result %149
%151 = OpExtInst %float %1 Sqrt %float_2
%150 = OpConvertFToS %int %151
%152 = OpCompositeConstruct %v4int %150 %150 %150 %150
OpStore %_0_result %152
OpStore %_0_result %153
OpStore %_0_result %153
OpStore %_0_result %153
%155 = OpExtInst %float %1 Sqrt %float_6
%154 = OpConvertFToS %int %155
%156 = OpCompositeConstruct %v4int %154 %154 %154 %154
OpStore %_0_result %156
%158 = OpExtInst %float %1 Sqrt %float_7
%157 = OpConvertFToS %int %158
%159 = OpCompositeConstruct %v4int %157 %157 %157 %157
OpStore %_0_result %159
%161 = OpExtInst %float %1 Sqrt %float_8
%160 = OpConvertFToS %int %161
%162 = OpCompositeConstruct %v4int %160 %160 %160 %160
OpStore %_0_result %162
%164 = OpExtInst %float %1 Sqrt %float_9
%163 = OpConvertFToS %int %164
%165 = OpCompositeConstruct %v4int %163 %163 %163 %163
OpStore %_0_result %165
OpStore %_0_result %153
OpStore %_0_result %153
%167 = OpExtInst %float %1 Sqrt %float_12
%166 = OpConvertFToS %int %167
%168 = OpCompositeConstruct %v4int %166 %166 %166 %166
OpStore %_0_result %168
%170 = OpExtInst %float %1 Sqrt %float_13
%169 = OpConvertFToS %int %170
%171 = OpCompositeConstruct %v4int %169 %169 %169 %169
OpStore %_0_result %171
OpStore %_0_result %153
OpStore %_0_result %153
%173 = OpExtInst %float %1 Sqrt %float_16
%172 = OpConvertFToS %int %173
%174 = OpCompositeConstruct %v4int %172 %172 %172 %172
OpStore %_0_result %174
%176 = OpExtInst %float %1 Sqrt %float_17
%175 = OpConvertFToS %int %176
%177 = OpCompositeConstruct %v4int %175 %175 %175 %175
OpStore %_0_result %177
OpStore %_0_result %153
%179 = OpExtInst %float %1 Sqrt %float_19
%178 = OpConvertFToS %int %179
%180 = OpCompositeConstruct %v4int %178 %178 %178 %178
OpStore %_0_result %180
%182 = OpExtInst %float %1 Sqrt %float_19_5
%181 = OpConvertFToS %int %182
%183 = OpCompositeConstruct %v4int %181 %181 %181 %181
OpStore %_0_result %183
%185 = OpExtInst %float %1 Sqrt %float_20
%184 = OpConvertFToS %int %185
%186 = OpCompositeConstruct %v4int %184 %184 %184 %184
OpStore %_0_result %186
%188 = OpExtInst %float %1 Sqrt %float_21
%187 = OpConvertFToS %int %188
%189 = OpCompositeConstruct %v4int %187 %187 %187 %187
OpStore %_0_result %189
%191 = OpExtInst %float %1 Sqrt %float_22
%190 = OpConvertFToS %int %191
%192 = OpCompositeConstruct %v4int %190 %190 %190 %190
OpStore %_0_result %192
%194 = OpExtInst %float %1 Sqrt %float_23
%193 = OpConvertFToS %int %194
%195 = OpCompositeConstruct %v4int %193 %193 %193 %193
OpStore %_0_result %195
%197 = OpExtInst %float %1 Sqrt %float_24
%196 = OpConvertFToS %int %197
%198 = OpCompositeConstruct %v4int %196 %196 %196 %196
OpStore %_0_result %198
%199 = OpLoad %v4int %_0_result
%202 = OpIAdd %v4int %199 %201
OpStore %_0_result %202
%203 = OpLoad %v4int %_0_result
%204 = OpISub %v4int %203 %201
OpStore %_0_result %204
%205 = OpLoad %v4int %_0_result
%207 = OpIMul %v4int %205 %206
OpStore %_0_result %207
%208 = OpLoad %v4int %_0_result
%209 = OpSDiv %v4int %208 %206
OpStore %_0_result %209
%210 = OpLoad %v4int %_0_result
%211 = OpCompositeExtract %int %210 0
%212 = OpConvertSToF %float %211
%213 = OpCompositeExtract %int %210 1
%214 = OpConvertSToF %float %213
%215 = OpCompositeExtract %int %210 2
%216 = OpConvertSToF %float %215
%217 = OpCompositeExtract %int %210 3
%218 = OpConvertSToF %float %217
%219 = OpCompositeConstruct %v4float %212 %214 %216 %218
OpStore %sk_FragColor %219
OpReturn
OpFunctionEnd
