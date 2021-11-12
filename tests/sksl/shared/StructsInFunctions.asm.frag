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
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
%90 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%222 = OpVariable %_ptr_Function_v4float Function
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
%96 = OpLoad %float %x
%98 = OpFOrdEqual %bool %96 %float_3
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%102 = OpLoad %float %101
%103 = OpFOrdEqual %bool %102 %float_2
OpBranch %100
%100 = OpLabel
%104 = OpPhi %bool %false %66 %103 %99
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%108 = OpLoad %int %107
%109 = OpIEqual %bool %108 %int_3
OpBranch %106
%106 = OpLabel
%110 = OpPhi %bool %false %100 %109 %105
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %S %s_0
%114 = OpLoad %S %expected
%115 = OpCompositeExtract %float %113 0
%116 = OpCompositeExtract %float %114 0
%117 = OpFOrdEqual %bool %115 %116
%118 = OpCompositeExtract %int %113 1
%119 = OpCompositeExtract %int %114 1
%120 = OpIEqual %bool %118 %119
%121 = OpLogicalAnd %bool %120 %117
OpBranch %112
%112 = OpLabel
%122 = OpPhi %bool %false %106 %121 %111
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %S %s_0
%126 = OpCompositeConstruct %S %float_2 %int_3
%127 = OpCompositeExtract %float %125 0
%128 = OpCompositeExtract %float %126 0
%129 = OpFOrdEqual %bool %127 %128
%130 = OpCompositeExtract %int %125 1
%131 = OpCompositeExtract %int %126 1
%132 = OpIEqual %bool %130 %131
%133 = OpLogicalAnd %bool %132 %129
OpBranch %124
%124 = OpLabel
%134 = OpPhi %bool %false %112 %133 %123
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpLoad %S %s_0
%138 = OpFunctionCall %S %returns_a_struct_S
%139 = OpCompositeExtract %float %137 0
%140 = OpCompositeExtract %float %138 0
%141 = OpFOrdNotEqual %bool %139 %140
%142 = OpCompositeExtract %int %137 1
%143 = OpCompositeExtract %int %138 1
%144 = OpINotEqual %bool %142 %143
%145 = OpLogicalOr %bool %144 %141
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %124 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %Nested %n1
%150 = OpLoad %Nested %n2
%151 = OpCompositeExtract %S %149 0
%152 = OpCompositeExtract %S %150 0
%153 = OpCompositeExtract %float %151 0
%154 = OpCompositeExtract %float %152 0
%155 = OpFOrdEqual %bool %153 %154
%156 = OpCompositeExtract %int %151 1
%157 = OpCompositeExtract %int %152 1
%158 = OpIEqual %bool %156 %157
%159 = OpLogicalAnd %bool %158 %155
%160 = OpCompositeExtract %S %149 1
%161 = OpCompositeExtract %S %150 1
%162 = OpCompositeExtract %float %160 0
%163 = OpCompositeExtract %float %161 0
%164 = OpFOrdEqual %bool %162 %163
%165 = OpCompositeExtract %int %160 1
%166 = OpCompositeExtract %int %161 1
%167 = OpIEqual %bool %165 %166
%168 = OpLogicalAnd %bool %167 %164
%169 = OpLogicalAnd %bool %168 %159
OpBranch %148
%148 = OpLabel
%170 = OpPhi %bool %false %136 %169 %147
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %Nested %n1
%174 = OpLoad %Nested %n3
%175 = OpCompositeExtract %S %173 0
%176 = OpCompositeExtract %S %174 0
%177 = OpCompositeExtract %float %175 0
%178 = OpCompositeExtract %float %176 0
%179 = OpFOrdNotEqual %bool %177 %178
%180 = OpCompositeExtract %int %175 1
%181 = OpCompositeExtract %int %176 1
%182 = OpINotEqual %bool %180 %181
%183 = OpLogicalOr %bool %182 %179
%184 = OpCompositeExtract %S %173 1
%185 = OpCompositeExtract %S %174 1
%186 = OpCompositeExtract %float %184 0
%187 = OpCompositeExtract %float %185 0
%188 = OpFOrdNotEqual %bool %186 %187
%189 = OpCompositeExtract %int %184 1
%190 = OpCompositeExtract %int %185 1
%191 = OpINotEqual %bool %189 %190
%192 = OpLogicalOr %bool %191 %188
%193 = OpLogicalOr %bool %192 %183
OpBranch %172
%172 = OpLabel
%194 = OpPhi %bool %false %148 %193 %171
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%197 = OpLoad %Nested %n3
%198 = OpCompositeConstruct %S %float_1 %int_2
%199 = OpCompositeConstruct %S %float_2 %int_3
%200 = OpCompositeConstruct %Nested %198 %199
%201 = OpCompositeExtract %S %197 0
%202 = OpCompositeExtract %S %200 0
%203 = OpCompositeExtract %float %201 0
%204 = OpCompositeExtract %float %202 0
%205 = OpFOrdEqual %bool %203 %204
%206 = OpCompositeExtract %int %201 1
%207 = OpCompositeExtract %int %202 1
%208 = OpIEqual %bool %206 %207
%209 = OpLogicalAnd %bool %208 %205
%210 = OpCompositeExtract %S %197 1
%211 = OpCompositeExtract %S %200 1
%212 = OpCompositeExtract %float %210 0
%213 = OpCompositeExtract %float %211 0
%214 = OpFOrdEqual %bool %212 %213
%215 = OpCompositeExtract %int %210 1
%216 = OpCompositeExtract %int %211 1
%217 = OpIEqual %bool %215 %216
%218 = OpLogicalAnd %bool %217 %214
%219 = OpLogicalAnd %bool %218 %209
OpBranch %196
%196 = OpLabel
%220 = OpPhi %bool %false %172 %219 %195
OpStore %valid %220
%221 = OpLoad %bool %valid
OpSelectionMerge %226 None
OpBranchConditional %221 %224 %225
%224 = OpLabel
%227 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%229 = OpLoad %v4float %227
OpStore %222 %229
OpBranch %226
%225 = OpLabel
%230 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%231 = OpLoad %v4float %230
OpStore %222 %231
OpBranch %226
%226 = OpLabel
%232 = OpLoad %v4float %222
OpReturnValue %232
OpFunctionEnd
