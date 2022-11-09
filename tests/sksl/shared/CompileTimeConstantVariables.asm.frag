OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %kConstant "kConstant"
OpName %kOtherConstant "kOtherConstant"
OpName %kAnotherConstant "kAnotherConstant"
OpName %kFloatConstant "kFloatConstant"
OpName %kFloatConstantAlias "kFloatConstantAlias"
OpName %kConstVec "kConstVec"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %kLocalFloatConstant "kLocalFloatConstant"
OpName %kLocalFloatConstantAlias "kLocalFloatConstantAlias"
OpName %integerInput "integerInput"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %kConstVec RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %28 Binding 0
OpDecorate %28 DescriptorSet 0
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%kConstant = OpVariable %_ptr_Private_int Private
%int_0 = OpConstant %int 0
%kOtherConstant = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%kAnotherConstant = OpVariable %_ptr_Private_int Private
%int_2 = OpConstant %int 2
%_ptr_Private_float = OpTypePointer Private %float
%kFloatConstant = OpVariable %_ptr_Private_float Private
%float_2_1400001 = OpConstant %float 2.1400001
%kFloatConstantAlias = OpVariable %_ptr_Private_float Private
%_ptr_Private_v4float = OpTypePointer Private %v4float
%kConstVec = OpVariable %_ptr_Private_v4float Private
%float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
%27 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%28 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%33 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%37 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%41 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_3_1400001 = OpConstant %float 3.1400001
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%60 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
%82 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
%93 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%94 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %33
%34 = OpLabel
%38 = OpVariable %_ptr_Function_v2float Function
OpStore %38 %37
%40 = OpFunctionCall %v4float %main %38
OpStore %sk_FragColor %40
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %41
%42 = OpFunctionParameter %_ptr_Function_v2float
%43 = OpLabel
%kLocalFloatConstant = OpVariable %_ptr_Function_float Function
%kLocalFloatConstantAlias = OpVariable %_ptr_Function_float Function
%integerInput = OpVariable %_ptr_Function_int Function
OpStore %kConstant %int_0
OpStore %kOtherConstant %int_1
OpStore %kAnotherConstant %int_2
OpStore %kFloatConstant %float_2_1400001
%22 = OpLoad %float %kFloatConstant
OpStore %kFloatConstantAlias %22
OpStore %kConstVec %27
OpStore %kLocalFloatConstant %float_3_1400001
OpStore %kLocalFloatConstantAlias %float_3_1400001
%50 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%52 = OpLoad %v4float %50
%53 = OpCompositeExtract %float %52 1
%54 = OpConvertFToS %int %53
OpStore %integerInput %54
%55 = OpLoad %int %kConstant
%56 = OpIEqual %bool %54 %55
OpSelectionMerge %59 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
OpReturnValue %60
%58 = OpLabel
%61 = OpLoad %int %kOtherConstant
%62 = OpIEqual %bool %54 %61
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%67 = OpLoad %v4float %66
OpReturnValue %67
%64 = OpLabel
%68 = OpLoad %int %kAnotherConstant
%69 = OpIEqual %bool %54 %68
OpSelectionMerge %72 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpLoad %v4float %kConstVec
OpReturnValue %73
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeExtract %float %75 0
%77 = OpFMul %float %76 %float_3_1400001
%78 = OpFOrdLessThan %bool %float_3_1400001 %77
OpSelectionMerge %81 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
OpReturnValue %82
%80 = OpLabel
%83 = OpLoad %float %kFloatConstantAlias
%84 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %float %kFloatConstantAlias
%88 = OpFMul %float %86 %87
%89 = OpFOrdGreaterThanEqual %bool %83 %88
OpSelectionMerge %92 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
OpReturnValue %93
%91 = OpLabel
OpReturnValue %94
%92 = OpLabel
OpBranch %81
%81 = OpLabel
OpBranch %72
%72 = OpLabel
OpBranch %65
%65 = OpLabel
OpBranch %59
%59 = OpLabel
OpUnreachable
OpFunctionEnd
