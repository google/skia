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
OpName %expected "expected"
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
OpDecorate %expected RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%98 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
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
%expected = OpVariable %_ptr_Function_v4float Function
%110 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%32 = OpExtInst %float %1 FClamp %38 %float_0 %float_1
%39 = OpLoad %v4float %expected
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %32 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v2float %46 %46 0 1
%48 = OpCompositeConstruct %v2float %float_0 %float_0
%49 = OpCompositeConstruct %v2float %float_1 %float_1
%44 = OpExtInst %v2float %1 FClamp %47 %48 %49
%50 = OpLoad %v4float %expected
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFOrdEqual %v2bool %44 %51
%54 = OpAll %bool %52
OpBranch %43
%43 = OpLabel
%55 = OpPhi %bool %false %25 %54 %42
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%63 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%64 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%58 = OpExtInst %v3float %1 FClamp %61 %63 %64
%65 = OpLoad %v4float %expected
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%67 = OpFOrdEqual %v3bool %58 %66
%69 = OpAll %bool %67
OpBranch %57
%57 = OpLabel
%70 = OpPhi %bool %false %43 %69 %56
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%73 = OpExtInst %v4float %1 FClamp %75 %76 %77
%78 = OpLoad %v4float %expected
%79 = OpFOrdEqual %v4bool %73 %78
%81 = OpAll %bool %79
OpBranch %72
%72 = OpLabel
%82 = OpPhi %bool %false %57 %81 %71
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %v4float %expected
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdEqual %bool %float_0 %86
OpBranch %84
%84 = OpLabel
%88 = OpPhi %bool %false %72 %87 %83
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %v4float %expected
%92 = OpVectorShuffle %v2float %91 %91 0 1
%93 = OpFOrdEqual %v2bool %19 %92
%94 = OpAll %bool %93
OpBranch %90
%90 = OpLabel
%95 = OpPhi %bool %false %84 %94 %89
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpLoad %v4float %expected
%100 = OpVectorShuffle %v3float %99 %99 0 1 2
%101 = OpFOrdEqual %v3bool %98 %100
%102 = OpAll %bool %101
OpBranch %97
%97 = OpLabel
%103 = OpPhi %bool %false %90 %102 %96
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %v4float %expected
%107 = OpFOrdEqual %v4bool %30 %106
%108 = OpAll %bool %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %97 %108 %104
OpSelectionMerge %113 None
OpBranchConditional %109 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %114
OpStore %110 %116
OpBranch %113
%112 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%119 = OpLoad %v4float %117
OpStore %110 %119
OpBranch %113
%113 = OpLabel
%120 = OpLoad %v4float %110
OpReturnValue %120
OpFunctionEnd
