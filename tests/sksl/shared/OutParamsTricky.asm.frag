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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
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
%36 = OpTypeFunction %void %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_5 = OpConstant %float 5
%57 = OpTypeFunction %v4float
%float_0 = OpConstant %float 0
%float_3 = OpConstant %float 3
%62 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%65 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
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
OpStore %26 %30
%31 = OpLoad %float %24
%32 = OpLoad %float %25
%33 = OpFAdd %float %31 %32
%34 = OpLoad %float %27
%35 = OpCompositeConstruct %v2float %33 %34
OpReturnValue %35
OpFunctionEnd
%func = OpFunction %void None %36
%38 = OpFunctionParameter %_ptr_Function_v4float
%39 = OpLabel
%t = OpVariable %_ptr_Function_v2float Function
%42 = OpVariable %_ptr_Function_float Function
%44 = OpVariable %_ptr_Function_float Function
%47 = OpVariable %_ptr_Function_v2float Function
%49 = OpVariable %_ptr_Function_float Function
OpStore %42 %float_1
OpStore %44 %float_2
%45 = OpLoad %v4float %38
%46 = OpVectorShuffle %v2float %45 %45 0 2
OpStore %47 %46
OpStore %49 %float_5
%50 = OpFunctionCall %v2float %tricky %42 %44 %47 %49
%51 = OpLoad %v2float %47
%52 = OpLoad %v4float %38
%53 = OpVectorShuffle %v4float %52 %51 4 1 5 3
OpStore %38 %53
OpStore %t %50
%54 = OpLoad %v2float %t
%55 = OpLoad %v4float %38
%56 = OpVectorShuffle %v4float %55 %54 0 4 2 5
OpStore %38 %56
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %57
%58 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%69 = OpVariable %_ptr_Function_v4float Function
OpStore %result %62
%63 = OpFunctionCall %void %func %result
%64 = OpLoad %v4float %result
%66 = OpFOrdEqual %v4bool %64 %65
%68 = OpAll %bool %66
OpSelectionMerge %72 None
OpBranchConditional %68 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%77 = OpLoad %v4float %73
OpStore %69 %77
OpBranch %72
%71 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%80 = OpLoad %v4float %78
OpStore %69 %80
OpBranch %72
%72 = OpLabel
%81 = OpLoad %v4float %69
OpReturnValue %81
OpFunctionEnd
