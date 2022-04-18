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
OpName %constGreen "constGreen"
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
OpDecorate %constGreen RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%31 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
%33 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
%false = OpConstantFalse %bool
%float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%47 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%60 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%72 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%91 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%115 = OpConstantComposite %v2float %float_0 %float_1
%126 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%145 = OpConstantComposite %v2float %float_1 %float_1
%152 = OpConstantComposite %v3float %float_1 %float_1 %float_0
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
%constGreen = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%162 = OpVariable %_ptr_Function_v4float Function
OpStore %constGreen %29
OpStore %expectedA %31
OpStore %expectedB %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%35 = OpExtInst %float %1 Step %float_0_5 %42
%43 = OpFOrdEqual %bool %35 %float_0
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%46 = OpExtInst %v2float %1 Step %47 %50
%51 = OpVectorShuffle %v2float %31 %31 0 1
%52 = OpFOrdEqual %v2bool %46 %51
%54 = OpAll %bool %52
OpBranch %45
%45 = OpLabel
%55 = OpPhi %bool %false %25 %54 %44
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%58 = OpExtInst %v3float %1 Step %60 %63
%64 = OpVectorShuffle %v3float %31 %31 0 1 2
%65 = OpFOrdEqual %v3bool %58 %64
%67 = OpAll %bool %65
OpBranch %57
%57 = OpLabel
%68 = OpPhi %bool %false %45 %67 %56
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%71 = OpExtInst %v4float %1 Step %72 %74
%75 = OpFOrdEqual %v4bool %71 %31
%77 = OpAll %bool %75
OpBranch %70
%70 = OpLabel
%78 = OpPhi %bool %false %57 %77 %69
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpFOrdEqual %bool %float_0 %float_0
OpBranch %80
%80 = OpLabel
%82 = OpPhi %bool %false %70 %81 %79
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpVectorShuffle %v2float %31 %31 0 1
%86 = OpFOrdEqual %v2bool %19 %85
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
%103 = OpExtInst %float %1 Step %106 %float_0
%107 = OpFOrdEqual %bool %103 %float_1
OpBranch %102
%102 = OpLabel
%108 = OpPhi %bool %false %97 %107 %101
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v2float %113 %113 0 1
%111 = OpExtInst %v2float %1 Step %114 %115
%116 = OpVectorShuffle %v2float %33 %33 0 1
%117 = OpFOrdEqual %v2bool %111 %116
%118 = OpAll %bool %117
OpBranch %110
%110 = OpLabel
%119 = OpPhi %bool %false %102 %118 %109
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%124 = OpLoad %v4float %123
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%122 = OpExtInst %v3float %1 Step %125 %126
%127 = OpVectorShuffle %v3float %33 %33 0 1 2
%128 = OpFOrdEqual %v3bool %122 %127
%129 = OpAll %bool %128
OpBranch %121
%121 = OpLabel
%130 = OpPhi %bool %false %110 %129 %120
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %134
%133 = OpExtInst %v4float %1 Step %135 %29
%136 = OpFOrdEqual %v4bool %133 %33
%137 = OpAll %bool %136
OpBranch %132
%132 = OpLabel
%138 = OpPhi %bool %false %121 %137 %131
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpFOrdEqual %bool %float_1 %float_1
OpBranch %140
%140 = OpLabel
%142 = OpPhi %bool %false %132 %141 %139
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%146 = OpVectorShuffle %v2float %33 %33 0 1
%147 = OpFOrdEqual %v2bool %145 %146
%148 = OpAll %bool %147
OpBranch %144
%144 = OpLabel
%149 = OpPhi %bool %false %140 %148 %143
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%153 = OpVectorShuffle %v3float %33 %33 0 1 2
%154 = OpFOrdEqual %v3bool %152 %153
%155 = OpAll %bool %154
OpBranch %151
%151 = OpLabel
%156 = OpPhi %bool %false %144 %155 %150
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpFOrdEqual %v4bool %33 %33
%160 = OpAll %bool %159
OpBranch %158
%158 = OpLabel
%161 = OpPhi %bool %false %151 %160 %157
OpSelectionMerge %165 None
OpBranchConditional %161 %163 %164
%163 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%168 = OpLoad %v4float %166
OpStore %162 %168
OpBranch %165
%164 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%171 = OpLoad %v4float %169
OpStore %162 %171
OpBranch %165
%165 = OpLabel
%172 = OpLoad %v4float %162
OpReturnValue %172
OpFunctionEnd
