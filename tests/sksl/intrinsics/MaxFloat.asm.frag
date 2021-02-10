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
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%25 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
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
%expectedB = OpVariable %_ptr_Function_v4float Function
%138 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %25
OpStore %expectedB %29
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpExtInst %float %1 FMax %37 %float_0_5
%38 = OpLoad %v4float %expectedA
%39 = OpCompositeExtract %float %38 0
%40 = OpFOrdEqual %bool %31 %39
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %44
%46 = OpVectorShuffle %v2float %45 %45 0 1
%48 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%43 = OpExtInst %v2float %1 FMax %46 %48
%49 = OpLoad %v4float %expectedA
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpFOrdEqual %v2bool %43 %50
%53 = OpAll %bool %51
OpBranch %42
%42 = OpLabel
%54 = OpPhi %bool %false %19 %53 %41
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%62 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%57 = OpExtInst %v3float %1 FMax %60 %62
%63 = OpLoad %v4float %expectedA
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%65 = OpFOrdEqual %v3bool %57 %64
%67 = OpAll %bool %65
OpBranch %56
%56 = OpLabel
%68 = OpPhi %bool %false %42 %67 %55
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpVectorShuffle %v4float %73 %73 0 1 2 3
%75 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%71 = OpExtInst %v4float %1 FMax %74 %75
%76 = OpLoad %v4float %expectedA
%77 = OpVectorShuffle %v4float %76 %76 0 1 2 3
%78 = OpFOrdEqual %v4bool %71 %77
%80 = OpAll %bool %78
OpBranch %70
%70 = OpLabel
%81 = OpPhi %bool %false %56 %80 %69
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%90 = OpLoad %v4float %88
%91 = OpCompositeExtract %float %90 0
%84 = OpExtInst %float %1 FMax %87 %91
%92 = OpLoad %v4float %expectedB
%93 = OpCompositeExtract %float %92 0
%94 = OpFOrdEqual %bool %84 %93
OpBranch %83
%83 = OpLabel
%95 = OpPhi %bool %false %70 %94 %82
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v2float %100 %100 0 1
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%103 = OpLoad %v4float %102
%104 = OpVectorShuffle %v2float %103 %103 0 1
%98 = OpExtInst %v2float %1 FMax %101 %104
%105 = OpLoad %v4float %expectedB
%106 = OpVectorShuffle %v2float %105 %105 0 1
%107 = OpFOrdEqual %v2bool %98 %106
%108 = OpAll %bool %107
OpBranch %97
%97 = OpLabel
%109 = OpPhi %bool %false %83 %108 %96
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%114 = OpLoad %v4float %113
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%117 = OpLoad %v4float %116
%118 = OpVectorShuffle %v3float %117 %117 0 1 2
%112 = OpExtInst %v3float %1 FMax %115 %118
%119 = OpLoad %v4float %expectedB
%120 = OpVectorShuffle %v3float %119 %119 0 1 2
%121 = OpFOrdEqual %v3bool %112 %120
%122 = OpAll %bool %121
OpBranch %111
%111 = OpLabel
%123 = OpPhi %bool %false %97 %122 %110
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v4float %128 %128 0 1 2 3
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%131 = OpLoad %v4float %130
%132 = OpVectorShuffle %v4float %131 %131 0 1 2 3
%126 = OpExtInst %v4float %1 FMax %129 %132
%133 = OpLoad %v4float %expectedB
%134 = OpVectorShuffle %v4float %133 %133 0 1 2 3
%135 = OpFOrdEqual %v4bool %126 %134
%136 = OpAll %bool %135
OpBranch %125
%125 = OpLabel
%137 = OpPhi %bool %false %111 %136 %124
OpSelectionMerge %141 None
OpBranchConditional %137 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%143 = OpLoad %v4float %142
OpStore %138 %143
OpBranch %141
%140 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%146 = OpLoad %v4float %144
OpStore %138 %146
OpBranch %141
%141 = OpLabel
%147 = OpLoad %v4float %138
OpReturnValue %147
OpFunctionEnd
