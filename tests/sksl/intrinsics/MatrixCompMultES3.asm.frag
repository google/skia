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
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %h42 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
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
OpDecorate %106 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_9 = OpConstant %float 9
%31 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
%32 = OpConstantComposite %mat2v4float %31 %31
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
%59 = OpConstantComposite %v2float %float_1 %float_2
%60 = OpConstantComposite %v2float %float_3 %float_4
%61 = OpConstantComposite %v2float %float_5 %float_6
%62 = OpConstantComposite %v2float %float_7 %float_8
%63 = OpConstantComposite %mat4v2float %59 %60 %61 %62
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%100 = OpConstantComposite %v3float %float_12 %float_22 %float_30
%101 = OpConstantComposite %v3float %float_36 %float_40 %float_42
%102 = OpConstantComposite %v3float %float_42 %float_40 %float_36
%103 = OpConstantComposite %v3float %float_30 %float_22 %float_12
%104 = OpConstantComposite %mat4v3float %100 %101 %102 %103
%false = OpConstantFalse %bool
%107 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
%108 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
%109 = OpConstantComposite %mat2v4float %107 %108
%v4bool = OpTypeVector %bool 4
%121 = OpConstantComposite %v2float %float_1 %float_0
%122 = OpConstantComposite %v2float %float_0 %float_4
%123 = OpConstantComposite %v2float %float_0 %float_6
%124 = OpConstantComposite %v2float %float_0 %float_8
%125 = OpConstantComposite %mat4v2float %121 %122 %123 %124
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
%163 = OpVariable %_ptr_Function_v4float Function
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%37 = OpLoad %v4float %33
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %38
%41 = OpCompositeConstruct %mat2v4float %37 %40
%42 = OpCompositeExtract %v4float %41 0
%43 = OpFMul %v4float %31 %42
%44 = OpCompositeExtract %v4float %41 1
%45 = OpFMul %v4float %31 %44
%46 = OpCompositeConstruct %mat2v4float %43 %45
OpStore %h24 %46
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%65 = OpLoad %v4float %64
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %65 0
%69 = OpCompositeExtract %float %65 1
%70 = OpCompositeConstruct %v2float %68 %69
%71 = OpCompositeExtract %float %65 2
%72 = OpCompositeExtract %float %65 3
%73 = OpCompositeConstruct %v2float %71 %72
%74 = OpCompositeExtract %float %67 0
%75 = OpCompositeExtract %float %67 1
%76 = OpCompositeConstruct %v2float %74 %75
%77 = OpCompositeExtract %float %67 2
%78 = OpCompositeExtract %float %67 3
%79 = OpCompositeConstruct %v2float %77 %78
%80 = OpCompositeConstruct %mat4v2float %70 %73 %76 %79
%81 = OpCompositeExtract %v2float %80 0
%82 = OpFMul %v2float %59 %81
%83 = OpCompositeExtract %v2float %80 1
%84 = OpFMul %v2float %60 %83
%85 = OpCompositeExtract %v2float %80 2
%86 = OpFMul %v2float %61 %85
%87 = OpCompositeExtract %v2float %80 3
%88 = OpFMul %v2float %62 %87
%89 = OpCompositeConstruct %mat4v2float %82 %84 %86 %88
OpStore %h42 %89
OpStore %f43 %104
%106 = OpLoad %mat2v4float %h24
%111 = OpCompositeExtract %v4float %106 0
%112 = OpFOrdEqual %v4bool %111 %107
%113 = OpAll %bool %112
%114 = OpCompositeExtract %v4float %106 1
%115 = OpFOrdEqual %v4bool %114 %108
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %113 %116
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpLoad %mat4v2float %h42
%127 = OpCompositeExtract %v2float %120 0
%128 = OpFOrdEqual %v2bool %127 %121
%129 = OpAll %bool %128
%130 = OpCompositeExtract %v2float %120 1
%131 = OpFOrdEqual %v2bool %130 %122
%132 = OpAll %bool %131
%133 = OpLogicalAnd %bool %129 %132
%134 = OpCompositeExtract %v2float %120 2
%135 = OpFOrdEqual %v2bool %134 %123
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %133 %136
%138 = OpCompositeExtract %v2float %120 3
%139 = OpFOrdEqual %v2bool %138 %124
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %137 %140
OpBranch %119
%119 = OpLabel
%142 = OpPhi %bool %false %25 %141 %118
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpLoad %mat4v3float %f43
%147 = OpCompositeExtract %v3float %145 0
%148 = OpFOrdEqual %v3bool %147 %100
%149 = OpAll %bool %148
%150 = OpCompositeExtract %v3float %145 1
%151 = OpFOrdEqual %v3bool %150 %101
%152 = OpAll %bool %151
%153 = OpLogicalAnd %bool %149 %152
%154 = OpCompositeExtract %v3float %145 2
%155 = OpFOrdEqual %v3bool %154 %102
%156 = OpAll %bool %155
%157 = OpLogicalAnd %bool %153 %156
%158 = OpCompositeExtract %v3float %145 3
%159 = OpFOrdEqual %v3bool %158 %103
%160 = OpAll %bool %159
%161 = OpLogicalAnd %bool %157 %160
OpBranch %144
%144 = OpLabel
%162 = OpPhi %bool %false %119 %161 %143
OpSelectionMerge %167 None
OpBranchConditional %162 %165 %166
%165 = OpLabel
%168 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%169 = OpLoad %v4float %168
OpStore %163 %169
OpBranch %167
%166 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%171 = OpLoad %v4float %170
OpStore %163 %171
OpBranch %167
%167 = OpLabel
%172 = OpLoad %v4float %163
OpReturnValue %172
OpFunctionEnd
