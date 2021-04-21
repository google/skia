OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %s1 "s1"
OpName %s2 "s2"
OpName %s3 "s3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_n4 = OpConstant %float -4
%S = OpTypeStruct %int %int
%int_3 = OpConstant %int 3
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%f1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f3 = OpVariable %_ptr_Function__arr_float_int_4 Function
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%171 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f2 %37
%40 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %40
%48 = OpCompositeConstruct %S %int_1 %int_2
%49 = OpCompositeConstruct %S %int_3 %int_4
%52 = OpCompositeConstruct %S %int_5 %int_6
%53 = OpCompositeConstruct %_arr_S_int_3 %48 %49 %52
OpStore %s1 %53
%55 = OpCompositeConstruct %S %int_1 %int_2
%57 = OpCompositeConstruct %S %int_0 %int_0
%58 = OpCompositeConstruct %S %int_5 %int_6
%59 = OpCompositeConstruct %_arr_S_int_3 %55 %57 %58
OpStore %s2 %59
%61 = OpCompositeConstruct %S %int_1 %int_2
%62 = OpCompositeConstruct %S %int_3 %int_4
%63 = OpCompositeConstruct %S %int_5 %int_6
%64 = OpCompositeConstruct %_arr_S_int_3 %61 %62 %63
OpStore %s3 %64
%66 = OpLoad %_arr_float_int_4 %f1
%67 = OpLoad %_arr_float_int_4 %f2
%68 = OpCompositeExtract %float %66 0
%69 = OpCompositeExtract %float %67 0
%70 = OpFOrdEqual %bool %68 %69
%71 = OpCompositeExtract %float %66 1
%72 = OpCompositeExtract %float %67 1
%73 = OpFOrdEqual %bool %71 %72
%74 = OpLogicalAnd %bool %73 %70
%75 = OpCompositeExtract %float %66 2
%76 = OpCompositeExtract %float %67 2
%77 = OpFOrdEqual %bool %75 %76
%78 = OpLogicalAnd %bool %77 %74
%79 = OpCompositeExtract %float %66 3
%80 = OpCompositeExtract %float %67 3
%81 = OpFOrdEqual %bool %79 %80
%82 = OpLogicalAnd %bool %81 %78
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %_arr_float_int_4 %f1
%86 = OpLoad %_arr_float_int_4 %f3
%87 = OpCompositeExtract %float %85 0
%88 = OpCompositeExtract %float %86 0
%89 = OpFOrdNotEqual %bool %87 %88
%90 = OpCompositeExtract %float %85 1
%91 = OpCompositeExtract %float %86 1
%92 = OpFOrdNotEqual %bool %90 %91
%93 = OpLogicalOr %bool %92 %89
%94 = OpCompositeExtract %float %85 2
%95 = OpCompositeExtract %float %86 2
%96 = OpFOrdNotEqual %bool %94 %95
%97 = OpLogicalOr %bool %96 %93
%98 = OpCompositeExtract %float %85 3
%99 = OpCompositeExtract %float %86 3
%100 = OpFOrdNotEqual %bool %98 %99
%101 = OpLogicalOr %bool %100 %97
OpBranch %84
%84 = OpLabel
%102 = OpPhi %bool %false %25 %101 %83
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %_arr_S_int_3 %s1
%106 = OpLoad %_arr_S_int_3 %s2
%107 = OpCompositeExtract %S %105 0
%108 = OpCompositeExtract %S %106 0
%109 = OpCompositeExtract %int %107 0
%110 = OpCompositeExtract %int %108 0
%111 = OpINotEqual %bool %109 %110
%112 = OpCompositeExtract %int %107 1
%113 = OpCompositeExtract %int %108 1
%114 = OpINotEqual %bool %112 %113
%115 = OpLogicalOr %bool %114 %111
%116 = OpCompositeExtract %S %105 1
%117 = OpCompositeExtract %S %106 1
%118 = OpCompositeExtract %int %116 0
%119 = OpCompositeExtract %int %117 0
%120 = OpINotEqual %bool %118 %119
%121 = OpCompositeExtract %int %116 1
%122 = OpCompositeExtract %int %117 1
%123 = OpINotEqual %bool %121 %122
%124 = OpLogicalOr %bool %123 %120
%125 = OpLogicalOr %bool %124 %115
%126 = OpCompositeExtract %S %105 2
%127 = OpCompositeExtract %S %106 2
%128 = OpCompositeExtract %int %126 0
%129 = OpCompositeExtract %int %127 0
%130 = OpINotEqual %bool %128 %129
%131 = OpCompositeExtract %int %126 1
%132 = OpCompositeExtract %int %127 1
%133 = OpINotEqual %bool %131 %132
%134 = OpLogicalOr %bool %133 %130
%135 = OpLogicalOr %bool %134 %125
OpBranch %104
%104 = OpLabel
%136 = OpPhi %bool %false %84 %135 %103
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%139 = OpLoad %_arr_S_int_3 %s3
%140 = OpLoad %_arr_S_int_3 %s1
%141 = OpCompositeExtract %S %139 0
%142 = OpCompositeExtract %S %140 0
%143 = OpCompositeExtract %int %141 0
%144 = OpCompositeExtract %int %142 0
%145 = OpIEqual %bool %143 %144
%146 = OpCompositeExtract %int %141 1
%147 = OpCompositeExtract %int %142 1
%148 = OpIEqual %bool %146 %147
%149 = OpLogicalAnd %bool %148 %145
%150 = OpCompositeExtract %S %139 1
%151 = OpCompositeExtract %S %140 1
%152 = OpCompositeExtract %int %150 0
%153 = OpCompositeExtract %int %151 0
%154 = OpIEqual %bool %152 %153
%155 = OpCompositeExtract %int %150 1
%156 = OpCompositeExtract %int %151 1
%157 = OpIEqual %bool %155 %156
%158 = OpLogicalAnd %bool %157 %154
%159 = OpLogicalAnd %bool %158 %149
%160 = OpCompositeExtract %S %139 2
%161 = OpCompositeExtract %S %140 2
%162 = OpCompositeExtract %int %160 0
%163 = OpCompositeExtract %int %161 0
%164 = OpIEqual %bool %162 %163
%165 = OpCompositeExtract %int %160 1
%166 = OpCompositeExtract %int %161 1
%167 = OpIEqual %bool %165 %166
%168 = OpLogicalAnd %bool %167 %164
%169 = OpLogicalAnd %bool %168 %159
OpBranch %138
%138 = OpLabel
%170 = OpPhi %bool %false %104 %169 %137
OpSelectionMerge %175 None
OpBranchConditional %170 %173 %174
%173 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%178 = OpLoad %v4float %176
OpStore %171 %178
OpBranch %175
%174 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%180 = OpLoad %v4float %179
OpStore %171 %180
OpBranch %175
%175 = OpLabel
%181 = OpLoad %v4float %171
OpReturnValue %181
OpFunctionEnd
