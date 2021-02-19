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
OpDecorate %26 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
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
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%82 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%122 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FMax %27 %float_0_5
%32 = OpCompositeExtract %float %31 0
%33 = OpFOrdEqual %bool %21 %32
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %37
%39 = OpVectorShuffle %v2float %38 %38 0 1
%41 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%36 = OpExtInst %v2float %1 FMax %39 %41
%42 = OpVectorShuffle %v2float %31 %31 0 1
%43 = OpFOrdEqual %v2bool %36 %42
%45 = OpAll %bool %43
OpBranch %35
%35 = OpLabel
%46 = OpPhi %bool %false %19 %45 %34
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v3float %51 %51 0 1 2
%54 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%49 = OpExtInst %v3float %1 FMax %52 %54
%55 = OpVectorShuffle %v3float %31 %31 0 1 2
%56 = OpFOrdEqual %v3bool %49 %55
%58 = OpAll %bool %56
OpBranch %48
%48 = OpLabel
%59 = OpPhi %bool %false %35 %58 %47
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%62 = OpExtInst %v4float %1 FMax %64 %65
%66 = OpFOrdEqual %v4bool %62 %31
%68 = OpAll %bool %66
OpBranch %61
%61 = OpLabel
%69 = OpPhi %bool %false %48 %68 %60
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%75 = OpCompositeExtract %float %74 0
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%78 = OpLoad %v4float %76
%79 = OpCompositeExtract %float %78 0
%72 = OpExtInst %float %1 FMax %75 %79
%83 = OpCompositeExtract %float %82 0
%84 = OpFOrdEqual %bool %72 %83
OpBranch %71
%71 = OpLabel
%85 = OpPhi %bool %false %61 %84 %70
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%90 = OpLoad %v4float %89
%91 = OpVectorShuffle %v2float %90 %90 0 1
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%93 = OpLoad %v4float %92
%94 = OpVectorShuffle %v2float %93 %93 0 1
%88 = OpExtInst %v2float %1 FMax %91 %94
%95 = OpVectorShuffle %v2float %82 %82 0 1
%96 = OpFOrdEqual %v2bool %88 %95
%97 = OpAll %bool %96
OpBranch %87
%87 = OpLabel
%98 = OpPhi %bool %false %71 %97 %86
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %102
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%106 = OpLoad %v4float %105
%107 = OpVectorShuffle %v3float %106 %106 0 1 2
%101 = OpExtInst %v3float %1 FMax %104 %107
%108 = OpVectorShuffle %v3float %82 %82 0 1 2
%109 = OpFOrdEqual %v3bool %101 %108
%110 = OpAll %bool %109
OpBranch %100
%100 = OpLabel
%111 = OpPhi %bool %false %87 %110 %99
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%118 = OpLoad %v4float %117
%114 = OpExtInst %v4float %1 FMax %116 %118
%119 = OpFOrdEqual %v4bool %114 %82
%120 = OpAll %bool %119
OpBranch %113
%113 = OpLabel
%121 = OpPhi %bool %false %100 %120 %112
OpSelectionMerge %126 None
OpBranchConditional %121 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%128 = OpLoad %v4float %127
OpStore %122 %128
OpBranch %126
%125 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%131 = OpLoad %v4float %129
OpStore %122 %131
OpBranch %126
%126 = OpLabel
%132 = OpLoad %v4float %122
OpReturnValue %132
OpFunctionEnd
