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
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
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
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%f1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f3 = OpVariable %_ptr_Function__arr_float_int_4 Function
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%165 = OpVariable %_ptr_Function_v4float Function
%29 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %29
%31 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f2 %31
%34 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %34
%42 = OpCompositeConstruct %S %int_1 %int_2
%43 = OpCompositeConstruct %S %int_3 %int_4
%46 = OpCompositeConstruct %S %int_5 %int_6
%47 = OpCompositeConstruct %_arr_S_int_3 %42 %43 %46
OpStore %s1 %47
%49 = OpCompositeConstruct %S %int_1 %int_2
%51 = OpCompositeConstruct %S %int_0 %int_0
%52 = OpCompositeConstruct %S %int_5 %int_6
%53 = OpCompositeConstruct %_arr_S_int_3 %49 %51 %52
OpStore %s2 %53
%55 = OpCompositeConstruct %S %int_1 %int_2
%56 = OpCompositeConstruct %S %int_3 %int_4
%57 = OpCompositeConstruct %S %int_5 %int_6
%58 = OpCompositeConstruct %_arr_S_int_3 %55 %56 %57
OpStore %s3 %58
%60 = OpLoad %_arr_float_int_4 %f1
%61 = OpLoad %_arr_float_int_4 %f2
%62 = OpCompositeExtract %float %60 0
%63 = OpCompositeExtract %float %61 0
%64 = OpFOrdEqual %bool %62 %63
%65 = OpCompositeExtract %float %60 1
%66 = OpCompositeExtract %float %61 1
%67 = OpFOrdEqual %bool %65 %66
%68 = OpLogicalAnd %bool %67 %64
%69 = OpCompositeExtract %float %60 2
%70 = OpCompositeExtract %float %61 2
%71 = OpFOrdEqual %bool %69 %70
%72 = OpLogicalAnd %bool %71 %68
%73 = OpCompositeExtract %float %60 3
%74 = OpCompositeExtract %float %61 3
%75 = OpFOrdEqual %bool %73 %74
%76 = OpLogicalAnd %bool %75 %72
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %_arr_float_int_4 %f1
%80 = OpLoad %_arr_float_int_4 %f3
%81 = OpCompositeExtract %float %79 0
%82 = OpCompositeExtract %float %80 0
%83 = OpFOrdNotEqual %bool %81 %82
%84 = OpCompositeExtract %float %79 1
%85 = OpCompositeExtract %float %80 1
%86 = OpFOrdNotEqual %bool %84 %85
%87 = OpLogicalOr %bool %86 %83
%88 = OpCompositeExtract %float %79 2
%89 = OpCompositeExtract %float %80 2
%90 = OpFOrdNotEqual %bool %88 %89
%91 = OpLogicalOr %bool %90 %87
%92 = OpCompositeExtract %float %79 3
%93 = OpCompositeExtract %float %80 3
%94 = OpFOrdNotEqual %bool %92 %93
%95 = OpLogicalOr %bool %94 %91
OpBranch %78
%78 = OpLabel
%96 = OpPhi %bool %false %19 %95 %77
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %_arr_S_int_3 %s1
%100 = OpLoad %_arr_S_int_3 %s2
%101 = OpCompositeExtract %S %99 0
%102 = OpCompositeExtract %S %100 0
%103 = OpCompositeExtract %int %101 0
%104 = OpCompositeExtract %int %102 0
%105 = OpINotEqual %bool %103 %104
%106 = OpCompositeExtract %int %101 1
%107 = OpCompositeExtract %int %102 1
%108 = OpINotEqual %bool %106 %107
%109 = OpLogicalOr %bool %108 %105
%110 = OpCompositeExtract %S %99 1
%111 = OpCompositeExtract %S %100 1
%112 = OpCompositeExtract %int %110 0
%113 = OpCompositeExtract %int %111 0
%114 = OpINotEqual %bool %112 %113
%115 = OpCompositeExtract %int %110 1
%116 = OpCompositeExtract %int %111 1
%117 = OpINotEqual %bool %115 %116
%118 = OpLogicalOr %bool %117 %114
%119 = OpLogicalOr %bool %118 %109
%120 = OpCompositeExtract %S %99 2
%121 = OpCompositeExtract %S %100 2
%122 = OpCompositeExtract %int %120 0
%123 = OpCompositeExtract %int %121 0
%124 = OpINotEqual %bool %122 %123
%125 = OpCompositeExtract %int %120 1
%126 = OpCompositeExtract %int %121 1
%127 = OpINotEqual %bool %125 %126
%128 = OpLogicalOr %bool %127 %124
%129 = OpLogicalOr %bool %128 %119
OpBranch %98
%98 = OpLabel
%130 = OpPhi %bool %false %78 %129 %97
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %_arr_S_int_3 %s3
%134 = OpLoad %_arr_S_int_3 %s1
%135 = OpCompositeExtract %S %133 0
%136 = OpCompositeExtract %S %134 0
%137 = OpCompositeExtract %int %135 0
%138 = OpCompositeExtract %int %136 0
%139 = OpIEqual %bool %137 %138
%140 = OpCompositeExtract %int %135 1
%141 = OpCompositeExtract %int %136 1
%142 = OpIEqual %bool %140 %141
%143 = OpLogicalAnd %bool %142 %139
%144 = OpCompositeExtract %S %133 1
%145 = OpCompositeExtract %S %134 1
%146 = OpCompositeExtract %int %144 0
%147 = OpCompositeExtract %int %145 0
%148 = OpIEqual %bool %146 %147
%149 = OpCompositeExtract %int %144 1
%150 = OpCompositeExtract %int %145 1
%151 = OpIEqual %bool %149 %150
%152 = OpLogicalAnd %bool %151 %148
%153 = OpLogicalAnd %bool %152 %143
%154 = OpCompositeExtract %S %133 2
%155 = OpCompositeExtract %S %134 2
%156 = OpCompositeExtract %int %154 0
%157 = OpCompositeExtract %int %155 0
%158 = OpIEqual %bool %156 %157
%159 = OpCompositeExtract %int %154 1
%160 = OpCompositeExtract %int %155 1
%161 = OpIEqual %bool %159 %160
%162 = OpLogicalAnd %bool %161 %158
%163 = OpLogicalAnd %bool %162 %153
OpBranch %132
%132 = OpLabel
%164 = OpPhi %bool %false %98 %163 %131
OpSelectionMerge %169 None
OpBranchConditional %164 %167 %168
%167 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%172 = OpLoad %v4float %170
OpStore %165 %172
OpBranch %169
%168 = OpLabel
%173 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%174 = OpLoad %v4float %173
OpStore %165 %174
OpBranch %169
%169 = OpLabel
%175 = OpLoad %v4float %165
OpReturnValue %175
OpFunctionEnd
