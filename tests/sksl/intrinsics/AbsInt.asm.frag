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
OpDecorate %32 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%27 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2float = OpTypeVector %float 2
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%expected = OpVariable %_ptr_Function_v4int Function
%97 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %27
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %30
%33 = OpCompositeExtract %float %32 0
%34 = OpConvertFToS %int %33
%29 = OpExtInst %int %1 SAbs %34
%35 = OpLoad %v4int %expected
%36 = OpCompositeExtract %int %35 0
%37 = OpIEqual %bool %29 %36
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%42 = OpLoad %v4float %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
%45 = OpCompositeExtract %float %43 0
%46 = OpConvertFToS %int %45
%47 = OpCompositeExtract %float %43 1
%48 = OpConvertFToS %int %47
%49 = OpCompositeConstruct %v2int %46 %48
%40 = OpExtInst %v2int %1 SAbs %49
%51 = OpLoad %v4int %expected
%52 = OpVectorShuffle %v2int %51 %51 0 1
%53 = OpIEqual %v2bool %40 %52
%55 = OpAll %bool %53
OpBranch %39
%39 = OpLabel
%56 = OpPhi %bool %false %19 %55 %38
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%64 = OpCompositeExtract %float %62 0
%65 = OpConvertFToS %int %64
%66 = OpCompositeExtract %float %62 1
%67 = OpConvertFToS %int %66
%68 = OpCompositeExtract %float %62 2
%69 = OpConvertFToS %int %68
%70 = OpCompositeConstruct %v3int %65 %67 %69
%59 = OpExtInst %v3int %1 SAbs %70
%72 = OpLoad %v4int %expected
%73 = OpVectorShuffle %v3int %72 %72 0 1 2
%74 = OpIEqual %v3bool %59 %73
%76 = OpAll %bool %74
OpBranch %58
%58 = OpLabel
%77 = OpPhi %bool %false %39 %76 %57
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %81
%83 = OpCompositeExtract %float %82 0
%84 = OpConvertFToS %int %83
%85 = OpCompositeExtract %float %82 1
%86 = OpConvertFToS %int %85
%87 = OpCompositeExtract %float %82 2
%88 = OpConvertFToS %int %87
%89 = OpCompositeExtract %float %82 3
%90 = OpConvertFToS %int %89
%91 = OpCompositeConstruct %v4int %84 %86 %88 %90
%80 = OpExtInst %v4int %1 SAbs %91
%92 = OpLoad %v4int %expected
%93 = OpIEqual %v4bool %80 %92
%95 = OpAll %bool %93
OpBranch %79
%79 = OpLabel
%96 = OpPhi %bool %false %58 %95 %78
OpSelectionMerge %101 None
OpBranchConditional %96 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%103 = OpLoad %v4float %102
OpStore %97 %103
OpBranch %101
%100 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%105 = OpLoad %v4float %104
OpStore %97 %105
OpBranch %101
%101 = OpLabel
%106 = OpLoad %v4float %97
OpReturnValue %106
OpFunctionEnd
