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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %35 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpMemberDecorate %Nested 0 Offset 0
OpMemberDecorate %Nested 0 RelaxedPrecision
OpMemberDecorate %Nested 1 Offset 16
OpMemberDecorate %Nested 1 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%int = OpTypeInt 32 1
%S = OpTypeStruct %float %int
%23 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%36 = OpTypeFunction %float %_ptr_Function_S
%45 = OpTypeFunction %void %_ptr_Function_S
%54 = OpTypeFunction %v4float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%20 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %23
%24 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%29 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %29 %float_1
%33 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %33 %int_2
%35 = OpLoad %S %s
OpReturnValue %35
OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_S
%38 = OpLabel
%39 = OpAccessChain %_ptr_Function_float %37 %int_0
%40 = OpLoad %float %39
%41 = OpAccessChain %_ptr_Function_int %37 %int_1
%42 = OpLoad %int %41
%43 = OpConvertSToF %float %42
%44 = OpFAdd %float %40 %43
OpReturnValue %44
OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %45
%46 = OpFunctionParameter %_ptr_Function_S
%47 = OpLabel
%48 = OpAccessChain %_ptr_Function_float %46 %int_0
%49 = OpLoad %float %48
%50 = OpFAdd %float %49 %float_1
OpStore %48 %50
%51 = OpAccessChain %_ptr_Function_int %46 %int_1
%52 = OpLoad %int %51
%53 = OpIAdd %int %52 %int_1
OpStore %51 %53
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%60 = OpVariable %_ptr_Function_S Function
%expected = OpVariable %_ptr_Function_S Function
%n1 = OpVariable %_ptr_Function_Nested Function
%n2 = OpVariable %_ptr_Function_Nested Function
%n3 = OpVariable %_ptr_Function_Nested Function
%78 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%210 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %57
%59 = OpLoad %S %s_0
OpStore %60 %59
%61 = OpFunctionCall %float %accepts_a_struct_fS %60
OpStore %x %61
%62 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%66 = OpCompositeConstruct %S %float_2 %int_3
OpStore %expected %66
%72 = OpFunctionCall %S %returns_a_struct_S
%73 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %73 %72
%74 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %74 %72
%75 = OpLoad %Nested %n1
OpStore %n2 %75
OpStore %n3 %75
%76 = OpAccessChain %_ptr_Function_S %n3 %int_1
%77 = OpLoad %S %76
OpStore %78 %77
%79 = OpFunctionCall %void %modifies_a_struct_vS %78
%80 = OpLoad %S %78
OpStore %76 %80
%84 = OpLoad %float %x
%86 = OpFOrdEqual %bool %84 %float_3
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%90 = OpLoad %float %89
%91 = OpFOrdEqual %bool %90 %float_2
OpBranch %88
%88 = OpLabel
%92 = OpPhi %bool %false %55 %91 %87
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%96 = OpLoad %int %95
%97 = OpIEqual %bool %96 %int_3
OpBranch %94
%94 = OpLabel
%98 = OpPhi %bool %false %88 %97 %93
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %S %s_0
%102 = OpLoad %S %expected
%103 = OpCompositeExtract %float %101 0
%104 = OpCompositeExtract %float %102 0
%105 = OpFOrdEqual %bool %103 %104
%106 = OpCompositeExtract %int %101 1
%107 = OpCompositeExtract %int %102 1
%108 = OpIEqual %bool %106 %107
%109 = OpLogicalAnd %bool %108 %105
OpBranch %100
%100 = OpLabel
%110 = OpPhi %bool %false %94 %109 %99
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %S %s_0
%114 = OpCompositeConstruct %S %float_2 %int_3
%115 = OpCompositeExtract %float %113 0
%116 = OpCompositeExtract %float %114 0
%117 = OpFOrdEqual %bool %115 %116
%118 = OpCompositeExtract %int %113 1
%119 = OpCompositeExtract %int %114 1
%120 = OpIEqual %bool %118 %119
%121 = OpLogicalAnd %bool %120 %117
OpBranch %112
%112 = OpLabel
%122 = OpPhi %bool %false %100 %121 %111
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %S %s_0
%126 = OpFunctionCall %S %returns_a_struct_S
%127 = OpCompositeExtract %float %125 0
%128 = OpCompositeExtract %float %126 0
%129 = OpFOrdNotEqual %bool %127 %128
%130 = OpCompositeExtract %int %125 1
%131 = OpCompositeExtract %int %126 1
%132 = OpINotEqual %bool %130 %131
%133 = OpLogicalOr %bool %132 %129
OpBranch %124
%124 = OpLabel
%134 = OpPhi %bool %false %112 %133 %123
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpLoad %Nested %n1
%138 = OpLoad %Nested %n2
%139 = OpCompositeExtract %S %137 0
%140 = OpCompositeExtract %S %138 0
%141 = OpCompositeExtract %float %139 0
%142 = OpCompositeExtract %float %140 0
%143 = OpFOrdEqual %bool %141 %142
%144 = OpCompositeExtract %int %139 1
%145 = OpCompositeExtract %int %140 1
%146 = OpIEqual %bool %144 %145
%147 = OpLogicalAnd %bool %146 %143
%148 = OpCompositeExtract %S %137 1
%149 = OpCompositeExtract %S %138 1
%150 = OpCompositeExtract %float %148 0
%151 = OpCompositeExtract %float %149 0
%152 = OpFOrdEqual %bool %150 %151
%153 = OpCompositeExtract %int %148 1
%154 = OpCompositeExtract %int %149 1
%155 = OpIEqual %bool %153 %154
%156 = OpLogicalAnd %bool %155 %152
%157 = OpLogicalAnd %bool %156 %147
OpBranch %136
%136 = OpLabel
%158 = OpPhi %bool %false %124 %157 %135
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %Nested %n1
%162 = OpLoad %Nested %n3
%163 = OpCompositeExtract %S %161 0
%164 = OpCompositeExtract %S %162 0
%165 = OpCompositeExtract %float %163 0
%166 = OpCompositeExtract %float %164 0
%167 = OpFOrdNotEqual %bool %165 %166
%168 = OpCompositeExtract %int %163 1
%169 = OpCompositeExtract %int %164 1
%170 = OpINotEqual %bool %168 %169
%171 = OpLogicalOr %bool %170 %167
%172 = OpCompositeExtract %S %161 1
%173 = OpCompositeExtract %S %162 1
%174 = OpCompositeExtract %float %172 0
%175 = OpCompositeExtract %float %173 0
%176 = OpFOrdNotEqual %bool %174 %175
%177 = OpCompositeExtract %int %172 1
%178 = OpCompositeExtract %int %173 1
%179 = OpINotEqual %bool %177 %178
%180 = OpLogicalOr %bool %179 %176
%181 = OpLogicalOr %bool %180 %171
OpBranch %160
%160 = OpLabel
%182 = OpPhi %bool %false %136 %181 %159
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %Nested %n3
%186 = OpCompositeConstruct %S %float_1 %int_2
%187 = OpCompositeConstruct %S %float_2 %int_3
%188 = OpCompositeConstruct %Nested %186 %187
%189 = OpCompositeExtract %S %185 0
%190 = OpCompositeExtract %S %188 0
%191 = OpCompositeExtract %float %189 0
%192 = OpCompositeExtract %float %190 0
%193 = OpFOrdEqual %bool %191 %192
%194 = OpCompositeExtract %int %189 1
%195 = OpCompositeExtract %int %190 1
%196 = OpIEqual %bool %194 %195
%197 = OpLogicalAnd %bool %196 %193
%198 = OpCompositeExtract %S %185 1
%199 = OpCompositeExtract %S %188 1
%200 = OpCompositeExtract %float %198 0
%201 = OpCompositeExtract %float %199 0
%202 = OpFOrdEqual %bool %200 %201
%203 = OpCompositeExtract %int %198 1
%204 = OpCompositeExtract %int %199 1
%205 = OpIEqual %bool %203 %204
%206 = OpLogicalAnd %bool %205 %202
%207 = OpLogicalAnd %bool %206 %197
OpBranch %184
%184 = OpLabel
%208 = OpPhi %bool %false %160 %207 %183
OpStore %valid %208
%209 = OpLoad %bool %valid
OpSelectionMerge %214 None
OpBranchConditional %209 %212 %213
%212 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%217 = OpLoad %v4float %215
OpStore %210 %217
OpBranch %214
%213 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%219 = OpLoad %v4float %218
OpStore %210 %219
OpBranch %214
%214 = OpLabel
%220 = OpLoad %v4float %210
OpReturnValue %220
OpFunctionEnd
