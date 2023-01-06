OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInput"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float %v4float %v4float
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
%int_1 = OpConstant %int 1
%float_0_5 = OpConstant %float 0.5
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%int_3 = OpConstant %int 3
%float_0_25 = OpConstant %float 0.25
%v3float = OpTypeVector %float 3
%46 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%float_0_75 = OpConstant %float 0.75
%53 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
%float_1 = OpConstant %float 1
%58 = OpConstantComposite %v4float %float_0_75 %float_1 %float_0_25 %float_1
%v4bool = OpTypeVector %bool 4
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
%color = OpVariable %_ptr_Function_v4float Function
%62 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%32 = OpLoad %v4float %28
%34 = OpVectorTimesScalar %v4float %32 %float_0_5
OpStore %color %34
%36 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %36 %float_2
%39 = OpAccessChain %_ptr_Function_float %color %int_1
%40 = OpLoad %float %39
%42 = OpFDiv %float %40 %float_0_25
OpStore %39 %42
%43 = OpLoad %v4float %color
%44 = OpVectorShuffle %v3float %43 %43 1 2 3
%47 = OpFMul %v3float %44 %46
%48 = OpLoad %v4float %color
%49 = OpVectorShuffle %v4float %48 %47 0 4 5 6
OpStore %color %49
%50 = OpLoad %v4float %color
%51 = OpVectorShuffle %v4float %50 %50 2 1 3 0
%54 = OpFAdd %v4float %51 %53
%55 = OpLoad %v4float %color
%56 = OpVectorShuffle %v4float %55 %54 7 5 4 6
OpStore %color %56
%59 = OpFOrdEqual %v4bool %56 %58
%61 = OpAll %bool %59
OpSelectionMerge %65 None
OpBranchConditional %61 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%67 = OpLoad %v4float %66
OpStore %62 %67
OpBranch %65
%64 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%70 = OpLoad %v4float %68
OpStore %62 %70
OpBranch %65
%65 = OpLabel
%71 = OpLoad %v4float %62
OpReturnValue %71
OpFunctionEnd
