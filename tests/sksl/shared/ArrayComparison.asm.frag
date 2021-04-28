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
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
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
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_1 = OpConstant %int 1
%float_3 = OpConstant %float 3
%int_2 = OpConstant %int 2
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%float_n4 = OpConstant %float -4
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%_ptr_Function_S = OpTypePointer Function %S
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
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
%188 = OpVariable %_ptr_Function_v4float Function
%33 = OpAccessChain %_ptr_Function_float %f1 %int_0
OpStore %33 %float_1
%37 = OpAccessChain %_ptr_Function_float %f1 %int_1
OpStore %37 %float_2
%40 = OpAccessChain %_ptr_Function_float %f1 %int_2
OpStore %40 %float_3
%43 = OpAccessChain %_ptr_Function_float %f1 %int_3
OpStore %43 %float_4
%45 = OpAccessChain %_ptr_Function_float %f2 %int_0
OpStore %45 %float_1
%46 = OpAccessChain %_ptr_Function_float %f2 %int_1
OpStore %46 %float_2
%47 = OpAccessChain %_ptr_Function_float %f2 %int_2
OpStore %47 %float_3
%48 = OpAccessChain %_ptr_Function_float %f2 %int_3
OpStore %48 %float_4
%50 = OpAccessChain %_ptr_Function_float %f3 %int_0
OpStore %50 %float_1
%51 = OpAccessChain %_ptr_Function_float %f3 %int_1
OpStore %51 %float_2
%52 = OpAccessChain %_ptr_Function_float %f3 %int_2
OpStore %52 %float_3
%54 = OpAccessChain %_ptr_Function_float %f3 %int_3
OpStore %54 %float_n4
%59 = OpCompositeConstruct %S %int_1 %int_2
%60 = OpAccessChain %_ptr_Function_S %s1 %int_0
OpStore %60 %59
%62 = OpCompositeConstruct %S %int_3 %int_4
%63 = OpAccessChain %_ptr_Function_S %s1 %int_1
OpStore %63 %62
%66 = OpCompositeConstruct %S %int_5 %int_6
%67 = OpAccessChain %_ptr_Function_S %s1 %int_2
OpStore %67 %66
%69 = OpCompositeConstruct %S %int_1 %int_2
%70 = OpAccessChain %_ptr_Function_S %s2 %int_0
OpStore %70 %69
%71 = OpCompositeConstruct %S %int_0 %int_0
%72 = OpAccessChain %_ptr_Function_S %s2 %int_1
OpStore %72 %71
%73 = OpCompositeConstruct %S %int_5 %int_6
%74 = OpAccessChain %_ptr_Function_S %s2 %int_2
OpStore %74 %73
%76 = OpCompositeConstruct %S %int_1 %int_2
%77 = OpAccessChain %_ptr_Function_S %s3 %int_0
OpStore %77 %76
%78 = OpCompositeConstruct %S %int_3 %int_4
%79 = OpAccessChain %_ptr_Function_S %s3 %int_1
OpStore %79 %78
%80 = OpCompositeConstruct %S %int_5 %int_6
%81 = OpAccessChain %_ptr_Function_S %s3 %int_2
OpStore %81 %80
%83 = OpLoad %_arr_float_int_4 %f1
%84 = OpLoad %_arr_float_int_4 %f2
%85 = OpCompositeExtract %float %83 0
%86 = OpCompositeExtract %float %84 0
%87 = OpFOrdEqual %bool %85 %86
%88 = OpCompositeExtract %float %83 1
%89 = OpCompositeExtract %float %84 1
%90 = OpFOrdEqual %bool %88 %89
%91 = OpLogicalAnd %bool %90 %87
%92 = OpCompositeExtract %float %83 2
%93 = OpCompositeExtract %float %84 2
%94 = OpFOrdEqual %bool %92 %93
%95 = OpLogicalAnd %bool %94 %91
%96 = OpCompositeExtract %float %83 3
%97 = OpCompositeExtract %float %84 3
%98 = OpFOrdEqual %bool %96 %97
%99 = OpLogicalAnd %bool %98 %95
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %_arr_float_int_4 %f1
%103 = OpLoad %_arr_float_int_4 %f3
%104 = OpCompositeExtract %float %102 0
%105 = OpCompositeExtract %float %103 0
%106 = OpFOrdNotEqual %bool %104 %105
%107 = OpCompositeExtract %float %102 1
%108 = OpCompositeExtract %float %103 1
%109 = OpFOrdNotEqual %bool %107 %108
%110 = OpLogicalOr %bool %109 %106
%111 = OpCompositeExtract %float %102 2
%112 = OpCompositeExtract %float %103 2
%113 = OpFOrdNotEqual %bool %111 %112
%114 = OpLogicalOr %bool %113 %110
%115 = OpCompositeExtract %float %102 3
%116 = OpCompositeExtract %float %103 3
%117 = OpFOrdNotEqual %bool %115 %116
%118 = OpLogicalOr %bool %117 %114
OpBranch %101
%101 = OpLabel
%119 = OpPhi %bool %false %25 %118 %100
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %_arr_S_int_3 %s1
%123 = OpLoad %_arr_S_int_3 %s2
%124 = OpCompositeExtract %S %122 0
%125 = OpCompositeExtract %S %123 0
%126 = OpCompositeExtract %int %124 0
%127 = OpCompositeExtract %int %125 0
%128 = OpINotEqual %bool %126 %127
%129 = OpCompositeExtract %int %124 1
%130 = OpCompositeExtract %int %125 1
%131 = OpINotEqual %bool %129 %130
%132 = OpLogicalOr %bool %131 %128
%133 = OpCompositeExtract %S %122 1
%134 = OpCompositeExtract %S %123 1
%135 = OpCompositeExtract %int %133 0
%136 = OpCompositeExtract %int %134 0
%137 = OpINotEqual %bool %135 %136
%138 = OpCompositeExtract %int %133 1
%139 = OpCompositeExtract %int %134 1
%140 = OpINotEqual %bool %138 %139
%141 = OpLogicalOr %bool %140 %137
%142 = OpLogicalOr %bool %141 %132
%143 = OpCompositeExtract %S %122 2
%144 = OpCompositeExtract %S %123 2
%145 = OpCompositeExtract %int %143 0
%146 = OpCompositeExtract %int %144 0
%147 = OpINotEqual %bool %145 %146
%148 = OpCompositeExtract %int %143 1
%149 = OpCompositeExtract %int %144 1
%150 = OpINotEqual %bool %148 %149
%151 = OpLogicalOr %bool %150 %147
%152 = OpLogicalOr %bool %151 %142
OpBranch %121
%121 = OpLabel
%153 = OpPhi %bool %false %101 %152 %120
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %_arr_S_int_3 %s3
%157 = OpLoad %_arr_S_int_3 %s1
%158 = OpCompositeExtract %S %156 0
%159 = OpCompositeExtract %S %157 0
%160 = OpCompositeExtract %int %158 0
%161 = OpCompositeExtract %int %159 0
%162 = OpIEqual %bool %160 %161
%163 = OpCompositeExtract %int %158 1
%164 = OpCompositeExtract %int %159 1
%165 = OpIEqual %bool %163 %164
%166 = OpLogicalAnd %bool %165 %162
%167 = OpCompositeExtract %S %156 1
%168 = OpCompositeExtract %S %157 1
%169 = OpCompositeExtract %int %167 0
%170 = OpCompositeExtract %int %168 0
%171 = OpIEqual %bool %169 %170
%172 = OpCompositeExtract %int %167 1
%173 = OpCompositeExtract %int %168 1
%174 = OpIEqual %bool %172 %173
%175 = OpLogicalAnd %bool %174 %171
%176 = OpLogicalAnd %bool %175 %166
%177 = OpCompositeExtract %S %156 2
%178 = OpCompositeExtract %S %157 2
%179 = OpCompositeExtract %int %177 0
%180 = OpCompositeExtract %int %178 0
%181 = OpIEqual %bool %179 %180
%182 = OpCompositeExtract %int %177 1
%183 = OpCompositeExtract %int %178 1
%184 = OpIEqual %bool %182 %183
%185 = OpLogicalAnd %bool %184 %181
%186 = OpLogicalAnd %bool %185 %176
OpBranch %155
%155 = OpLabel
%187 = OpPhi %bool %false %121 %186 %154
OpSelectionMerge %192 None
OpBranchConditional %187 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%195 = OpLoad %v4float %193
OpStore %188 %195
OpBranch %192
%191 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%197 = OpLoad %v4float %196
OpStore %188 %197
OpBranch %192
%192 = OpLabel
%198 = OpLoad %v4float %188
OpReturnValue %198
OpFunctionEnd
