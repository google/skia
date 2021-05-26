OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputA"
OpMemberName %_UniformBuffer 1 "inputB"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
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
%float_5 = OpConstant %float 5
%float_17 = OpConstant %float 17
%float_38 = OpConstant %float 38
%float_70 = OpConstant %float 70
%32 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
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
%110 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %32
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%44 = OpCompositeExtract %float %43 0
%34 = OpFMul %float %40 %44
%45 = OpLoad %v4float %expected
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %34 %46
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%55 = OpLoad %v4float %54
%56 = OpVectorShuffle %v2float %55 %55 0 1
%50 = OpDot %float %53 %56
%57 = OpLoad %v4float %expected
%58 = OpCompositeExtract %float %57 1
%59 = OpFOrdEqual %bool %50 %58
OpBranch %49
%49 = OpLabel
%60 = OpPhi %bool %false %25 %59 %48
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%69 = OpLoad %v4float %68
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%63 = OpDot %float %66 %70
%71 = OpLoad %v4float %expected
%72 = OpCompositeExtract %float %71 2
%73 = OpFOrdEqual %bool %63 %72
OpBranch %62
%62 = OpLabel
%74 = OpPhi %bool %false %49 %73 %61
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %78
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%81 = OpLoad %v4float %80
%77 = OpDot %float %79 %81
%82 = OpLoad %v4float %expected
%83 = OpCompositeExtract %float %82 3
%84 = OpFOrdEqual %bool %77 %83
OpBranch %76
%76 = OpLabel
%85 = OpPhi %bool %false %62 %84 %75
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %v4float %expected
%89 = OpCompositeExtract %float %88 0
%90 = OpFOrdEqual %bool %float_5 %89
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %76 %90 %86
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %v4float %expected
%95 = OpCompositeExtract %float %94 1
%96 = OpFOrdEqual %bool %float_17 %95
OpBranch %93
%93 = OpLabel
%97 = OpPhi %bool %false %87 %96 %92
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %v4float %expected
%101 = OpCompositeExtract %float %100 2
%102 = OpFOrdEqual %bool %float_38 %101
OpBranch %99
%99 = OpLabel
%103 = OpPhi %bool %false %93 %102 %98
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %v4float %expected
%107 = OpCompositeExtract %float %106 3
%108 = OpFOrdEqual %bool %float_70 %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %99 %108 %104
OpSelectionMerge %113 None
OpBranchConditional %109 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%116 = OpLoad %v4float %114
OpStore %110 %116
OpBranch %113
%112 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%119 = OpLoad %v4float %117
OpStore %110 %119
OpBranch %113
%113 = OpLabel
%120 = OpLoad %v4float %110
OpReturnValue %120
OpFunctionEnd
