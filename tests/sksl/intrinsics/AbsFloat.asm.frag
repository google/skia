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
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
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
%float_1_25 = OpConstant %float 1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_1_25 %float_0 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%86 = OpConstantComposite %v2float %float_1_25 %float_0
%94 = OpConstantComposite %v3float %float_1_25 %float_0 %float_0_75
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
%106 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%33 = OpExtInst %float %1 FAbs %39
%40 = OpLoad %v4float %expected
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %33 %41
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpExtInst %v2float %1 FAbs %48
%49 = OpLoad %v4float %expected
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpFOrdEqual %v2bool %45 %50
%53 = OpAll %bool %51
OpBranch %44
%44 = OpLabel
%54 = OpPhi %bool %false %25 %53 %43
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%57 = OpExtInst %v3float %1 FAbs %60
%62 = OpLoad %v4float %expected
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpFOrdEqual %v3bool %57 %63
%66 = OpAll %bool %64
OpBranch %56
%56 = OpLabel
%67 = OpPhi %bool %false %44 %66 %55
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%70 = OpExtInst %v4float %1 FAbs %72
%73 = OpLoad %v4float %expected
%74 = OpFOrdEqual %v4bool %70 %73
%76 = OpAll %bool %74
OpBranch %69
%69 = OpLabel
%77 = OpPhi %bool %false %56 %76 %68
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %v4float %expected
%81 = OpCompositeExtract %float %80 0
%82 = OpFOrdEqual %bool %float_1_25 %81
OpBranch %79
%79 = OpLabel
%83 = OpPhi %bool %false %69 %82 %78
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4float %expected
%88 = OpVectorShuffle %v2float %87 %87 0 1
%89 = OpFOrdEqual %v2bool %86 %88
%90 = OpAll %bool %89
OpBranch %85
%85 = OpLabel
%91 = OpPhi %bool %false %79 %90 %84
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpLoad %v4float %expected
%96 = OpVectorShuffle %v3float %95 %95 0 1 2
%97 = OpFOrdEqual %v3bool %94 %96
%98 = OpAll %bool %97
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %85 %98 %92
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %v4float %expected
%103 = OpFOrdEqual %v4bool %31 %102
%104 = OpAll %bool %103
OpBranch %101
%101 = OpLabel
%105 = OpPhi %bool %false %93 %104 %100
OpSelectionMerge %109 None
OpBranchConditional %105 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%112 = OpLoad %v4float %110
OpStore %106 %112
OpBranch %109
%108 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %113
OpStore %106 %115
OpBranch %109
%109 = OpLabel
%116 = OpLoad %v4float %106
OpReturnValue %116
OpFunctionEnd
