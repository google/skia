### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %49 = OpFunctionCall %v2float %tricky %44 %46 %4294967295 %48

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
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
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
%53 = OpTypeFunction %v4float
%float_0 = OpConstant %float 0
%float_3 = OpConstant %float 3
%58 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%61 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
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
%48 = OpVariable %_ptr_Function_float Function
OpStore %44 %float_1
OpStore %46 %float_2
OpStore %48 %float_5
%49 = OpFunctionCall %v2float %tricky %44 %46 %4294967295 %48
OpStore %t %49
%50 = OpLoad %v2float %t
%51 = OpLoad %v4float %40
%52 = OpVectorShuffle %v4float %51 %50 0 4 2 5
OpStore %40 %52
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %53
%54 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_v4float Function
OpStore %result %58
%59 = OpFunctionCall %void %func %result
%60 = OpLoad %v4float %result
%62 = OpFOrdEqual %v4bool %60 %61
%64 = OpAll %bool %62
OpSelectionMerge %68 None
OpBranchConditional %64 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%73 = OpLoad %v4float %69
OpStore %65 %73
OpBranch %68
%67 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%76 = OpLoad %v4float %74
OpStore %65 %76
OpBranch %68
%68 = OpLabel
%77 = OpLoad %v4float %65
OpReturnValue %77
OpFunctionEnd

1 error
