OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
%float_0_5 = OpConstant %float 0.5
%30 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%49 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%62 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%73 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%84 = OpConstantComposite %v2float %float_n1_25 %float_0
%91 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_5
%int_1 = OpConstant %int 1
%161 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0
%int_2 = OpConstant %int 2
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
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%171 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %30
OpStore %expectedB %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%35 = OpExtInst %float %1 FMin %41 %float_0_5
%42 = OpFOrdEqual %bool %35 %float_n1_25
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpExtInst %v2float %1 FMin %48 %49
%50 = OpVectorShuffle %v2float %30 %30 0 1
%51 = OpFOrdEqual %v2bool %45 %50
%53 = OpAll %bool %51
OpBranch %44
%44 = OpLabel
%54 = OpPhi %bool %false %25 %53 %43
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%57 = OpExtInst %v3float %1 FMin %60 %62
%63 = OpVectorShuffle %v3float %30 %30 0 1 2
%64 = OpFOrdEqual %v3bool %57 %63
%66 = OpAll %bool %64
OpBranch %56
%56 = OpLabel
%67 = OpPhi %bool %false %44 %66 %55
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%70 = OpExtInst %v4float %1 FMin %72 %73
%74 = OpFOrdEqual %v4bool %70 %30
%76 = OpAll %bool %74
OpBranch %69
%69 = OpLabel
%77 = OpPhi %bool %false %56 %76 %68
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpFOrdEqual %bool %float_n1_25 %float_n1_25
OpBranch %79
%79 = OpLabel
%81 = OpPhi %bool %false %69 %80 %78
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpVectorShuffle %v2float %30 %30 0 1
%86 = OpFOrdEqual %v2bool %84 %85
%87 = OpAll %bool %86
OpBranch %83
%83 = OpLabel
%88 = OpPhi %bool %false %79 %87 %82
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpVectorShuffle %v3float %30 %30 0 1 2
%93 = OpFOrdEqual %v3bool %91 %92
%94 = OpAll %bool %93
OpBranch %90
%90 = OpLabel
%95 = OpPhi %bool %false %83 %94 %89
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpFOrdEqual %v4bool %30 %30
%99 = OpAll %bool %98
OpBranch %97
%97 = OpLabel
%100 = OpPhi %bool %false %90 %99 %96
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%105 = OpLoad %v4float %104
%106 = OpCompositeExtract %float %105 0
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%109 = OpLoad %v4float %107
%110 = OpCompositeExtract %float %109 0
%103 = OpExtInst %float %1 FMin %106 %110
%111 = OpFOrdEqual %bool %103 %float_n1_25
OpBranch %102
%102 = OpLabel
%112 = OpPhi %bool %false %97 %111 %101
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%117 = OpLoad %v4float %116
%118 = OpVectorShuffle %v2float %117 %117 0 1
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%120 = OpLoad %v4float %119
%121 = OpVectorShuffle %v2float %120 %120 0 1
%115 = OpExtInst %v2float %1 FMin %118 %121
%122 = OpVectorShuffle %v2float %33 %33 0 1
%123 = OpFOrdEqual %v2bool %115 %122
%124 = OpAll %bool %123
OpBranch %114
%114 = OpLabel
%125 = OpPhi %bool %false %102 %124 %113
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %129
%131 = OpVectorShuffle %v3float %130 %130 0 1 2
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v3float %133 %133 0 1 2
%128 = OpExtInst %v3float %1 FMin %131 %134
%135 = OpVectorShuffle %v3float %33 %33 0 1 2
%136 = OpFOrdEqual %v3bool %128 %135
%137 = OpAll %bool %136
OpBranch %127
%127 = OpLabel
%138 = OpPhi %bool %false %114 %137 %126
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%143 = OpLoad %v4float %142
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%145 = OpLoad %v4float %144
%141 = OpExtInst %v4float %1 FMin %143 %145
%146 = OpFOrdEqual %v4bool %141 %33
%147 = OpAll %bool %146
OpBranch %140
%140 = OpLabel
%148 = OpPhi %bool %false %127 %147 %139
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpFOrdEqual %bool %float_n1_25 %float_n1_25
OpBranch %150
%150 = OpLabel
%152 = OpPhi %bool %false %140 %151 %149
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpVectorShuffle %v2float %33 %33 0 1
%156 = OpFOrdEqual %v2bool %84 %155
%157 = OpAll %bool %156
OpBranch %154
%154 = OpLabel
%158 = OpPhi %bool %false %150 %157 %153
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpVectorShuffle %v3float %33 %33 0 1 2
%163 = OpFOrdEqual %v3bool %161 %162
%164 = OpAll %bool %163
OpBranch %160
%160 = OpLabel
%165 = OpPhi %bool %false %154 %164 %159
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%168 = OpFOrdEqual %v4bool %33 %33
%169 = OpAll %bool %168
OpBranch %167
%167 = OpLabel
%170 = OpPhi %bool %false %160 %169 %166
OpSelectionMerge %174 None
OpBranchConditional %170 %172 %173
%172 = OpLabel
%175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%176 = OpLoad %v4float %175
OpStore %171 %176
OpBranch %174
%173 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%179 = OpLoad %v4float %177
OpStore %171 %179
OpBranch %174
%174 = OpLabel
%180 = OpLoad %v4float %171
OpReturnValue %180
OpFunctionEnd
