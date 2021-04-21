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
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%87 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%219 = OpVariable %_ptr_Function_v4float Function
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
%82 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %82 %81
%83 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %83 %81
%84 = OpLoad %Nested %n1
OpStore %n2 %84
OpStore %n3 %84
%85 = OpAccessChain %_ptr_Function_S %n3 %int_1
%86 = OpLoad %S %85
OpStore %87 %86
%88 = OpFunctionCall %void %modifies_a_struct_vS %87
%89 = OpLoad %S %87
OpStore %85 %89
%93 = OpLoad %float %x
%95 = OpFOrdEqual %bool %93 %float_3
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%99 = OpLoad %float %98
%100 = OpFOrdEqual %bool %99 %float_2
OpBranch %97
%97 = OpLabel
%101 = OpPhi %bool %false %66 %100 %96
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%105 = OpLoad %int %104
%106 = OpIEqual %bool %105 %int_3
OpBranch %103
%103 = OpLabel
%107 = OpPhi %bool %false %97 %106 %102
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %S %s_0
%111 = OpLoad %S %expected
%112 = OpCompositeExtract %float %110 0
%113 = OpCompositeExtract %float %111 0
%114 = OpFOrdEqual %bool %112 %113
%115 = OpCompositeExtract %int %110 1
%116 = OpCompositeExtract %int %111 1
%117 = OpIEqual %bool %115 %116
%118 = OpLogicalAnd %bool %117 %114
OpBranch %109
%109 = OpLabel
%119 = OpPhi %bool %false %103 %118 %108
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %S %s_0
%123 = OpCompositeConstruct %S %float_2 %int_3
%124 = OpCompositeExtract %float %122 0
%125 = OpCompositeExtract %float %123 0
%126 = OpFOrdEqual %bool %124 %125
%127 = OpCompositeExtract %int %122 1
%128 = OpCompositeExtract %int %123 1
%129 = OpIEqual %bool %127 %128
%130 = OpLogicalAnd %bool %129 %126
OpBranch %121
%121 = OpLabel
%131 = OpPhi %bool %false %109 %130 %120
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpLoad %S %s_0
%135 = OpFunctionCall %S %returns_a_struct_S
%136 = OpCompositeExtract %float %134 0
%137 = OpCompositeExtract %float %135 0
%138 = OpFOrdNotEqual %bool %136 %137
%139 = OpCompositeExtract %int %134 1
%140 = OpCompositeExtract %int %135 1
%141 = OpINotEqual %bool %139 %140
%142 = OpLogicalOr %bool %141 %138
OpBranch %133
%133 = OpLabel
%143 = OpPhi %bool %false %121 %142 %132
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpLoad %Nested %n1
%147 = OpLoad %Nested %n2
%148 = OpCompositeExtract %S %146 0
%149 = OpCompositeExtract %S %147 0
%150 = OpCompositeExtract %float %148 0
%151 = OpCompositeExtract %float %149 0
%152 = OpFOrdEqual %bool %150 %151
%153 = OpCompositeExtract %int %148 1
%154 = OpCompositeExtract %int %149 1
%155 = OpIEqual %bool %153 %154
%156 = OpLogicalAnd %bool %155 %152
%157 = OpCompositeExtract %S %146 1
%158 = OpCompositeExtract %S %147 1
%159 = OpCompositeExtract %float %157 0
%160 = OpCompositeExtract %float %158 0
%161 = OpFOrdEqual %bool %159 %160
%162 = OpCompositeExtract %int %157 1
%163 = OpCompositeExtract %int %158 1
%164 = OpIEqual %bool %162 %163
%165 = OpLogicalAnd %bool %164 %161
%166 = OpLogicalAnd %bool %165 %156
OpBranch %145
%145 = OpLabel
%167 = OpPhi %bool %false %133 %166 %144
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpLoad %Nested %n1
%171 = OpLoad %Nested %n3
%172 = OpCompositeExtract %S %170 0
%173 = OpCompositeExtract %S %171 0
%174 = OpCompositeExtract %float %172 0
%175 = OpCompositeExtract %float %173 0
%176 = OpFOrdNotEqual %bool %174 %175
%177 = OpCompositeExtract %int %172 1
%178 = OpCompositeExtract %int %173 1
%179 = OpINotEqual %bool %177 %178
%180 = OpLogicalOr %bool %179 %176
%181 = OpCompositeExtract %S %170 1
%182 = OpCompositeExtract %S %171 1
%183 = OpCompositeExtract %float %181 0
%184 = OpCompositeExtract %float %182 0
%185 = OpFOrdNotEqual %bool %183 %184
%186 = OpCompositeExtract %int %181 1
%187 = OpCompositeExtract %int %182 1
%188 = OpINotEqual %bool %186 %187
%189 = OpLogicalOr %bool %188 %185
%190 = OpLogicalOr %bool %189 %180
OpBranch %169
%169 = OpLabel
%191 = OpPhi %bool %false %145 %190 %168
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpLoad %Nested %n3
%195 = OpCompositeConstruct %S %float_1 %int_2
%196 = OpCompositeConstruct %S %float_2 %int_3
%197 = OpCompositeConstruct %Nested %195 %196
%198 = OpCompositeExtract %S %194 0
%199 = OpCompositeExtract %S %197 0
%200 = OpCompositeExtract %float %198 0
%201 = OpCompositeExtract %float %199 0
%202 = OpFOrdEqual %bool %200 %201
%203 = OpCompositeExtract %int %198 1
%204 = OpCompositeExtract %int %199 1
%205 = OpIEqual %bool %203 %204
%206 = OpLogicalAnd %bool %205 %202
%207 = OpCompositeExtract %S %194 1
%208 = OpCompositeExtract %S %197 1
%209 = OpCompositeExtract %float %207 0
%210 = OpCompositeExtract %float %208 0
%211 = OpFOrdEqual %bool %209 %210
%212 = OpCompositeExtract %int %207 1
%213 = OpCompositeExtract %int %208 1
%214 = OpIEqual %bool %212 %213
%215 = OpLogicalAnd %bool %214 %211
%216 = OpLogicalAnd %bool %215 %206
OpBranch %193
%193 = OpLabel
%217 = OpPhi %bool %false %169 %216 %192
OpStore %valid %217
%218 = OpLoad %bool %valid
OpSelectionMerge %223 None
OpBranchConditional %218 %221 %222
%221 = OpLabel
%224 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%226 = OpLoad %v4float %224
OpStore %219 %226
OpBranch %223
%222 = OpLabel
%227 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%228 = OpLoad %v4float %227
OpStore %219 %228
OpBranch %223
%223 = OpLabel
%229 = OpLoad %v4float %219
OpReturnValue %229
OpFunctionEnd
