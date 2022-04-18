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
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %clampLow RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%34 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%38 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%42 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%58 = OpConstantComposite %v2float %float_n1 %float_n1
%59 = OpConstantComposite %v2float %float_1 %float_1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%72 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
%73 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%v3bool = OpTypeVector %bool 3
%84 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%85 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%104 = OpConstantComposite %v2float %float_n1 %float_n2
%105 = OpConstantComposite %v2float %float_1 %float_2
%116 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%117 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%136 = OpConstantComposite %v2float %float_n1 %float_0
%143 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%165 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
%int_1 = OpConstant %int 1
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
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%175 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %clampLow %34
OpStore %expectedB %38
OpStore %clampHigh %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %45
%50 = OpCompositeExtract %float %49 0
%44 = OpExtInst %float %1 FClamp %50 %float_n1 %float_1
%51 = OpFOrdEqual %bool %44 %float_n1
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v2float %56 %56 0 1
%54 = OpExtInst %v2float %1 FClamp %57 %58 %59
%60 = OpVectorShuffle %v2float %31 %31 0 1
%61 = OpFOrdEqual %v2bool %54 %60
%63 = OpAll %bool %61
OpBranch %53
%53 = OpLabel
%64 = OpPhi %bool %false %25 %63 %52
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%69 = OpLoad %v4float %68
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%67 = OpExtInst %v3float %1 FClamp %70 %72 %73
%74 = OpVectorShuffle %v3float %31 %31 0 1 2
%75 = OpFOrdEqual %v3bool %67 %74
%77 = OpAll %bool %75
OpBranch %66
%66 = OpLabel
%78 = OpPhi %bool %false %53 %77 %65
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%83 = OpLoad %v4float %82
%81 = OpExtInst %v4float %1 FClamp %83 %84 %85
%86 = OpFOrdEqual %v4bool %81 %31
%88 = OpAll %bool %86
OpBranch %80
%80 = OpLabel
%89 = OpPhi %bool %false %66 %88 %79
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 0
%92 = OpExtInst %float %1 FClamp %95 %float_n1 %float_1
%96 = OpFOrdEqual %bool %92 %float_n1
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %80 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v2float %102 %102 0 1
%100 = OpExtInst %v2float %1 FClamp %103 %104 %105
%106 = OpVectorShuffle %v2float %38 %38 0 1
%107 = OpFOrdEqual %v2bool %100 %106
%108 = OpAll %bool %107
OpBranch %99
%99 = OpLabel
%109 = OpPhi %bool %false %91 %108 %98
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%114 = OpLoad %v4float %113
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%112 = OpExtInst %v3float %1 FClamp %115 %116 %117
%118 = OpVectorShuffle %v3float %38 %38 0 1 2
%119 = OpFOrdEqual %v3bool %112 %118
%120 = OpAll %bool %119
OpBranch %111
%111 = OpLabel
%121 = OpPhi %bool %false %99 %120 %110
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%126 = OpLoad %v4float %125
%124 = OpExtInst %v4float %1 FClamp %126 %34 %42
%127 = OpFOrdEqual %v4bool %124 %38
%128 = OpAll %bool %127
OpBranch %123
%123 = OpLabel
%129 = OpPhi %bool %false %111 %128 %122
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpFOrdEqual %bool %float_n1 %float_n1
OpBranch %131
%131 = OpLabel
%133 = OpPhi %bool %false %123 %132 %130
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpVectorShuffle %v2float %31 %31 0 1
%138 = OpFOrdEqual %v2bool %136 %137
%139 = OpAll %bool %138
OpBranch %135
%135 = OpLabel
%140 = OpPhi %bool %false %131 %139 %134
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpVectorShuffle %v3float %31 %31 0 1 2
%145 = OpFOrdEqual %v3bool %143 %144
%146 = OpAll %bool %145
OpBranch %142
%142 = OpLabel
%147 = OpPhi %bool %false %135 %146 %141
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpFOrdEqual %v4bool %31 %31
%151 = OpAll %bool %150
OpBranch %149
%149 = OpLabel
%152 = OpPhi %bool %false %142 %151 %148
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpFOrdEqual %bool %float_n1 %float_n1
OpBranch %154
%154 = OpLabel
%156 = OpPhi %bool %false %149 %155 %153
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpVectorShuffle %v2float %38 %38 0 1
%160 = OpFOrdEqual %v2bool %136 %159
%161 = OpAll %bool %160
OpBranch %158
%158 = OpLabel
%162 = OpPhi %bool %false %154 %161 %157
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpVectorShuffle %v3float %38 %38 0 1 2
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %158 %168 %163
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpFOrdEqual %v4bool %38 %38
%173 = OpAll %bool %172
OpBranch %171
%171 = OpLabel
%174 = OpPhi %bool %false %164 %173 %170
OpSelectionMerge %178 None
OpBranchConditional %174 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%181 = OpLoad %v4float %179
OpStore %175 %181
OpBranch %178
%177 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%184 = OpLoad %v4float %182
OpStore %175 %184
OpBranch %178
%178 = OpLabel
%185 = OpLoad %v4float %175
OpReturnValue %185
OpFunctionEnd
