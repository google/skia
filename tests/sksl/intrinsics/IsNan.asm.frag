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
OpName %valueIsNaN "valueIsNaN"
OpName %valueIsNumber "valueIsNumber"
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
OpDecorate %valueIsNaN RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %valueIsNumber RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%valueIsNaN = OpVariable %_ptr_Function_v4float Function
%valueIsNumber = OpVariable %_ptr_Function_v4float Function
%100 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%34 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%35 = OpFDiv %v4float %34 %33
OpStore %valueIsNaN %35
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%40 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%41 = OpFDiv %v4float %40 %39
OpStore %valueIsNumber %41
%44 = OpLoad %v4float %valueIsNaN
%45 = OpCompositeExtract %float %44 0
%43 = OpIsNan %bool %45
OpSelectionMerge %47 None
OpBranchConditional %43 %46 %47
%46 = OpLabel
%50 = OpLoad %v4float %valueIsNaN
%51 = OpVectorShuffle %v2float %50 %50 0 1
%49 = OpIsNan %v2bool %51
%48 = OpAll %bool %49
OpBranch %47
%47 = OpLabel
%53 = OpPhi %bool %false %25 %48 %46
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%58 = OpLoad %v4float %valueIsNaN
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%57 = OpIsNan %v3bool %59
%56 = OpAll %bool %57
OpBranch %55
%55 = OpLabel
%62 = OpPhi %bool %false %47 %56 %54
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%67 = OpLoad %v4float %valueIsNaN
%66 = OpIsNan %v4bool %67
%65 = OpAll %bool %66
OpBranch %64
%64 = OpLabel
%69 = OpPhi %bool %false %55 %65 %63
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%74 = OpLoad %v4float %valueIsNumber
%75 = OpCompositeExtract %float %74 0
%73 = OpIsNan %bool %75
%72 = OpLogicalNot %bool %73
OpBranch %71
%71 = OpLabel
%76 = OpPhi %bool %false %64 %72 %70
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%82 = OpLoad %v4float %valueIsNumber
%83 = OpVectorShuffle %v2float %82 %82 0 1
%81 = OpIsNan %v2bool %83
%80 = OpAny %bool %81
%79 = OpLogicalNot %bool %80
OpBranch %78
%78 = OpLabel
%84 = OpPhi %bool %false %71 %79 %77
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%90 = OpLoad %v4float %valueIsNumber
%91 = OpVectorShuffle %v3float %90 %90 0 1 2
%89 = OpIsNan %v3bool %91
%88 = OpAny %bool %89
%87 = OpLogicalNot %bool %88
OpBranch %86
%86 = OpLabel
%92 = OpPhi %bool %false %78 %87 %85
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%98 = OpLoad %v4float %valueIsNumber
%97 = OpIsNan %v4bool %98
%96 = OpAny %bool %97
%95 = OpLogicalNot %bool %96
OpBranch %94
%94 = OpLabel
%99 = OpPhi %bool %false %86 %95 %93
OpSelectionMerge %103 None
OpBranchConditional %99 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%106 = OpLoad %v4float %104
OpStore %100 %106
OpBranch %103
%102 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%109 = OpLoad %v4float %107
OpStore %100 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %100
OpReturnValue %110
OpFunctionEnd
