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
OpName %foo "foo"
OpName %bar "bar"
OpName %y "y"
OpName %main "main"
OpName %x "x"
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
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
%_ptr_Function_v2float = OpTypePointer Function %v2float
%21 = OpTypeFunction %float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%30 = OpTypeFunction %void %_ptr_Function_float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%int_1 = OpConstant %int 1
%54 = OpTypeFunction %v4float
%float_10 = OpConstant %float 10
%float_200 = OpConstant %float 200
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%foo = OpFunction %float None %21
%23 = OpFunctionParameter %_ptr_Function_v2float
%24 = OpLabel
%25 = OpLoad %v2float %23
%26 = OpCompositeExtract %float %25 0
%27 = OpLoad %v2float %23
%28 = OpCompositeExtract %float %27 1
%29 = OpFMul %float %26 %28
OpReturnValue %29
OpFunctionEnd
%bar = OpFunction %void None %30
%32 = OpFunctionParameter %_ptr_Function_float
%33 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%52 = OpVariable %_ptr_Function_v2float Function
%39 = OpLoad %float %32
%41 = OpAccessChain %_ptr_Function_float %y %int_0
OpStore %41 %39
%42 = OpLoad %float %32
%44 = OpFMul %float %42 %float_2
%46 = OpAccessChain %_ptr_Function_float %y %int_1
OpStore %46 %44
%47 = OpAccessChain %_ptr_Function_float %y %int_0
%48 = OpLoad %float %47
%49 = OpAccessChain %_ptr_Function_float %y %int_1
%50 = OpLoad %float %49
%51 = OpCompositeConstruct %v2float %48 %50
OpStore %52 %51
%53 = OpFunctionCall %float %foo %52
OpStore %32 %53
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%59 = OpVariable %_ptr_Function_float Function
%65 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_10
%58 = OpLoad %float %x
OpStore %59 %58
%60 = OpFunctionCall %void %bar %59
%61 = OpLoad %float %59
OpStore %x %61
%62 = OpLoad %float %x
%64 = OpFOrdEqual %bool %62 %float_200
OpSelectionMerge %69 None
OpBranchConditional %64 %67 %68
%67 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%72 = OpLoad %v4float %70
OpStore %65 %72
OpBranch %69
%68 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%74 = OpLoad %v4float %73
OpStore %65 %74
OpBranch %69
%69 = OpLabel
%75 = OpLoad %v4float %65
OpReturnValue %75
OpFunctionEnd
