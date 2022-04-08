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
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %h42 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
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
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
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
%31 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
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
%61 = OpConstantComposite %v2float %float_1 %float_2
%62 = OpConstantComposite %v2float %float_3 %float_4
%63 = OpConstantComposite %v2float %float_5 %float_6
%64 = OpConstantComposite %v2float %float_7 %float_8
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%106 = OpConstantComposite %v3float %float_12 %float_22 %float_30
%107 = OpConstantComposite %v3float %float_36 %float_40 %float_42
%108 = OpConstantComposite %v3float %float_42 %float_40 %float_36
%109 = OpConstantComposite %v3float %float_30 %float_22 %float_12
%false = OpConstantFalse %bool
%113 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
%114 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
%v4bool = OpTypeVector %bool 4
%129 = OpConstantComposite %v2float %float_1 %float_0
%130 = OpConstantComposite %v2float %float_0 %float_4
%131 = OpConstantComposite %v2float %float_0 %float_6
%132 = OpConstantComposite %v2float %float_0 %float_8
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
%180 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %mat2v4float %31 %31
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%37 = OpLoad %v4float %33
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %38
%41 = OpCompositeConstruct %mat2v4float %37 %40
%42 = OpCompositeExtract %v4float %32 0
%43 = OpCompositeExtract %v4float %41 0
%44 = OpFMul %v4float %42 %43
%45 = OpCompositeExtract %v4float %32 1
%46 = OpCompositeExtract %v4float %41 1
%47 = OpFMul %v4float %45 %46
%48 = OpCompositeConstruct %mat2v4float %44 %47
OpStore %h24 %48
%65 = OpCompositeConstruct %mat4v2float %61 %62 %63 %64
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%67 = OpLoad %v4float %66
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%69 = OpLoad %v4float %68
%70 = OpCompositeExtract %float %67 0
%71 = OpCompositeExtract %float %67 1
%72 = OpCompositeConstruct %v2float %70 %71
%73 = OpCompositeExtract %float %67 2
%74 = OpCompositeExtract %float %67 3
%75 = OpCompositeConstruct %v2float %73 %74
%76 = OpCompositeExtract %float %69 0
%77 = OpCompositeExtract %float %69 1
%78 = OpCompositeConstruct %v2float %76 %77
%79 = OpCompositeExtract %float %69 2
%80 = OpCompositeExtract %float %69 3
%81 = OpCompositeConstruct %v2float %79 %80
%82 = OpCompositeConstruct %mat4v2float %72 %75 %78 %81
%83 = OpCompositeExtract %v2float %65 0
%84 = OpCompositeExtract %v2float %82 0
%85 = OpFMul %v2float %83 %84
%86 = OpCompositeExtract %v2float %65 1
%87 = OpCompositeExtract %v2float %82 1
%88 = OpFMul %v2float %86 %87
%89 = OpCompositeExtract %v2float %65 2
%90 = OpCompositeExtract %v2float %82 2
%91 = OpFMul %v2float %89 %90
%92 = OpCompositeExtract %v2float %65 3
%93 = OpCompositeExtract %v2float %82 3
%94 = OpFMul %v2float %92 %93
%95 = OpCompositeConstruct %mat4v2float %85 %88 %91 %94
OpStore %h42 %95
%110 = OpCompositeConstruct %mat4v3float %106 %107 %108 %109
OpStore %f43 %110
%112 = OpLoad %mat2v4float %h24
%115 = OpCompositeConstruct %mat2v4float %113 %114
%117 = OpCompositeExtract %v4float %112 0
%118 = OpCompositeExtract %v4float %115 0
%119 = OpFOrdEqual %v4bool %117 %118
%120 = OpAll %bool %119
%121 = OpCompositeExtract %v4float %112 1
%122 = OpCompositeExtract %v4float %115 1
%123 = OpFOrdEqual %v4bool %121 %122
%124 = OpAll %bool %123
%125 = OpLogicalAnd %bool %120 %124
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %mat4v2float %h42
%133 = OpCompositeConstruct %mat4v2float %129 %130 %131 %132
%135 = OpCompositeExtract %v2float %128 0
%136 = OpCompositeExtract %v2float %133 0
%137 = OpFOrdEqual %v2bool %135 %136
%138 = OpAll %bool %137
%139 = OpCompositeExtract %v2float %128 1
%140 = OpCompositeExtract %v2float %133 1
%141 = OpFOrdEqual %v2bool %139 %140
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %138 %142
%144 = OpCompositeExtract %v2float %128 2
%145 = OpCompositeExtract %v2float %133 2
%146 = OpFOrdEqual %v2bool %144 %145
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %143 %147
%149 = OpCompositeExtract %v2float %128 3
%150 = OpCompositeExtract %v2float %133 3
%151 = OpFOrdEqual %v2bool %149 %150
%152 = OpAll %bool %151
%153 = OpLogicalAnd %bool %148 %152
OpBranch %127
%127 = OpLabel
%154 = OpPhi %bool %false %25 %153 %126
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %mat4v3float %f43
%158 = OpCompositeConstruct %mat4v3float %106 %107 %108 %109
%160 = OpCompositeExtract %v3float %157 0
%161 = OpCompositeExtract %v3float %158 0
%162 = OpFOrdEqual %v3bool %160 %161
%163 = OpAll %bool %162
%164 = OpCompositeExtract %v3float %157 1
%165 = OpCompositeExtract %v3float %158 1
%166 = OpFOrdEqual %v3bool %164 %165
%167 = OpAll %bool %166
%168 = OpLogicalAnd %bool %163 %167
%169 = OpCompositeExtract %v3float %157 2
%170 = OpCompositeExtract %v3float %158 2
%171 = OpFOrdEqual %v3bool %169 %170
%172 = OpAll %bool %171
%173 = OpLogicalAnd %bool %168 %172
%174 = OpCompositeExtract %v3float %157 3
%175 = OpCompositeExtract %v3float %158 3
%176 = OpFOrdEqual %v3bool %174 %175
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %173 %177
OpBranch %156
%156 = OpLabel
%179 = OpPhi %bool %false %127 %178 %155
OpSelectionMerge %184 None
OpBranchConditional %179 %182 %183
%182 = OpLabel
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%186 = OpLoad %v4float %185
OpStore %180 %186
OpBranch %184
%183 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%188 = OpLoad %v4float %187
OpStore %180 %188
OpBranch %184
%184 = OpLabel
%189 = OpLoad %v4float %180
OpReturnValue %189
OpFunctionEnd
