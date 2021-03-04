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
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%40 = OpConstantComposite %v2float %float_n1_25 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%53 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_5
%v3bool = OpTypeVector %bool 3
%64 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%102 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0
%float_1 = OpConstant %float 1
%114 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
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
%118 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FMin %27 %float_0_5
%30 = OpFOrdEqual %bool %21 %float_n1_25
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %34
%36 = OpVectorShuffle %v2float %35 %35 0 1
%38 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%33 = OpExtInst %v2float %1 FMin %36 %38
%41 = OpFOrdEqual %v2bool %33 %40
%43 = OpAll %bool %41
OpBranch %32
%32 = OpLabel
%44 = OpPhi %bool %false %19 %43 %31
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v3float %49 %49 0 1 2
%52 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%47 = OpExtInst %v3float %1 FMin %50 %52
%54 = OpFOrdEqual %v3bool %47 %53
%56 = OpAll %bool %54
OpBranch %46
%46 = OpLabel
%57 = OpPhi %bool %false %32 %56 %45
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%60 = OpExtInst %v4float %1 FMin %62 %63
%65 = OpFOrdEqual %v4bool %60 %64
%67 = OpAll %bool %65
OpBranch %59
%59 = OpLabel
%68 = OpPhi %bool %false %46 %67 %58
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 0
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%77 = OpLoad %v4float %75
%78 = OpCompositeExtract %float %77 0
%71 = OpExtInst %float %1 FMin %74 %78
%79 = OpFOrdEqual %bool %71 %float_n1_25
OpBranch %70
%70 = OpLabel
%80 = OpPhi %bool %false %59 %79 %69
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpVectorShuffle %v2float %85 %85 0 1
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%88 = OpLoad %v4float %87
%89 = OpVectorShuffle %v2float %88 %88 0 1
%83 = OpExtInst %v2float %1 FMin %86 %89
%90 = OpFOrdEqual %v2bool %83 %40
%91 = OpAll %bool %90
OpBranch %82
%82 = OpLabel
%92 = OpPhi %bool %false %70 %91 %81
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%95 = OpExtInst %v3float %1 FMin %98 %101
%103 = OpFOrdEqual %v3bool %95 %102
%104 = OpAll %bool %103
OpBranch %94
%94 = OpLabel
%105 = OpPhi %bool %false %82 %104 %93
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%110 = OpLoad %v4float %109
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%112 = OpLoad %v4float %111
%108 = OpExtInst %v4float %1 FMin %110 %112
%115 = OpFOrdEqual %v4bool %108 %114
%116 = OpAll %bool %115
OpBranch %107
%107 = OpLabel
%117 = OpPhi %bool %false %94 %116 %106
OpSelectionMerge %122 None
OpBranchConditional %117 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%124 = OpLoad %v4float %123
OpStore %118 %124
OpBranch %122
%121 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%127 = OpLoad %v4float %125
OpStore %118 %127
OpBranch %122
%122 = OpLabel
%128 = OpLoad %v4float %118
OpReturnValue %128
OpFunctionEnd
