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
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
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
%48 = OpConstantComposite %v2float %float_1 %float_1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%62 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%63 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%v3bool = OpTypeVector %bool 3
%75 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%76 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%97 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
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
%109 = OpVariable %_ptr_Function_v4float Function
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
%44 = OpExtInst %v2float %1 FClamp %47 %19 %48
%49 = OpLoad %v4float %expected
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpFOrdEqual %v2bool %44 %50
%53 = OpAll %bool %51
OpBranch %43
%43 = OpLabel
%54 = OpPhi %bool %false %25 %53 %42
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%57 = OpExtInst %v3float %1 FClamp %60 %62 %63
%64 = OpLoad %v4float %expected
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpFOrdEqual %v3bool %57 %65
%68 = OpAll %bool %66
OpBranch %56
%56 = OpLabel
%69 = OpPhi %bool %false %43 %68 %55
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%72 = OpExtInst %v4float %1 FClamp %74 %75 %76
%77 = OpLoad %v4float %expected
%78 = OpFOrdEqual %v4bool %72 %77
%80 = OpAll %bool %78
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %56 %80 %70
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %v4float %expected
%85 = OpCompositeExtract %float %84 0
%86 = OpFOrdEqual %bool %float_0 %85
OpBranch %83
%83 = OpLabel
%87 = OpPhi %bool %false %71 %86 %82
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %expected
%91 = OpVectorShuffle %v2float %90 %90 0 1
%92 = OpFOrdEqual %v2bool %19 %91
%93 = OpAll %bool %92
OpBranch %89
%89 = OpLabel
%94 = OpPhi %bool %false %83 %93 %88
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v4float %expected
%99 = OpVectorShuffle %v3float %98 %98 0 1 2
%100 = OpFOrdEqual %v3bool %97 %99
%101 = OpAll %bool %100
OpBranch %96
%96 = OpLabel
%102 = OpPhi %bool %false %89 %101 %95
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %v4float %expected
%106 = OpFOrdEqual %v4bool %30 %105
%107 = OpAll %bool %106
OpBranch %104
%104 = OpLabel
%108 = OpPhi %bool %false %96 %107 %103
OpSelectionMerge %112 None
OpBranchConditional %108 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%115 = OpLoad %v4float %113
OpStore %109 %115
OpBranch %112
%111 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%118 = OpLoad %v4float %116
OpStore %109 %118
OpBranch %112
%112 = OpLabel
%119 = OpLoad %v4float %109
OpReturnValue %119
OpFunctionEnd
