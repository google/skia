OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %tricky_h2hhh2h "tricky_h2hhh2h"
OpName %func_vh4 "func_vh4"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %t RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%25 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%39 = OpTypeFunction %void %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_5 = OpConstant %float 5
%60 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%65 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%68 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%tricky_h2hhh2h = OpFunction %v2float None %25
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpFunctionParameter %_ptr_Function_float
%29 = OpFunctionParameter %_ptr_Function_v2float
%30 = OpFunctionParameter %_ptr_Function_float
%31 = OpLabel
%32 = OpLoad %v2float %29
%33 = OpVectorShuffle %v2float %32 %32 1 0
OpStore %29 %33
%34 = OpLoad %float %27
%35 = OpLoad %float %28
%36 = OpFAdd %float %34 %35
%37 = OpLoad %float %30
%38 = OpCompositeConstruct %v2float %36 %37
OpReturnValue %38
OpFunctionEnd
%func_vh4 = OpFunction %void None %39
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpLabel
%t = OpVariable %_ptr_Function_v2float Function
%45 = OpVariable %_ptr_Function_float Function
%47 = OpVariable %_ptr_Function_float Function
%50 = OpVariable %_ptr_Function_v2float Function
%52 = OpVariable %_ptr_Function_float Function
OpStore %45 %float_1
OpStore %47 %float_2
%48 = OpLoad %v4float %41
%49 = OpVectorShuffle %v2float %48 %48 0 2
OpStore %50 %49
OpStore %52 %float_5
%53 = OpFunctionCall %v2float %tricky_h2hhh2h %45 %47 %50 %52
%54 = OpLoad %v2float %50
%55 = OpLoad %v4float %41
%56 = OpVectorShuffle %v4float %55 %54 4 1 5 3
OpStore %41 %56
OpStore %t %53
%57 = OpLoad %v2float %t
%58 = OpLoad %v4float %41
%59 = OpVectorShuffle %v4float %58 %57 0 4 2 5
OpStore %41 %59
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %60
%61 = OpFunctionParameter %_ptr_Function_v2float
%62 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%72 = OpVariable %_ptr_Function_v4float Function
OpStore %result %65
%66 = OpFunctionCall %void %func_vh4 %result
%67 = OpLoad %v4float %result
%69 = OpFOrdEqual %v4bool %67 %68
%71 = OpAll %bool %69
OpSelectionMerge %75 None
OpBranchConditional %71 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%80 = OpLoad %v4float %76
OpStore %72 %80
OpBranch %75
%74 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%83 = OpLoad %v4float %81
OpStore %72 %83
OpBranch %75
%75 = OpLabel
%84 = OpLoad %v4float %72
OpReturnValue %84
OpFunctionEnd
