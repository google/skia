OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct_S "returns_a_struct_S"
OpName %s "s"
OpName %constructs_a_struct_S "constructs_a_struct_S"
OpName %accepts_a_struct_fS "accepts_a_struct_fS"
OpName %modifies_a_struct_vS "modifies_a_struct_vS"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %expected "expected"
OpName %Nested "Nested"
OpMemberName %Nested 0 "a"
OpMemberName %Nested 1 "b"
OpName %n1 "n1"
OpName %n2 "n2"
OpName %n3 "n3"
OpName %Compound "Compound"
OpMemberName %Compound 0 "f4"
OpMemberName %Compound 1 "i3"
OpName %c1 "c1"
OpName %c2 "c2"
OpName %c3 "c3"
OpName %valid "valid"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %41 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 1 Offset 16
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%29 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%46 = OpTypeFunction %float %_ptr_Function_S
%55 = OpTypeFunction %void %_ptr_Function_S
%64 = OpTypeFunction %v4float %_ptr_Function_v2float
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%v3int = OpTypeVector %int 3
%Compound = OpTypeStruct %v4float %v3int
%_ptr_Function_Compound = OpTypePointer Function %Compound
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%99 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%103 = OpConstantComposite %v3int %int_5 %int_6 %int_7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %29
%30 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%35 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %35 %float_1
%39 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %39 %int_2
%41 = OpLoad %S %s
OpReturnValue %41
OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %29
%42 = OpLabel
%45 = OpCompositeConstruct %S %float_2 %int_3
OpReturnValue %45
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %46
%47 = OpFunctionParameter %_ptr_Function_S
%48 = OpLabel
%49 = OpAccessChain %_ptr_Function_float %47 %int_0
%50 = OpLoad %float %49
%51 = OpAccessChain %_ptr_Function_int %47 %int_1
%52 = OpLoad %int %51
%53 = OpConvertSToF %float %52
%54 = OpFAdd %float %50 %53
OpReturnValue %54
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %55
%56 = OpFunctionParameter %_ptr_Function_S
%57 = OpLabel
%58 = OpAccessChain %_ptr_Function_float %56 %int_0
%59 = OpLoad %float %58
%60 = OpFAdd %float %59 %float_1
OpStore %58 %60
%61 = OpAccessChain %_ptr_Function_int %56 %int_1
%62 = OpLoad %int %61
%63 = OpIAdd %int %62 %int_1
OpStore %61 %63
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %64
%65 = OpFunctionParameter %_ptr_Function_v2float
%66 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%71 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%90 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%276 = OpVariable %_ptr_Function_v4float Function
%68 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %68
%70 = OpLoad %S %s_0
OpStore %71 %70
%72 = OpFunctionCall %float %accepts_a_struct_fS %71
OpStore %x %72
%73 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%75 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %75
%81 = OpFunctionCall %S %returns_a_struct_S
%82 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %82 %81
%83 = OpAccessChain %_ptr_Function_S %n1 %int_0
%84 = OpLoad %S %83
%85 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %85 %84
%86 = OpLoad %Nested %n1
OpStore %n2 %86
%87 = OpLoad %Nested %n2
OpStore %n3 %87
%88 = OpAccessChain %_ptr_Function_S %n3 %int_1
%89 = OpLoad %S %88
OpStore %90 %89
%91 = OpFunctionCall %void %modifies_a_struct_vS %90
%92 = OpLoad %S %90
OpStore %88 %92
%104 = OpCompositeConstruct %Compound %99 %103
OpStore %c1 %104
%106 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%108 = OpLoad %v4float %106
%109 = OpCompositeExtract %float %108 1
%110 = OpCompositeConstruct %v4float %109 %float_2 %float_3 %float_4
%111 = OpCompositeConstruct %Compound %110 %103
OpStore %c2 %111
%113 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%114 = OpLoad %v4float %113
%115 = OpCompositeExtract %float %114 0
%116 = OpCompositeConstruct %v4float %115 %float_2 %float_3 %float_4
%117 = OpCompositeConstruct %Compound %116 %103
OpStore %c3 %117
%121 = OpLoad %float %x
%122 = OpFOrdEqual %bool %121 %float_3
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%126 = OpLoad %float %125
%127 = OpFOrdEqual %bool %126 %float_2
OpBranch %124
%124 = OpLabel
%128 = OpPhi %bool %false %66 %127 %123
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%132 = OpLoad %int %131
%133 = OpIEqual %bool %132 %int_3
OpBranch %130
%130 = OpLabel
%134 = OpPhi %bool %false %124 %133 %129
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpLoad %S %s_0
%138 = OpLoad %S %expected
%139 = OpCompositeExtract %float %137 0
%140 = OpCompositeExtract %float %138 0
%141 = OpFOrdEqual %bool %139 %140
%142 = OpCompositeExtract %int %137 1
%143 = OpCompositeExtract %int %138 1
%144 = OpIEqual %bool %142 %143
%145 = OpLogicalAnd %bool %144 %141
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %130 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %S %s_0
%150 = OpCompositeConstruct %S %float_2 %int_3
%151 = OpCompositeExtract %float %149 0
%152 = OpCompositeExtract %float %150 0
%153 = OpFOrdEqual %bool %151 %152
%154 = OpCompositeExtract %int %149 1
%155 = OpCompositeExtract %int %150 1
%156 = OpIEqual %bool %154 %155
%157 = OpLogicalAnd %bool %156 %153
OpBranch %148
%148 = OpLabel
%158 = OpPhi %bool %false %136 %157 %147
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %S %s_0
%162 = OpFunctionCall %S %returns_a_struct_S
%163 = OpCompositeExtract %float %161 0
%164 = OpCompositeExtract %float %162 0
%165 = OpFOrdNotEqual %bool %163 %164
%166 = OpCompositeExtract %int %161 1
%167 = OpCompositeExtract %int %162 1
%168 = OpINotEqual %bool %166 %167
%169 = OpLogicalOr %bool %168 %165
OpBranch %160
%160 = OpLabel
%170 = OpPhi %bool %false %148 %169 %159
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %Nested %n1
%174 = OpLoad %Nested %n2
%175 = OpCompositeExtract %S %173 0
%176 = OpCompositeExtract %S %174 0
%177 = OpCompositeExtract %float %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpFOrdEqual %bool %177 %178
%180 = OpCompositeExtract %int %175 1
%181 = OpCompositeExtract %int %176 1
%182 = OpIEqual %bool %180 %181
%183 = OpLogicalAnd %bool %182 %179
%184 = OpCompositeExtract %S %173 1
%185 = OpCompositeExtract %S %174 1
%186 = OpCompositeExtract %float %184 0
%187 = OpCompositeExtract %float %185 0
%188 = OpFOrdEqual %bool %186 %187
%189 = OpCompositeExtract %int %184 1
%190 = OpCompositeExtract %int %185 1
%191 = OpIEqual %bool %189 %190
%192 = OpLogicalAnd %bool %191 %188
%193 = OpLogicalAnd %bool %192 %183
OpBranch %172
%172 = OpLabel
%194 = OpPhi %bool %false %160 %193 %171
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %Nested %n1
%198 = OpLoad %Nested %n3
%199 = OpCompositeExtract %S %197 0
%200 = OpCompositeExtract %S %198 0
%201 = OpCompositeExtract %float %199 0
%202 = OpCompositeExtract %float %200 0
%203 = OpFOrdNotEqual %bool %201 %202
%204 = OpCompositeExtract %int %199 1
%205 = OpCompositeExtract %int %200 1
%206 = OpINotEqual %bool %204 %205
%207 = OpLogicalOr %bool %206 %203
%208 = OpCompositeExtract %S %197 1
%209 = OpCompositeExtract %S %198 1
%210 = OpCompositeExtract %float %208 0
%211 = OpCompositeExtract %float %209 0
%212 = OpFOrdNotEqual %bool %210 %211
%213 = OpCompositeExtract %int %208 1
%214 = OpCompositeExtract %int %209 1
%215 = OpINotEqual %bool %213 %214
%216 = OpLogicalOr %bool %215 %212
%217 = OpLogicalOr %bool %216 %207
OpBranch %196
%196 = OpLabel
%218 = OpPhi %bool %false %172 %217 %195
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %Nested %n3
%222 = OpCompositeConstruct %S %float_1 %int_2
%223 = OpCompositeConstruct %S %float_2 %int_3
%224 = OpCompositeConstruct %Nested %222 %223
%225 = OpCompositeExtract %S %221 0
%226 = OpCompositeExtract %S %224 0
%227 = OpCompositeExtract %float %225 0
%228 = OpCompositeExtract %float %226 0
%229 = OpFOrdEqual %bool %227 %228
%230 = OpCompositeExtract %int %225 1
%231 = OpCompositeExtract %int %226 1
%232 = OpIEqual %bool %230 %231
%233 = OpLogicalAnd %bool %232 %229
%234 = OpCompositeExtract %S %221 1
%235 = OpCompositeExtract %S %224 1
%236 = OpCompositeExtract %float %234 0
%237 = OpCompositeExtract %float %235 0
%238 = OpFOrdEqual %bool %236 %237
%239 = OpCompositeExtract %int %234 1
%240 = OpCompositeExtract %int %235 1
%241 = OpIEqual %bool %239 %240
%242 = OpLogicalAnd %bool %241 %238
%243 = OpLogicalAnd %bool %242 %233
OpBranch %220
%220 = OpLabel
%244 = OpPhi %bool %false %196 %243 %219
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%247 = OpLoad %Compound %c1
%248 = OpLoad %Compound %c2
%249 = OpCompositeExtract %v4float %247 0
%250 = OpCompositeExtract %v4float %248 0
%251 = OpFOrdEqual %v4bool %249 %250
%253 = OpAll %bool %251
%254 = OpCompositeExtract %v3int %247 1
%255 = OpCompositeExtract %v3int %248 1
%256 = OpIEqual %v3bool %254 %255
%258 = OpAll %bool %256
%259 = OpLogicalAnd %bool %258 %253
OpBranch %246
%246 = OpLabel
%260 = OpPhi %bool %false %220 %259 %245
OpSelectionMerge %262 None
OpBranchConditional %260 %261 %262
%261 = OpLabel
%263 = OpLoad %Compound %c2
%264 = OpLoad %Compound %c3
%265 = OpCompositeExtract %v4float %263 0
%266 = OpCompositeExtract %v4float %264 0
%267 = OpFOrdNotEqual %v4bool %265 %266
%268 = OpAny %bool %267
%269 = OpCompositeExtract %v3int %263 1
%270 = OpCompositeExtract %v3int %264 1
%271 = OpINotEqual %v3bool %269 %270
%272 = OpAny %bool %271
%273 = OpLogicalOr %bool %272 %268
OpBranch %262
%262 = OpLabel
%274 = OpPhi %bool %false %246 %273 %261
OpStore %valid %274
%275 = OpLoad %bool %valid
OpSelectionMerge %280 None
OpBranchConditional %275 %278 %279
%278 = OpLabel
%281 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%282 = OpLoad %v4float %281
OpStore %276 %282
OpBranch %280
%279 = OpLabel
%283 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%284 = OpLoad %v4float %283
OpStore %276 %284
OpBranch %280
%280 = OpLabel
%285 = OpLoad %v4float %276
OpReturnValue %285
OpFunctionEnd
