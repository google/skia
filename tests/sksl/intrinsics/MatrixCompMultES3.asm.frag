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
OpName %h24 "h24"
OpName %h42 "h42"
OpName %f43 "f43"
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
OpDecorate %h24 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %h42 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
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
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_9 = OpConstant %float 9
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%h24 = OpVariable %_ptr_Function_mat2v4float Function
%h42 = OpVariable %_ptr_Function_mat4v2float Function
%f43 = OpVariable %_ptr_Function_mat4v3float Function
%181 = OpVariable %_ptr_Function_v4float Function
%31 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_9
%32 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_9
%33 = OpCompositeConstruct %mat2v4float %31 %32
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%38 = OpLoad %v4float %34
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %39
%42 = OpCompositeConstruct %mat2v4float %38 %41
%43 = OpCompositeExtract %v4float %33 0
%44 = OpCompositeExtract %v4float %42 0
%45 = OpFMul %v4float %43 %44
%46 = OpCompositeExtract %v4float %33 1
%47 = OpCompositeExtract %v4float %42 1
%48 = OpFMul %v4float %46 %47
%49 = OpCompositeConstruct %mat2v4float %45 %48
OpStore %h24 %49
%62 = OpCompositeConstruct %v2float %float_1 %float_2
%63 = OpCompositeConstruct %v2float %float_3 %float_4
%64 = OpCompositeConstruct %v2float %float_5 %float_6
%65 = OpCompositeConstruct %v2float %float_7 %float_8
%66 = OpCompositeConstruct %mat4v2float %62 %63 %64 %65
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v2float %68 %68 0 1
%70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%71 = OpLoad %v4float %70
%72 = OpVectorShuffle %v2float %71 %71 2 3
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%75 = OpVectorShuffle %v2float %74 %74 0 1
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v2float %77 %77 2 3
%79 = OpCompositeConstruct %mat4v2float %69 %72 %75 %78
%80 = OpCompositeExtract %v2float %66 0
%81 = OpCompositeExtract %v2float %79 0
%82 = OpFMul %v2float %80 %81
%83 = OpCompositeExtract %v2float %66 1
%84 = OpCompositeExtract %v2float %79 1
%85 = OpFMul %v2float %83 %84
%86 = OpCompositeExtract %v2float %66 2
%87 = OpCompositeExtract %v2float %79 2
%88 = OpFMul %v2float %86 %87
%89 = OpCompositeExtract %v2float %66 3
%90 = OpCompositeExtract %v2float %79 3
%91 = OpFMul %v2float %89 %90
%92 = OpCompositeConstruct %mat4v2float %82 %85 %88 %91
OpStore %h42 %92
%103 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%104 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%105 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%106 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%107 = OpCompositeConstruct %mat4v3float %103 %104 %105 %106
OpStore %f43 %107
%109 = OpLoad %mat2v4float %h24
%110 = OpCompositeConstruct %v4float %float_9 %float_0 %float_0 %float_9
%111 = OpCompositeConstruct %v4float %float_0 %float_9 %float_0 %float_9
%112 = OpCompositeConstruct %mat2v4float %110 %111
%114 = OpCompositeExtract %v4float %109 0
%115 = OpCompositeExtract %v4float %112 0
%116 = OpFOrdEqual %v4bool %114 %115
%117 = OpAll %bool %116
%118 = OpCompositeExtract %v4float %109 1
%119 = OpCompositeExtract %v4float %112 1
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %117 %121
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %mat4v2float %h42
%126 = OpCompositeConstruct %v2float %float_1 %float_0
%127 = OpCompositeConstruct %v2float %float_0 %float_4
%128 = OpCompositeConstruct %v2float %float_0 %float_6
%129 = OpCompositeConstruct %v2float %float_0 %float_8
%130 = OpCompositeConstruct %mat4v2float %126 %127 %128 %129
%132 = OpCompositeExtract %v2float %125 0
%133 = OpCompositeExtract %v2float %130 0
%134 = OpFOrdEqual %v2bool %132 %133
%135 = OpAll %bool %134
%136 = OpCompositeExtract %v2float %125 1
%137 = OpCompositeExtract %v2float %130 1
%138 = OpFOrdEqual %v2bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
%141 = OpCompositeExtract %v2float %125 2
%142 = OpCompositeExtract %v2float %130 2
%143 = OpFOrdEqual %v2bool %141 %142
%144 = OpAll %bool %143
%145 = OpLogicalAnd %bool %140 %144
%146 = OpCompositeExtract %v2float %125 3
%147 = OpCompositeExtract %v2float %130 3
%148 = OpFOrdEqual %v2bool %146 %147
%149 = OpAll %bool %148
%150 = OpLogicalAnd %bool %145 %149
OpBranch %124
%124 = OpLabel
%151 = OpPhi %bool %false %25 %150 %123
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpLoad %mat4v3float %f43
%155 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%156 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%157 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%158 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%159 = OpCompositeConstruct %mat4v3float %155 %156 %157 %158
%161 = OpCompositeExtract %v3float %154 0
%162 = OpCompositeExtract %v3float %159 0
%163 = OpFOrdEqual %v3bool %161 %162
%164 = OpAll %bool %163
%165 = OpCompositeExtract %v3float %154 1
%166 = OpCompositeExtract %v3float %159 1
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
%170 = OpCompositeExtract %v3float %154 2
%171 = OpCompositeExtract %v3float %159 2
%172 = OpFOrdEqual %v3bool %170 %171
%173 = OpAll %bool %172
%174 = OpLogicalAnd %bool %169 %173
%175 = OpCompositeExtract %v3float %154 3
%176 = OpCompositeExtract %v3float %159 3
%177 = OpFOrdEqual %v3bool %175 %176
%178 = OpAll %bool %177
%179 = OpLogicalAnd %bool %174 %178
OpBranch %153
%153 = OpLabel
%180 = OpPhi %bool %false %124 %179 %152
OpSelectionMerge %185 None
OpBranchConditional %180 %183 %184
%183 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%187 = OpLoad %v4float %186
OpStore %181 %187
OpBranch %185
%184 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%189 = OpLoad %v4float %188
OpStore %181 %189
OpBranch %185
%185 = OpLabel
%190 = OpLoad %v4float %181
OpReturnValue %190
OpFunctionEnd
