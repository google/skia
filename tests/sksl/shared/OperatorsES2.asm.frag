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
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %c "c"
OpName %d "d"
OpName %e "e"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%int_5 = OpConstant %int 5
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_6 = OpConstant %int 6
%float_0 = OpConstant %float 0
%float_6 = OpConstant %float 6
%int_4 = OpConstant %int 4
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%78 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_5
%33 = OpExtInst %float %1 Sqrt %float_2
%34 = OpFOrdGreaterThan %bool %33 %float_2
OpStore %c %34
%37 = OpLoad %bool %c
%38 = OpLogicalNotEqual %bool %true %37
OpStore %d %38
%40 = OpLoad %bool %c
OpStore %e %40
%41 = OpLoad %float %x
%43 = OpFAdd %float %41 %float_12
OpStore %x %43
%44 = OpLoad %float %x
%45 = OpFSub %float %44 %float_12
OpStore %x %45
%46 = OpLoad %float %x
%47 = OpLoad %float %y
%49 = OpFDiv %float %47 %float_10
OpStore %y %49
%50 = OpFMul %float %46 %49
OpStore %x %50
%51 = OpExtInst %float %1 Sqrt %float_1
%53 = OpConvertSToF %float %int_6
OpStore %x %53
%54 = OpLoad %bool %c
%55 = OpSelect %float %54 %float_1 %float_0
%57 = OpLoad %bool %d
%58 = OpSelect %float %57 %float_1 %float_0
%59 = OpFMul %float %55 %58
%60 = OpLoad %bool %e
%61 = OpSelect %float %60 %float_1 %float_0
%62 = OpFMul %float %59 %61
OpStore %y %float_6
OpStore %z %int_6
%66 = OpLoad %float %x
%67 = OpFOrdEqual %bool %66 %float_6
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %float %y
%71 = OpFOrdEqual %bool %70 %float_6
OpBranch %69
%69 = OpLabel
%72 = OpPhi %bool %false %19 %71 %68
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %int %z
%76 = OpIEqual %bool %75 %int_6
OpBranch %74
%74 = OpLabel
%77 = OpPhi %bool %false %69 %76 %73
OpSelectionMerge %82 None
OpBranchConditional %77 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%86 = OpLoad %v4float %83
OpStore %78 %86
OpBranch %82
%81 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%89 = OpLoad %v4float %87
OpStore %78 %89
OpBranch %82
%82 = OpLabel
%90 = OpLoad %v4float %78
OpReturnValue %90
OpFunctionEnd
