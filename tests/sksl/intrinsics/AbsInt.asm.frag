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
OpDecorate %39 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
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
%v4int = OpTypeVector %int 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%32 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
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
%91 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%28 = OpConvertFToS %int %27
%21 = OpExtInst %int %1 SAbs %28
%33 = OpCompositeExtract %int %32 0
%34 = OpIEqual %bool %21 %33
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%40 = OpVectorShuffle %v2float %39 %39 0 1
%42 = OpCompositeExtract %float %40 0
%43 = OpConvertFToS %int %42
%44 = OpCompositeExtract %float %40 1
%45 = OpConvertFToS %int %44
%46 = OpCompositeConstruct %v2int %43 %45
%37 = OpExtInst %v2int %1 SAbs %46
%48 = OpVectorShuffle %v2int %32 %32 0 1
%49 = OpIEqual %v2bool %37 %48
%51 = OpAll %bool %49
OpBranch %36
%36 = OpLabel
%52 = OpPhi %bool %false %19 %51 %35
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%60 = OpCompositeExtract %float %58 0
%61 = OpConvertFToS %int %60
%62 = OpCompositeExtract %float %58 1
%63 = OpConvertFToS %int %62
%64 = OpCompositeExtract %float %58 2
%65 = OpConvertFToS %int %64
%66 = OpCompositeConstruct %v3int %61 %63 %65
%55 = OpExtInst %v3int %1 SAbs %66
%68 = OpVectorShuffle %v3int %32 %32 0 1 2
%69 = OpIEqual %v3bool %55 %68
%71 = OpAll %bool %69
OpBranch %54
%54 = OpLabel
%72 = OpPhi %bool %false %36 %71 %53
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeExtract %float %77 0
%79 = OpConvertFToS %int %78
%80 = OpCompositeExtract %float %77 1
%81 = OpConvertFToS %int %80
%82 = OpCompositeExtract %float %77 2
%83 = OpConvertFToS %int %82
%84 = OpCompositeExtract %float %77 3
%85 = OpConvertFToS %int %84
%86 = OpCompositeConstruct %v4int %79 %81 %83 %85
%75 = OpExtInst %v4int %1 SAbs %86
%87 = OpIEqual %v4bool %75 %32
%89 = OpAll %bool %87
OpBranch %74
%74 = OpLabel
%90 = OpPhi %bool %false %54 %89 %73
OpSelectionMerge %95 None
OpBranchConditional %90 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%97 = OpLoad %v4float %96
OpStore %91 %97
OpBranch %95
%94 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%99 = OpLoad %v4float %98
OpStore %91 %99
OpBranch %95
%95 = OpLabel
%100 = OpLoad %v4float %91
OpReturnValue %100
OpFunctionEnd
