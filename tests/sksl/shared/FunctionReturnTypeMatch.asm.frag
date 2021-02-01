OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %returns_bool3 "returns_bool3"
OpName %returns_bool4 "returns_bool4"
OpName %returns_int "returns_int"
OpName %returns_int2 "returns_int2"
OpName %returns_int3 "returns_int3"
OpName %returns_int4 "returns_int4"
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
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%v3bool = OpTypeVector %bool 3
%25 = OpTypeFunction %v3bool
%true = OpConstantTrue %bool
%28 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%30 = OpTypeFunction %v4bool
%32 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%34 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%38 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%41 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%43 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%46 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%48 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%51 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%52 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%56 = OpConstantComposite %v2bool %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %21
%22 = OpLabel
%23 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%returns_bool3 = OpFunction %v3bool None %25
%26 = OpLabel
OpReturnValue %28
OpFunctionEnd
%returns_bool4 = OpFunction %v4bool None %30
%31 = OpLabel
OpReturnValue %32
OpFunctionEnd
%returns_int = OpFunction %int None %34
%35 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2 = OpFunction %v2int None %38
%39 = OpLabel
OpReturnValue %41
OpFunctionEnd
%returns_int3 = OpFunction %v3int None %43
%44 = OpLabel
OpReturnValue %46
OpFunctionEnd
%returns_int4 = OpFunction %v4int None %48
%49 = OpLabel
OpReturnValue %51
OpFunctionEnd
%main = OpFunction %v4float None %52
%53 = OpLabel
%94 = OpVariable %_ptr_Function_v4float Function
%57 = OpLogicalEqual %v2bool %56 %56
%58 = OpAll %bool %57
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpFunctionCall %v3bool %returns_bool3
%62 = OpLogicalEqual %v3bool %28 %61
%63 = OpAll %bool %62
OpBranch %60
%60 = OpLabel
%64 = OpPhi %bool %false %53 %63 %59
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpFunctionCall %v4bool %returns_bool4
%68 = OpLogicalEqual %v4bool %32 %67
%69 = OpAll %bool %68
OpBranch %66
%66 = OpLabel
%70 = OpPhi %bool %false %60 %69 %65
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpFunctionCall %int %returns_int
%74 = OpIEqual %bool %int_1 %73
OpBranch %72
%72 = OpLabel
%75 = OpPhi %bool %false %66 %74 %71
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpFunctionCall %v2int %returns_int2
%79 = OpIEqual %v2bool %41 %78
%80 = OpAll %bool %79
OpBranch %77
%77 = OpLabel
%81 = OpPhi %bool %false %72 %80 %76
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpFunctionCall %v3int %returns_int3
%85 = OpIEqual %v3bool %46 %84
%86 = OpAll %bool %85
OpBranch %83
%83 = OpLabel
%87 = OpPhi %bool %false %77 %86 %82
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpFunctionCall %v4int %returns_int4
%91 = OpIEqual %v4bool %51 %90
%92 = OpAll %bool %91
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %83 %92 %88
OpSelectionMerge %98 None
OpBranchConditional %93 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%102 = OpLoad %v4float %99
OpStore %94 %102
OpBranch %98
%97 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%104 = OpLoad %v4float %103
OpStore %94 %104
OpBranch %98
%98 = OpLabel
%105 = OpLoad %v4float %94
OpReturnValue %105
OpFunctionEnd
