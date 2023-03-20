OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_5 = OpConstant %float 0.5
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%int_3 = OpConstant %int 3
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%v3float = OpTypeVector %float 3
%47 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%55 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
%float_1 = OpConstant %float 1
%72 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
%v4bool = OpTypeVector %bool 4
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
%color = OpVariable %_ptr_Function_v4float Function
%64 = OpVariable %_ptr_Function_float Function
%76 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%34 = OpVectorTimesScalar %v4float %32 %float_0_5
OpStore %color %34
%36 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %36 %float_2
%39 = OpAccessChain %_ptr_Function_float %color %int_1
%41 = OpLoad %float %39
%43 = OpFMul %float %41 %float_4
OpStore %39 %43
%44 = OpLoad %v4float %color
%45 = OpVectorShuffle %v3float %44 %44 1 2 3
%48 = OpFMul %v3float %45 %47
%49 = OpLoad %v4float %color
%50 = OpVectorShuffle %v4float %49 %48 0 4 5 6
OpStore %color %50
%51 = OpLoad %v4float %color
%52 = OpVectorShuffle %v4float %51 %51 2 1 3 0
%56 = OpFAdd %v4float %52 %55
%57 = OpLoad %v4float %color
%58 = OpVectorShuffle %v4float %57 %56 7 5 4 6
OpStore %color %58
%59 = OpAccessChain %_ptr_Function_float %color %int_0
%60 = OpLoad %float %59
%61 = OpCompositeExtract %float %58 3
%63 = OpFOrdLessThanEqual %bool %61 %float_1
OpSelectionMerge %67 None
OpBranchConditional %63 %65 %66
%65 = OpLabel
%68 = OpCompositeExtract %float %58 2
OpStore %64 %68
OpBranch %67
%66 = OpLabel
OpStore %64 %float_0
OpBranch %67
%67 = OpLabel
%69 = OpLoad %float %64
%70 = OpFAdd %float %60 %69
OpStore %59 %70
%71 = OpLoad %v4float %color
%73 = OpFOrdEqual %v4bool %71 %72
%75 = OpAll %bool %73
OpSelectionMerge %79 None
OpBranchConditional %75 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%81 = OpLoad %v4float %80
OpStore %76 %81
OpBranch %79
%78 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%83 = OpLoad %v4float %82
OpStore %76 %83
OpBranch %79
%79 = OpLabel
%84 = OpLoad %v4float %76
OpReturnValue %84
OpFunctionEnd
