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
OpDecorate %32 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
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
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
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
%109 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %27
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %30
%33 = OpCompositeExtract %float %32 0
%34 = OpConvertFToS %int %33
%35 = OpCompositeExtract %float %32 1
%36 = OpConvertFToS %int %35
%37 = OpCompositeExtract %float %32 2
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %32 3
%40 = OpConvertFToS %int %39
%41 = OpCompositeConstruct %v4int %34 %36 %38 %40
%42 = OpCompositeExtract %int %41 0
%29 = OpExtInst %int %1 SAbs %42
%43 = OpLoad %v4int %expected
%44 = OpCompositeExtract %int %43 0
%45 = OpIEqual %bool %29 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 0
%52 = OpConvertFToS %int %51
%53 = OpCompositeExtract %float %50 1
%54 = OpConvertFToS %int %53
%55 = OpCompositeExtract %float %50 2
%56 = OpConvertFToS %int %55
%57 = OpCompositeExtract %float %50 3
%58 = OpConvertFToS %int %57
%59 = OpCompositeConstruct %v4int %52 %54 %56 %58
%60 = OpVectorShuffle %v2int %59 %59 0 1
%48 = OpExtInst %v2int %1 SAbs %60
%62 = OpLoad %v4int %expected
%63 = OpVectorShuffle %v2int %62 %62 0 1
%64 = OpIEqual %v2bool %48 %63
%66 = OpAll %bool %64
OpBranch %47
%47 = OpLabel
%67 = OpPhi %bool %false %19 %66 %46
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%73 = OpCompositeExtract %float %72 0
%74 = OpConvertFToS %int %73
%75 = OpCompositeExtract %float %72 1
%76 = OpConvertFToS %int %75
%77 = OpCompositeExtract %float %72 2
%78 = OpConvertFToS %int %77
%79 = OpCompositeExtract %float %72 3
%80 = OpConvertFToS %int %79
%81 = OpCompositeConstruct %v4int %74 %76 %78 %80
%82 = OpVectorShuffle %v3int %81 %81 0 1 2
%70 = OpExtInst %v3int %1 SAbs %82
%84 = OpLoad %v4int %expected
%85 = OpVectorShuffle %v3int %84 %84 0 1 2
%86 = OpIEqual %v3bool %70 %85
%88 = OpAll %bool %86
OpBranch %69
%69 = OpLabel
%89 = OpPhi %bool %false %47 %88 %68
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 0
%96 = OpConvertFToS %int %95
%97 = OpCompositeExtract %float %94 1
%98 = OpConvertFToS %int %97
%99 = OpCompositeExtract %float %94 2
%100 = OpConvertFToS %int %99
%101 = OpCompositeExtract %float %94 3
%102 = OpConvertFToS %int %101
%103 = OpCompositeConstruct %v4int %96 %98 %100 %102
%92 = OpExtInst %v4int %1 SAbs %103
%104 = OpLoad %v4int %expected
%105 = OpIEqual %v4bool %92 %104
%107 = OpAll %bool %105
OpBranch %91
%91 = OpLabel
%108 = OpPhi %bool %false %69 %107 %90
OpSelectionMerge %113 None
OpBranchConditional %108 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%115 = OpLoad %v4float %114
OpStore %109 %115
OpBranch %113
%112 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%117 = OpLoad %v4float %116
OpStore %109 %117
OpBranch %113
%113 = OpLabel
%118 = OpLoad %v4float %109
OpReturnValue %118
OpFunctionEnd
