OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "pos1"
OpMemberName %_UniformBuffer 1 "pos2"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_13 = OpConstant %float 13
%31 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%43 = OpCompositeExtract %float %42 0
%33 = OpExtInst %float %1 Distance %39 %43
%44 = OpLoad %v4float %expected
%45 = OpCompositeExtract %float %44 0
%46 = OpFOrdEqual %bool %33 %45
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v2float %54 %54 0 1
%49 = OpExtInst %float %1 Distance %52 %55
%56 = OpLoad %v4float %expected
%57 = OpCompositeExtract %float %56 1
%58 = OpFOrdEqual %bool %49 %57
OpBranch %48
%48 = OpLabel
%59 = OpPhi %bool %false %25 %58 %47
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%62 = OpExtInst %float %1 Distance %65 %69
%70 = OpLoad %v4float %expected
%71 = OpCompositeExtract %float %70 2
%72 = OpFOrdEqual %bool %62 %71
OpBranch %61
%61 = OpLabel
%73 = OpPhi %bool %false %48 %72 %60
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%80 = OpLoad %v4float %79
%76 = OpExtInst %float %1 Distance %78 %80
%81 = OpLoad %v4float %expected
%82 = OpCompositeExtract %float %81 3
%83 = OpFOrdEqual %bool %76 %82
OpBranch %75
%75 = OpLabel
%84 = OpPhi %bool %false %61 %83 %74
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %expected
%88 = OpCompositeExtract %float %87 0
%89 = OpFOrdEqual %bool %float_3 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %75 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4float %expected
%94 = OpCompositeExtract %float %93 1
%95 = OpFOrdEqual %bool %float_3 %94
OpBranch %92
%92 = OpLabel
%96 = OpPhi %bool %false %86 %95 %91
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4float %expected
%100 = OpCompositeExtract %float %99 2
%101 = OpFOrdEqual %bool %float_5 %100
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %92 %101 %97
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %v4float %expected
%106 = OpCompositeExtract %float %105 3
%107 = OpFOrdEqual %bool %float_13 %106
OpBranch %104
%104 = OpLabel
%108 = OpPhi %bool %false %98 %107 %103
OpSelectionMerge %112 None
OpBranchConditional %108 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %113
OpStore %109 %115
OpBranch %112
%111 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%118 = OpLoad %v4float %116
OpStore %109 %118
OpBranch %112
%112 = OpLabel
%119 = OpLoad %v4float %109
OpReturnValue %119
OpFunctionEnd
