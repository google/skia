OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputVal"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %negativeVal "negativeVal"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %negativeVal RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_n4 = OpConstant %float -4
%float_n16 = OpConstant %float -16
%float_n64 = OpConstant %float -64
%32 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%float_0_5 = OpConstant %float 0.5
%95 = OpConstantComposite %v2float %float_1 %float_0_5
%float_0_25 = OpConstant %float 0.25
%105 = OpConstantComposite %v3float %float_1 %float_0_5 %float_0_25
%float_0_125 = OpConstant %float 0.125
%115 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0_25 %float_0_125
%132 = OpConstantComposite %v2float %float_n1 %float_n4
%142 = OpConstantComposite %v3float %float_n1 %float_n4 %float_n16
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%negativeVal = OpVariable %_ptr_Function_v4float Function
%158 = OpVariable %_ptr_Function_v4float Function
OpStore %negativeVal %32
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%34 = OpExtInst %float %1 InverseSqrt %40
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %34 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%48 = OpExtInst %v2float %1 InverseSqrt %51
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFOrdEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%61 = OpExtInst %v3float %1 InverseSqrt %64
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%75 = OpExtInst %v4float %1 InverseSqrt %77
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%79 = OpLoad %v4float %78
%80 = OpFOrdEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%88 = OpLoad %v4float %87
%89 = OpCompositeExtract %float %88 0
%90 = OpFOrdEqual %bool %float_1 %89
OpBranch %85
%85 = OpLabel
%91 = OpPhi %bool %false %74 %90 %84
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v2float %97 %97 0 1
%99 = OpFOrdEqual %v2bool %95 %98
%100 = OpAll %bool %99
OpBranch %93
%93 = OpLabel
%101 = OpPhi %bool %false %85 %100 %92
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%109 = OpFOrdEqual %v3bool %105 %108
%110 = OpAll %bool %109
OpBranch %103
%103 = OpLabel
%111 = OpPhi %bool %false %93 %110 %102
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%117 = OpLoad %v4float %116
%118 = OpFOrdEqual %v4bool %115 %117
%119 = OpAll %bool %118
OpBranch %113
%113 = OpLabel
%120 = OpPhi %bool %false %103 %119 %112
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpExtInst %float %1 InverseSqrt %float_n1
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%125 = OpLoad %v4float %124
%126 = OpCompositeExtract %float %125 0
%127 = OpFOrdEqual %bool %123 %126
OpBranch %122
%122 = OpLabel
%128 = OpPhi %bool %false %113 %127 %121
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpExtInst %v2float %1 InverseSqrt %132
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %133
%135 = OpVectorShuffle %v2float %134 %134 0 1
%136 = OpFOrdEqual %v2bool %131 %135
%137 = OpAll %bool %136
OpBranch %130
%130 = OpLabel
%138 = OpPhi %bool %false %122 %137 %129
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpExtInst %v3float %1 InverseSqrt %142
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%146 = OpFOrdEqual %v3bool %141 %145
%147 = OpAll %bool %146
OpBranch %140
%140 = OpLabel
%148 = OpPhi %bool %false %130 %147 %139
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpLoad %v4float %negativeVal
%151 = OpExtInst %v4float %1 InverseSqrt %152
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %153
%155 = OpFOrdEqual %v4bool %151 %154
%156 = OpAll %bool %155
OpBranch %150
%150 = OpLabel
%157 = OpPhi %bool %false %140 %156 %149
OpSelectionMerge %161 None
OpBranchConditional %157 %159 %160
%159 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%164 = OpLoad %v4float %162
OpStore %158 %164
OpBranch %161
%160 = OpLabel
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%167 = OpLoad %v4float %165
OpStore %158 %167
OpBranch %161
%161 = OpLabel
%168 = OpLoad %v4float %158
OpReturnValue %168
OpFunctionEnd
