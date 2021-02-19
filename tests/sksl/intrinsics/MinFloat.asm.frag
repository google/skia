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
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
%float_0 = OpConstant %float 0
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%81 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
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
%121 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FMin %27 %float_0_5
%32 = OpCompositeExtract %float %31 0
%33 = OpFOrdEqual %bool %21 %32
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %37
%39 = OpVectorShuffle %v2float %38 %38 0 1
%41 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%36 = OpExtInst %v2float %1 FMin %39 %41
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
%49 = OpExtInst %v3float %1 FMin %52 %54
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
%62 = OpExtInst %v4float %1 FMin %64 %65
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
%72 = OpExtInst %float %1 FMin %75 %79
%82 = OpCompositeExtract %float %81 0
%83 = OpFOrdEqual %bool %72 %82
OpBranch %71
%71 = OpLabel
%84 = OpPhi %bool %false %61 %83 %70
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v2float %89 %89 0 1
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v2float %92 %92 0 1
%87 = OpExtInst %v2float %1 FMin %90 %93
%94 = OpVectorShuffle %v2float %81 %81 0 1
%95 = OpFOrdEqual %v2bool %87 %94
%96 = OpAll %bool %95
OpBranch %86
%86 = OpLabel
%97 = OpPhi %bool %false %71 %96 %85
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%100 = OpExtInst %v3float %1 FMin %103 %106
%107 = OpVectorShuffle %v3float %81 %81 0 1 2
%108 = OpFOrdEqual %v3bool %100 %107
%109 = OpAll %bool %108
OpBranch %99
%99 = OpLabel
%110 = OpPhi %bool %false %86 %109 %98
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%115 = OpLoad %v4float %114
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%117 = OpLoad %v4float %116
%113 = OpExtInst %v4float %1 FMin %115 %117
%118 = OpFOrdEqual %v4bool %113 %81
%119 = OpAll %bool %118
OpBranch %112
%112 = OpLabel
%120 = OpPhi %bool %false %99 %119 %111
OpSelectionMerge %125 None
OpBranchConditional %120 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%127 = OpLoad %v4float %126
OpStore %121 %127
OpBranch %125
%124 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%130 = OpLoad %v4float %128
OpStore %121 %130
OpBranch %125
%125 = OpLabel
%131 = OpLoad %v4float %121
OpReturnValue %131
OpFunctionEnd
