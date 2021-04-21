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
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%33 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%expected = OpVariable %_ptr_Function_v4int Function
%102 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpConvertFToS %int %39
%35 = OpExtInst %int %1 SAbs %40
%41 = OpLoad %v4int %expected
%42 = OpCompositeExtract %int %41 0
%43 = OpIEqual %bool %35 %42
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeConstruct %v2int %51 %53
%46 = OpExtInst %v2int %1 SAbs %54
%56 = OpLoad %v4int %expected
%57 = OpVectorShuffle %v2int %56 %56 0 1
%58 = OpIEqual %v2bool %46 %57
%60 = OpAll %bool %58
OpBranch %45
%45 = OpLabel
%61 = OpPhi %bool %false %25 %60 %44
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%69 = OpCompositeExtract %float %67 0
%70 = OpConvertFToS %int %69
%71 = OpCompositeExtract %float %67 1
%72 = OpConvertFToS %int %71
%73 = OpCompositeExtract %float %67 2
%74 = OpConvertFToS %int %73
%75 = OpCompositeConstruct %v3int %70 %72 %74
%64 = OpExtInst %v3int %1 SAbs %75
%77 = OpLoad %v4int %expected
%78 = OpVectorShuffle %v3int %77 %77 0 1 2
%79 = OpIEqual %v3bool %64 %78
%81 = OpAll %bool %79
OpBranch %63
%63 = OpLabel
%82 = OpPhi %bool %false %45 %81 %62
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpCompositeExtract %float %87 0
%89 = OpConvertFToS %int %88
%90 = OpCompositeExtract %float %87 1
%91 = OpConvertFToS %int %90
%92 = OpCompositeExtract %float %87 2
%93 = OpConvertFToS %int %92
%94 = OpCompositeExtract %float %87 3
%95 = OpConvertFToS %int %94
%96 = OpCompositeConstruct %v4int %89 %91 %93 %95
%85 = OpExtInst %v4int %1 SAbs %96
%97 = OpLoad %v4int %expected
%98 = OpIEqual %v4bool %85 %97
%100 = OpAll %bool %98
OpBranch %84
%84 = OpLabel
%101 = OpPhi %bool %false %63 %100 %83
OpSelectionMerge %106 None
OpBranchConditional %101 %104 %105
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%108 = OpLoad %v4float %107
OpStore %102 %108
OpBranch %106
%105 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%110 = OpLoad %v4float %109
OpStore %102 %110
OpBranch %106
%106 = OpLabel
%111 = OpLoad %v4float %102
OpReturnValue %111
OpFunctionEnd
