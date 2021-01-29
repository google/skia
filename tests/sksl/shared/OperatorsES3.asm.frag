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
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
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
%int_8 = OpConstant %int 8
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v2float = OpTypeVector %float 2
%int_6 = OpConstant %int 6
%float_0 = OpConstant %float 0
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%98 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_8
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
%51 = OpLoad %int %z
%53 = OpBitwiseOr %int %51 %int_0
OpStore %z %53
%54 = OpLoad %int %z
%56 = OpBitwiseAnd %int %54 %int_n1
OpStore %z %56
%57 = OpLoad %int %z
%58 = OpBitwiseXor %int %57 %int_0
OpStore %z %58
%59 = OpLoad %int %z
%61 = OpShiftRightArithmetic %int %59 %int_2
OpStore %z %61
%62 = OpLoad %int %z
%64 = OpShiftLeftLogical %int %62 %int_4
OpStore %z %64
%65 = OpLoad %int %z
%67 = OpSMod %int %65 %int_5
OpStore %z %67
%68 = OpExtInst %float %1 Sqrt %float_1
%69 = OpCompositeConstruct %v2float %68 %68
%72 = OpConvertSToF %float %int_6
OpStore %x %72
%73 = OpLoad %bool %c
%74 = OpSelect %float %73 %float_1 %float_0
%76 = OpLoad %bool %d
%77 = OpSelect %float %76 %float_1 %float_0
%78 = OpFMul %float %74 %77
%79 = OpLoad %bool %e
%80 = OpSelect %float %79 %float_1 %float_0
%81 = OpFMul %float %78 %80
OpStore %y %float_6
%83 = OpExtInst %float %1 Sqrt %float_1
%84 = OpCompositeConstruct %v2float %83 %83
OpStore %z %int_6
%86 = OpLoad %float %x
%87 = OpFOrdEqual %bool %86 %float_6
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %float %y
%91 = OpFOrdEqual %bool %90 %float_6
OpBranch %89
%89 = OpLabel
%92 = OpPhi %bool %false %19 %91 %88
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %int %z
%96 = OpIEqual %bool %95 %int_6
OpBranch %94
%94 = OpLabel
%97 = OpPhi %bool %false %89 %96 %93
OpSelectionMerge %102 None
OpBranchConditional %97 %100 %101
%100 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%105 = OpLoad %v4float %103
OpStore %98 %105
OpBranch %102
%101 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%108 = OpLoad %v4float %106
OpStore %98 %108
OpBranch %102
%102 = OpLabel
%109 = OpLoad %v4float %98
OpReturnValue %109
OpFunctionEnd
