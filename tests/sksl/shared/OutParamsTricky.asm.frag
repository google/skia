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
OpName %tricky "tricky"
OpName %func "func"
OpName %t "t"
OpName %main "main"
OpName %result "result"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%21 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%38 = OpTypeFunction %void %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_5 = OpConstant %float 5
%59 = OpTypeFunction %v4float
%float_0 = OpConstant %float 0
%float_3 = OpConstant %float 3
%64 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%67 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%tricky = OpFunction %v2float None %21
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpFunctionParameter %_ptr_Function_float
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpLabel
%29 = OpLoad %v2float %26
%30 = OpVectorShuffle %v2float %29 %29 1 0
%31 = OpLoad %v2float %26
%32 = OpVectorShuffle %v2float %31 %30 2 3
OpStore %26 %32
%33 = OpLoad %float %24
%34 = OpLoad %float %25
%35 = OpFAdd %float %33 %34
%36 = OpLoad %float %27
%37 = OpCompositeConstruct %v2float %35 %36
OpReturnValue %37
OpFunctionEnd
%func = OpFunction %void None %38
%40 = OpFunctionParameter %_ptr_Function_v4float
%41 = OpLabel
%t = OpVariable %_ptr_Function_v2float Function
%44 = OpVariable %_ptr_Function_float Function
%46 = OpVariable %_ptr_Function_float Function
%49 = OpVariable %_ptr_Function_v2float Function
%51 = OpVariable %_ptr_Function_float Function
OpStore %44 %float_1
OpStore %46 %float_2
%47 = OpLoad %v4float %40
%48 = OpVectorShuffle %v2float %47 %47 0 2
OpStore %49 %48
OpStore %51 %float_5
%52 = OpFunctionCall %v2float %tricky %44 %46 %49 %51
%53 = OpLoad %v2float %49
%54 = OpLoad %v4float %40
%55 = OpVectorShuffle %v4float %54 %53 4 1 5 3
OpStore %40 %55
OpStore %t %52
%56 = OpLoad %v2float %t
%57 = OpLoad %v4float %40
%58 = OpVectorShuffle %v4float %57 %56 0 4 2 5
OpStore %40 %58
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %59
%60 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%71 = OpVariable %_ptr_Function_v4float Function
OpStore %result %64
%65 = OpFunctionCall %void %func %result
%66 = OpLoad %v4float %result
%68 = OpFOrdEqual %v4bool %66 %67
%70 = OpAll %bool %68
OpSelectionMerge %74 None
OpBranchConditional %70 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%79 = OpLoad %v4float %75
OpStore %71 %79
OpBranch %74
%73 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%82 = OpLoad %v4float %80
OpStore %71 %82
OpBranch %74
%74 = OpLabel
%83 = OpLoad %v4float %71
OpReturnValue %83
OpFunctionEnd
