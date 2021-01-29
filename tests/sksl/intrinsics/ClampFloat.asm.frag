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
OpDecorate %39 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%41 = OpConstantComposite %v2float %float_n1 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_0_75 = OpConstant %float 0.75
%56 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%v3bool = OpTypeVector %bool 3
%68 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%88 = OpConstantComposite %v2float %float_n1 %float_n2
%float_2 = OpConstant %float 2
%90 = OpConstantComposite %v2float %float_1 %float_2
%100 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%float_0_5 = OpConstant %float 0.5
%102 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%103 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
%112 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_3 = OpConstant %float 3
%114 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%float_2_25 = OpConstant %float 2.25
%116 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%120 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FClamp %27 %float_n1 %float_1
%30 = OpFOrdEqual %bool %21 %float_n1
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %34
%36 = OpVectorShuffle %v2float %35 %35 0 1
%38 = OpCompositeConstruct %v2float %float_n1 %float_n1
%39 = OpCompositeConstruct %v2float %float_1 %float_1
%33 = OpExtInst %v2float %1 FClamp %36 %38 %39
%42 = OpFOrdEqual %v2bool %33 %41
%44 = OpAll %bool %42
OpBranch %32
%32 = OpLabel
%45 = OpPhi %bool %false %19 %44 %31
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%53 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%54 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%48 = OpExtInst %v3float %1 FClamp %51 %53 %54
%57 = OpFOrdEqual %v3bool %48 %56
%59 = OpAll %bool %57
OpBranch %47
%47 = OpLabel
%60 = OpPhi %bool %false %32 %59 %46
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %64
%66 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%67 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%63 = OpExtInst %v4float %1 FClamp %65 %66 %67
%69 = OpFOrdEqual %v4bool %63 %68
%71 = OpAll %bool %69
OpBranch %62
%62 = OpLabel
%72 = OpPhi %bool %false %47 %71 %61
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeExtract %float %77 0
%75 = OpExtInst %float %1 FClamp %78 %float_n1 %float_1
%79 = OpFOrdEqual %bool %75 %float_n1
OpBranch %74
%74 = OpLabel
%80 = OpPhi %bool %false %62 %79 %73
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpVectorShuffle %v2float %85 %85 0 1
%83 = OpExtInst %v2float %1 FClamp %86 %88 %90
%91 = OpFOrdEqual %v2bool %83 %41
%92 = OpAll %bool %91
OpBranch %82
%82 = OpLabel
%93 = OpPhi %bool %false %74 %92 %81
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%98 = OpLoad %v4float %97
%99 = OpVectorShuffle %v3float %98 %98 0 1 2
%96 = OpExtInst %v3float %1 FClamp %99 %100 %102
%104 = OpFOrdEqual %v3bool %96 %103
%105 = OpAll %bool %104
OpBranch %95
%95 = OpLabel
%106 = OpPhi %bool %false %82 %105 %94
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%111 = OpLoad %v4float %110
%109 = OpExtInst %v4float %1 FClamp %111 %112 %114
%117 = OpFOrdEqual %v4bool %109 %116
%118 = OpAll %bool %117
OpBranch %108
%108 = OpLabel
%119 = OpPhi %bool %false %95 %118 %107
OpSelectionMerge %124 None
OpBranchConditional %119 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%127 = OpLoad %v4float %125
OpStore %120 %127
OpBranch %124
%123 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%130 = OpLoad %v4float %128
OpStore %120 %130
OpBranch %124
%124 = OpLabel
%131 = OpLoad %v4float %120
OpReturnValue %131
OpFunctionEnd
