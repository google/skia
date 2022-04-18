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
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%50 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%63 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%74 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%91 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%int_1 = OpConstant %int 1
%155 = OpConstantComposite %v2float %float_0 %float_1
%162 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
%172 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %expectedB %34
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 FMax %42 %float_0_5
%43 = OpFOrdEqual %bool %36 %float_0_5
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%46 = OpExtInst %v2float %1 FMax %49 %50
%51 = OpVectorShuffle %v2float %31 %31 0 1
%52 = OpFOrdEqual %v2bool %46 %51
%54 = OpAll %bool %52
OpBranch %45
%45 = OpLabel
%55 = OpPhi %bool %false %25 %54 %44
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%58 = OpExtInst %v3float %1 FMax %61 %63
%64 = OpVectorShuffle %v3float %31 %31 0 1 2
%65 = OpFOrdEqual %v3bool %58 %64
%67 = OpAll %bool %65
OpBranch %57
%57 = OpLabel
%68 = OpPhi %bool %false %45 %67 %56
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%71 = OpExtInst %v4float %1 FMax %73 %74
%75 = OpFOrdEqual %v4bool %71 %31
%77 = OpAll %bool %75
OpBranch %70
%70 = OpLabel
%78 = OpPhi %bool %false %57 %77 %69
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpFOrdEqual %bool %float_0_5 %float_0_5
OpBranch %80
%80 = OpLabel
%82 = OpPhi %bool %false %70 %81 %79
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpVectorShuffle %v2float %31 %31 0 1
%86 = OpFOrdEqual %v2bool %50 %85
%87 = OpAll %bool %86
OpBranch %84
%84 = OpLabel
%88 = OpPhi %bool %false %80 %87 %83
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpVectorShuffle %v3float %31 %31 0 1 2
%93 = OpFOrdEqual %v3bool %91 %92
%94 = OpAll %bool %93
OpBranch %90
%90 = OpLabel
%95 = OpPhi %bool %false %84 %94 %89
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpFOrdEqual %v4bool %31 %31
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
%103 = OpExtInst %float %1 FMax %106 %110
%111 = OpFOrdEqual %bool %103 %float_0
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
%115 = OpExtInst %v2float %1 FMax %118 %121
%122 = OpVectorShuffle %v2float %34 %34 0 1
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
%128 = OpExtInst %v3float %1 FMax %131 %134
%135 = OpVectorShuffle %v3float %34 %34 0 1 2
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
%141 = OpExtInst %v4float %1 FMax %143 %145
%146 = OpFOrdEqual %v4bool %141 %34
%147 = OpAll %bool %146
OpBranch %140
%140 = OpLabel
%148 = OpPhi %bool %false %127 %147 %139
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpFOrdEqual %bool %float_0 %float_0
OpBranch %150
%150 = OpLabel
%152 = OpPhi %bool %false %140 %151 %149
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpVectorShuffle %v2float %34 %34 0 1
%157 = OpFOrdEqual %v2bool %155 %156
%158 = OpAll %bool %157
OpBranch %154
%154 = OpLabel
%159 = OpPhi %bool %false %150 %158 %153
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%163 = OpVectorShuffle %v3float %34 %34 0 1 2
%164 = OpFOrdEqual %v3bool %162 %163
%165 = OpAll %bool %164
OpBranch %161
%161 = OpLabel
%166 = OpPhi %bool %false %154 %165 %160
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpFOrdEqual %v4bool %34 %34
%170 = OpAll %bool %169
OpBranch %168
%168 = OpLabel
%171 = OpPhi %bool %false %161 %170 %167
OpSelectionMerge %175 None
OpBranchConditional %171 %173 %174
%173 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%177 = OpLoad %v4float %176
OpStore %172 %177
OpBranch %175
%174 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%180 = OpLoad %v4float %178
OpStore %172 %180
OpBranch %175
%175 = OpLabel
%181 = OpLoad %v4float %172
OpReturnValue %181
OpFunctionEnd
