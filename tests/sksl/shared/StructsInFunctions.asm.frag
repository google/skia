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
OpDecorate %73 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpMemberDecorate %Compound 0 Offset 0
OpMemberDecorate %Compound 1 Offset 16
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%102 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_7 = OpConstant %int 7
%106 = OpConstantComposite %v3int %int_5 %int_6 %int_7
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
%74 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%93 = OpVariable %_ptr_Function_S Function
%c1 = OpVariable %_ptr_Function_Compound Function
%c2 = OpVariable %_ptr_Function_Compound Function
%c3 = OpVariable %_ptr_Function_Compound Function
%valid = OpVariable %_ptr_Function_bool Function
%271 = OpVariable %_ptr_Function_v4float Function
%68 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %68
%70 = OpLoad %S %s_0
OpStore %71 %70
%72 = OpFunctionCall %float %accepts_a_struct_fS %71
OpStore %x %72
%73 = OpLoad %S %s_0
OpStore %74 %73
%75 = OpFunctionCall %void %modifies_a_struct_vS %74
%76 = OpLoad %S %74
OpStore %s_0 %76
%78 = OpFunctionCall %S %constructs_a_struct_S
OpStore %expected %78
%84 = OpFunctionCall %S %returns_a_struct_S
%85 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %85 %84
%86 = OpAccessChain %_ptr_Function_S %n1 %int_0
%87 = OpLoad %S %86
%88 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %88 %87
%89 = OpLoad %Nested %n1
OpStore %n2 %89
%90 = OpLoad %Nested %n2
OpStore %n3 %90
%91 = OpAccessChain %_ptr_Function_S %n3 %int_1
%92 = OpLoad %S %91
OpStore %93 %92
%94 = OpFunctionCall %void %modifies_a_struct_vS %93
%95 = OpLoad %S %93
OpStore %91 %95
%107 = OpCompositeConstruct %Compound %102 %106
OpStore %c1 %107
%109 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%111 = OpLoad %v4float %109
%112 = OpCompositeExtract %float %111 1
%113 = OpCompositeConstruct %v4float %112 %float_2 %float_3 %float_4
%114 = OpCompositeConstruct %Compound %113 %106
OpStore %c2 %114
%116 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpCompositeConstruct %v4float %118 %float_2 %float_3 %float_4
%120 = OpCompositeConstruct %Compound %119 %106
OpStore %c3 %120
%124 = OpLoad %float %x
%125 = OpFOrdEqual %bool %124 %float_3
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%129 = OpLoad %float %128
%130 = OpFOrdEqual %bool %129 %float_2
OpBranch %127
%127 = OpLabel
%131 = OpPhi %bool %false %66 %130 %126
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%135 = OpLoad %int %134
%136 = OpIEqual %bool %135 %int_3
OpBranch %133
%133 = OpLabel
%137 = OpPhi %bool %false %127 %136 %132
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpLoad %S %s_0
%141 = OpLoad %S %expected
%142 = OpCompositeExtract %float %140 0
%143 = OpCompositeExtract %float %141 0
%144 = OpFOrdEqual %bool %142 %143
%145 = OpCompositeExtract %int %140 1
%146 = OpCompositeExtract %int %141 1
%147 = OpIEqual %bool %145 %146
%148 = OpLogicalAnd %bool %147 %144
OpBranch %139
%139 = OpLabel
%149 = OpPhi %bool %false %133 %148 %138
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpLoad %S %s_0
%153 = OpCompositeConstruct %S %float_2 %int_3
%154 = OpCompositeExtract %float %152 0
%155 = OpFOrdEqual %bool %154 %float_2
%156 = OpCompositeExtract %int %152 1
%157 = OpIEqual %bool %156 %int_3
%158 = OpLogicalAnd %bool %157 %155
OpBranch %151
%151 = OpLabel
%159 = OpPhi %bool %false %139 %158 %150
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %S %s_0
%163 = OpFunctionCall %S %returns_a_struct_S
%164 = OpCompositeExtract %float %162 0
%165 = OpCompositeExtract %float %163 0
%166 = OpFUnordNotEqual %bool %164 %165
%167 = OpCompositeExtract %int %162 1
%168 = OpCompositeExtract %int %163 1
%169 = OpINotEqual %bool %167 %168
%170 = OpLogicalOr %bool %169 %166
OpBranch %161
%161 = OpLabel
%171 = OpPhi %bool %false %151 %170 %160
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %Nested %n1
%175 = OpLoad %Nested %n2
%176 = OpCompositeExtract %S %174 0
%177 = OpCompositeExtract %S %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpCompositeExtract %float %177 0
%180 = OpFOrdEqual %bool %178 %179
%181 = OpCompositeExtract %int %176 1
%182 = OpCompositeExtract %int %177 1
%183 = OpIEqual %bool %181 %182
%184 = OpLogicalAnd %bool %183 %180
%185 = OpCompositeExtract %S %174 1
%186 = OpCompositeExtract %S %175 1
%187 = OpCompositeExtract %float %185 0
%188 = OpCompositeExtract %float %186 0
%189 = OpFOrdEqual %bool %187 %188
%190 = OpCompositeExtract %int %185 1
%191 = OpCompositeExtract %int %186 1
%192 = OpIEqual %bool %190 %191
%193 = OpLogicalAnd %bool %192 %189
%194 = OpLogicalAnd %bool %193 %184
OpBranch %173
%173 = OpLabel
%195 = OpPhi %bool %false %161 %194 %172
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%198 = OpLoad %Nested %n1
%199 = OpLoad %Nested %n3
%200 = OpCompositeExtract %S %198 0
%201 = OpCompositeExtract %S %199 0
%202 = OpCompositeExtract %float %200 0
%203 = OpCompositeExtract %float %201 0
%204 = OpFUnordNotEqual %bool %202 %203
%205 = OpCompositeExtract %int %200 1
%206 = OpCompositeExtract %int %201 1
%207 = OpINotEqual %bool %205 %206
%208 = OpLogicalOr %bool %207 %204
%209 = OpCompositeExtract %S %198 1
%210 = OpCompositeExtract %S %199 1
%211 = OpCompositeExtract %float %209 0
%212 = OpCompositeExtract %float %210 0
%213 = OpFUnordNotEqual %bool %211 %212
%214 = OpCompositeExtract %int %209 1
%215 = OpCompositeExtract %int %210 1
%216 = OpINotEqual %bool %214 %215
%217 = OpLogicalOr %bool %216 %213
%218 = OpLogicalOr %bool %217 %208
OpBranch %197
%197 = OpLabel
%219 = OpPhi %bool %false %173 %218 %196
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%222 = OpLoad %Nested %n3
%223 = OpCompositeConstruct %S %float_1 %int_2
%224 = OpCompositeConstruct %S %float_2 %int_3
%225 = OpCompositeConstruct %Nested %223 %224
%226 = OpCompositeExtract %S %222 0
%227 = OpCompositeExtract %float %226 0
%228 = OpFOrdEqual %bool %227 %float_1
%229 = OpCompositeExtract %int %226 1
%230 = OpIEqual %bool %229 %int_2
%231 = OpLogicalAnd %bool %230 %228
%232 = OpCompositeExtract %S %222 1
%233 = OpCompositeExtract %float %232 0
%234 = OpFOrdEqual %bool %233 %float_2
%235 = OpCompositeExtract %int %232 1
%236 = OpIEqual %bool %235 %int_3
%237 = OpLogicalAnd %bool %236 %234
%238 = OpLogicalAnd %bool %237 %231
OpBranch %221
%221 = OpLabel
%239 = OpPhi %bool %false %197 %238 %220
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
%242 = OpLoad %Compound %c1
%243 = OpLoad %Compound %c2
%244 = OpCompositeExtract %v4float %242 0
%245 = OpCompositeExtract %v4float %243 0
%246 = OpFOrdEqual %v4bool %244 %245
%248 = OpAll %bool %246
%249 = OpCompositeExtract %v3int %242 1
%250 = OpCompositeExtract %v3int %243 1
%251 = OpIEqual %v3bool %249 %250
%253 = OpAll %bool %251
%254 = OpLogicalAnd %bool %253 %248
OpBranch %241
%241 = OpLabel
%255 = OpPhi %bool %false %221 %254 %240
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%258 = OpLoad %Compound %c2
%259 = OpLoad %Compound %c3
%260 = OpCompositeExtract %v4float %258 0
%261 = OpCompositeExtract %v4float %259 0
%262 = OpFUnordNotEqual %v4bool %260 %261
%263 = OpAny %bool %262
%264 = OpCompositeExtract %v3int %258 1
%265 = OpCompositeExtract %v3int %259 1
%266 = OpINotEqual %v3bool %264 %265
%267 = OpAny %bool %266
%268 = OpLogicalOr %bool %267 %263
OpBranch %257
%257 = OpLabel
%269 = OpPhi %bool %false %241 %268 %256
OpStore %valid %269
%270 = OpLoad %bool %valid
OpSelectionMerge %275 None
OpBranchConditional %270 %273 %274
%273 = OpLabel
%276 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%277 = OpLoad %v4float %276
OpStore %271 %277
OpBranch %275
%274 = OpLabel
%278 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%279 = OpLoad %v4float %278
OpStore %271 %279
OpBranch %275
%275 = OpLabel
%280 = OpLoad %v4float %271
OpReturnValue %280
OpFunctionEnd
