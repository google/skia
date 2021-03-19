OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
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
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_0 = OpConstant %float 0
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%26 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%29 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%37 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%expectedA = OpVariable %_ptr_Function_v4float Function
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%146 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %26
OpStore %clampLow %29
OpStore %expectedB %33
OpStore %clampHigh %37
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %40
%45 = OpCompositeExtract %float %44 0
%39 = OpExtInst %float %1 FClamp %45 %float_n1 %float_1
%46 = OpLoad %v4float %expectedA
%47 = OpCompositeExtract %float %46 0
%48 = OpFOrdEqual %bool %39 %47
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%56 = OpCompositeConstruct %v2float %float_n1 %float_n1
%57 = OpCompositeConstruct %v2float %float_1 %float_1
%51 = OpExtInst %v2float %1 FClamp %54 %56 %57
%58 = OpLoad %v4float %expectedA
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpFOrdEqual %v2bool %51 %59
%62 = OpAll %bool %60
OpBranch %50
%50 = OpLabel
%63 = OpPhi %bool %false %19 %62 %49
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%71 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%72 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%66 = OpExtInst %v3float %1 FClamp %69 %71 %72
%73 = OpLoad %v4float %expectedA
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%75 = OpFOrdEqual %v3bool %66 %74
%77 = OpAll %bool %75
OpBranch %65
%65 = OpLabel
%78 = OpPhi %bool %false %50 %77 %64
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%83 = OpLoad %v4float %82
%84 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%85 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%81 = OpExtInst %v4float %1 FClamp %83 %84 %85
%86 = OpLoad %v4float %expectedA
%87 = OpFOrdEqual %v4bool %81 %86
%89 = OpAll %bool %87
OpBranch %80
%80 = OpLabel
%90 = OpPhi %bool %false %65 %89 %79
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%95 = OpLoad %v4float %94
%96 = OpCompositeExtract %float %95 0
%97 = OpLoad %v4float %clampLow
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v4float %clampHigh
%100 = OpCompositeExtract %float %99 0
%93 = OpExtInst %float %1 FClamp %96 %98 %100
%101 = OpLoad %v4float %expectedB
%102 = OpCompositeExtract %float %101 0
%103 = OpFOrdEqual %bool %93 %102
OpBranch %92
%92 = OpLabel
%104 = OpPhi %bool %false %80 %103 %91
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
%111 = OpLoad %v4float %clampLow
%112 = OpVectorShuffle %v2float %111 %111 0 1
%113 = OpLoad %v4float %clampHigh
%114 = OpVectorShuffle %v2float %113 %113 0 1
%107 = OpExtInst %v2float %1 FClamp %110 %112 %114
%115 = OpLoad %v4float %expectedB
%116 = OpVectorShuffle %v2float %115 %115 0 1
%117 = OpFOrdEqual %v2bool %107 %116
%118 = OpAll %bool %117
OpBranch %106
%106 = OpLabel
%119 = OpPhi %bool %false %92 %118 %105
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%124 = OpLoad %v4float %123
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%126 = OpLoad %v4float %clampLow
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpLoad %v4float %clampHigh
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%122 = OpExtInst %v3float %1 FClamp %125 %127 %129
%130 = OpLoad %v4float %expectedB
%131 = OpVectorShuffle %v3float %130 %130 0 1 2
%132 = OpFOrdEqual %v3bool %122 %131
%133 = OpAll %bool %132
OpBranch %121
%121 = OpLabel
%134 = OpPhi %bool %false %106 %133 %120
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%139 = OpLoad %v4float %138
%140 = OpLoad %v4float %clampLow
%141 = OpLoad %v4float %clampHigh
%137 = OpExtInst %v4float %1 FClamp %139 %140 %141
%142 = OpLoad %v4float %expectedB
%143 = OpFOrdEqual %v4bool %137 %142
%144 = OpAll %bool %143
OpBranch %136
%136 = OpLabel
%145 = OpPhi %bool %false %121 %144 %135
OpSelectionMerge %149 None
OpBranchConditional %145 %147 %148
%147 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%152 = OpLoad %v4float %150
OpStore %146 %152
OpBranch %149
%148 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%155 = OpLoad %v4float %153
OpStore %146 %155
OpBranch %149
%149 = OpLabel
%156 = OpLoad %v4float %146
OpReturnValue %156
OpFunctionEnd
