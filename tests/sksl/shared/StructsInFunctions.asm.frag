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
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
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
%79 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%173 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %S %returns_a_struct_S
OpStore %s_0 %57
%59 = OpLoad %S %s_0
OpStore %60 %59
%61 = OpFunctionCall %float %accepts_a_struct_fS %60
OpStore %x %61
%62 = OpFunctionCall %void %modifies_a_struct_vS %s_0
%65 = OpAccessChain %_ptr_Function_float %expected %int_0
OpStore %65 %float_2
%67 = OpAccessChain %_ptr_Function_int %expected %int_1
OpStore %67 %int_3
%73 = OpFunctionCall %S %returns_a_struct_S
%74 = OpAccessChain %_ptr_Function_S %n1 %int_1
OpStore %74 %73
%75 = OpAccessChain %_ptr_Function_S %n1 %int_0
OpStore %75 %73
%76 = OpLoad %Nested %n1
OpStore %n2 %76
OpStore %n3 %76
%77 = OpAccessChain %_ptr_Function_S %n3 %int_1
%78 = OpLoad %S %77
OpStore %79 %78
%80 = OpFunctionCall %void %modifies_a_struct_vS %79
%81 = OpLoad %S %79
OpStore %77 %81
%85 = OpLoad %float %x
%87 = OpFOrdEqual %bool %85 %float_3
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%91 = OpLoad %float %90
%92 = OpFOrdEqual %bool %91 %float_2
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %55 %92 %88
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%97 = OpLoad %int %96
%98 = OpIEqual %bool %97 %int_3
OpBranch %95
%95 = OpLabel
%99 = OpPhi %bool %false %89 %98 %94
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %S %s_0
%103 = OpLoad %S %expected
%104 = OpCompositeExtract %float %102 0
%105 = OpCompositeExtract %float %103 0
%106 = OpFOrdEqual %bool %104 %105
%107 = OpCompositeExtract %int %102 1
%108 = OpCompositeExtract %int %103 1
%109 = OpIEqual %bool %107 %108
%110 = OpLogicalAnd %bool %109 %106
OpBranch %101
%101 = OpLabel
%111 = OpPhi %bool %false %95 %110 %100
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %S %s_0
%115 = OpFunctionCall %S %returns_a_struct_S
%116 = OpCompositeExtract %float %114 0
%117 = OpCompositeExtract %float %115 0
%118 = OpFOrdNotEqual %bool %116 %117
%119 = OpCompositeExtract %int %114 1
%120 = OpCompositeExtract %int %115 1
%121 = OpINotEqual %bool %119 %120
%122 = OpLogicalOr %bool %121 %118
OpBranch %113
%113 = OpLabel
%123 = OpPhi %bool %false %101 %122 %112
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %Nested %n1
%127 = OpLoad %Nested %n2
%128 = OpCompositeExtract %S %126 0
%129 = OpCompositeExtract %S %127 0
%130 = OpCompositeExtract %float %128 0
%131 = OpCompositeExtract %float %129 0
%132 = OpFOrdEqual %bool %130 %131
%133 = OpCompositeExtract %int %128 1
%134 = OpCompositeExtract %int %129 1
%135 = OpIEqual %bool %133 %134
%136 = OpLogicalAnd %bool %135 %132
%137 = OpCompositeExtract %S %126 1
%138 = OpCompositeExtract %S %127 1
%139 = OpCompositeExtract %float %137 0
%140 = OpCompositeExtract %float %138 0
%141 = OpFOrdEqual %bool %139 %140
%142 = OpCompositeExtract %int %137 1
%143 = OpCompositeExtract %int %138 1
%144 = OpIEqual %bool %142 %143
%145 = OpLogicalAnd %bool %144 %141
%146 = OpLogicalAnd %bool %145 %136
OpBranch %125
%125 = OpLabel
%147 = OpPhi %bool %false %113 %146 %124
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpLoad %Nested %n1
%151 = OpLoad %Nested %n3
%152 = OpCompositeExtract %S %150 0
%153 = OpCompositeExtract %S %151 0
%154 = OpCompositeExtract %float %152 0
%155 = OpCompositeExtract %float %153 0
%156 = OpFOrdNotEqual %bool %154 %155
%157 = OpCompositeExtract %int %152 1
%158 = OpCompositeExtract %int %153 1
%159 = OpINotEqual %bool %157 %158
%160 = OpLogicalOr %bool %159 %156
%161 = OpCompositeExtract %S %150 1
%162 = OpCompositeExtract %S %151 1
%163 = OpCompositeExtract %float %161 0
%164 = OpCompositeExtract %float %162 0
%165 = OpFOrdNotEqual %bool %163 %164
%166 = OpCompositeExtract %int %161 1
%167 = OpCompositeExtract %int %162 1
%168 = OpINotEqual %bool %166 %167
%169 = OpLogicalOr %bool %168 %165
%170 = OpLogicalOr %bool %169 %160
OpBranch %149
%149 = OpLabel
%171 = OpPhi %bool %false %125 %170 %148
OpStore %valid %171
%172 = OpLoad %bool %valid
OpSelectionMerge %177 None
OpBranchConditional %172 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%180 = OpLoad %v4float %178
OpStore %173 %180
OpBranch %177
%176 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%182 = OpLoad %v4float %181
OpStore %173 %182
OpBranch %177
%177 = OpLabel
%183 = OpLoad %v4float %173
OpReturnValue %183
OpFunctionEnd
