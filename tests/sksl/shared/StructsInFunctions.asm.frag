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
OpDecorate %164 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
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
%279 = OpVariable %_ptr_Function_v4float Function
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
%155 = OpCompositeExtract %float %153 0
%156 = OpFOrdEqual %bool %154 %155
%157 = OpCompositeExtract %int %152 1
%158 = OpCompositeExtract %int %153 1
%159 = OpIEqual %bool %157 %158
%160 = OpLogicalAnd %bool %159 %156
OpBranch %151
%151 = OpLabel
%161 = OpPhi %bool %false %139 %160 %150
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %S %s_0
%165 = OpFunctionCall %S %returns_a_struct_S
%166 = OpCompositeExtract %float %164 0
%167 = OpCompositeExtract %float %165 0
%168 = OpFOrdNotEqual %bool %166 %167
%169 = OpCompositeExtract %int %164 1
%170 = OpCompositeExtract %int %165 1
%171 = OpINotEqual %bool %169 %170
%172 = OpLogicalOr %bool %171 %168
OpBranch %163
%163 = OpLabel
%173 = OpPhi %bool %false %151 %172 %162
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%176 = OpLoad %Nested %n1
%177 = OpLoad %Nested %n2
%178 = OpCompositeExtract %S %176 0
%179 = OpCompositeExtract %S %177 0
%180 = OpCompositeExtract %float %178 0
%181 = OpCompositeExtract %float %179 0
%182 = OpFOrdEqual %bool %180 %181
%183 = OpCompositeExtract %int %178 1
%184 = OpCompositeExtract %int %179 1
%185 = OpIEqual %bool %183 %184
%186 = OpLogicalAnd %bool %185 %182
%187 = OpCompositeExtract %S %176 1
%188 = OpCompositeExtract %S %177 1
%189 = OpCompositeExtract %float %187 0
%190 = OpCompositeExtract %float %188 0
%191 = OpFOrdEqual %bool %189 %190
%192 = OpCompositeExtract %int %187 1
%193 = OpCompositeExtract %int %188 1
%194 = OpIEqual %bool %192 %193
%195 = OpLogicalAnd %bool %194 %191
%196 = OpLogicalAnd %bool %195 %186
OpBranch %175
%175 = OpLabel
%197 = OpPhi %bool %false %163 %196 %174
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %Nested %n1
%201 = OpLoad %Nested %n3
%202 = OpCompositeExtract %S %200 0
%203 = OpCompositeExtract %S %201 0
%204 = OpCompositeExtract %float %202 0
%205 = OpCompositeExtract %float %203 0
%206 = OpFOrdNotEqual %bool %204 %205
%207 = OpCompositeExtract %int %202 1
%208 = OpCompositeExtract %int %203 1
%209 = OpINotEqual %bool %207 %208
%210 = OpLogicalOr %bool %209 %206
%211 = OpCompositeExtract %S %200 1
%212 = OpCompositeExtract %S %201 1
%213 = OpCompositeExtract %float %211 0
%214 = OpCompositeExtract %float %212 0
%215 = OpFOrdNotEqual %bool %213 %214
%216 = OpCompositeExtract %int %211 1
%217 = OpCompositeExtract %int %212 1
%218 = OpINotEqual %bool %216 %217
%219 = OpLogicalOr %bool %218 %215
%220 = OpLogicalOr %bool %219 %210
OpBranch %199
%199 = OpLabel
%221 = OpPhi %bool %false %175 %220 %198
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpLoad %Nested %n3
%225 = OpCompositeConstruct %S %float_1 %int_2
%226 = OpCompositeConstruct %S %float_2 %int_3
%227 = OpCompositeConstruct %Nested %225 %226
%228 = OpCompositeExtract %S %224 0
%229 = OpCompositeExtract %S %227 0
%230 = OpCompositeExtract %float %228 0
%231 = OpCompositeExtract %float %229 0
%232 = OpFOrdEqual %bool %230 %231
%233 = OpCompositeExtract %int %228 1
%234 = OpCompositeExtract %int %229 1
%235 = OpIEqual %bool %233 %234
%236 = OpLogicalAnd %bool %235 %232
%237 = OpCompositeExtract %S %224 1
%238 = OpCompositeExtract %S %227 1
%239 = OpCompositeExtract %float %237 0
%240 = OpCompositeExtract %float %238 0
%241 = OpFOrdEqual %bool %239 %240
%242 = OpCompositeExtract %int %237 1
%243 = OpCompositeExtract %int %238 1
%244 = OpIEqual %bool %242 %243
%245 = OpLogicalAnd %bool %244 %241
%246 = OpLogicalAnd %bool %245 %236
OpBranch %223
%223 = OpLabel
%247 = OpPhi %bool %false %199 %246 %222
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%250 = OpLoad %Compound %c1
%251 = OpLoad %Compound %c2
%252 = OpCompositeExtract %v4float %250 0
%253 = OpCompositeExtract %v4float %251 0
%254 = OpFOrdEqual %v4bool %252 %253
%256 = OpAll %bool %254
%257 = OpCompositeExtract %v3int %250 1
%258 = OpCompositeExtract %v3int %251 1
%259 = OpIEqual %v3bool %257 %258
%261 = OpAll %bool %259
%262 = OpLogicalAnd %bool %261 %256
OpBranch %249
%249 = OpLabel
%263 = OpPhi %bool %false %223 %262 %248
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpLoad %Compound %c2
%267 = OpLoad %Compound %c3
%268 = OpCompositeExtract %v4float %266 0
%269 = OpCompositeExtract %v4float %267 0
%270 = OpFOrdNotEqual %v4bool %268 %269
%271 = OpAny %bool %270
%272 = OpCompositeExtract %v3int %266 1
%273 = OpCompositeExtract %v3int %267 1
%274 = OpINotEqual %v3bool %272 %273
%275 = OpAny %bool %274
%276 = OpLogicalOr %bool %275 %271
OpBranch %265
%265 = OpLabel
%277 = OpPhi %bool %false %249 %276 %264
OpStore %valid %277
%278 = OpLoad %bool %valid
OpSelectionMerge %283 None
OpBranchConditional %278 %281 %282
%281 = OpLabel
%284 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%285 = OpLoad %v4float %284
OpStore %279 %285
OpBranch %283
%282 = OpLabel
%286 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%287 = OpLoad %v4float %286
OpStore %279 %287
OpBranch %283
%283 = OpLabel
%288 = OpLoad %v4float %279
OpReturnValue %288
OpFunctionEnd
