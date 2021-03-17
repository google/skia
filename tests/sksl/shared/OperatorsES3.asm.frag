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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_1 = OpConstant %int 1
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_5 = OpConstant %int 5
%float_6 = OpConstant %float 6
%int_6 = OpConstant %int 6
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%96 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%29 = OpLoad %float %x
%30 = OpLoad %float %x
%31 = OpFSub %float %29 %30
%32 = OpLoad %float %y
%33 = OpLoad %float %x
%34 = OpFMul %float %32 %33
%35 = OpLoad %float %x
%36 = OpFMul %float %34 %35
%37 = OpLoad %float %y
%38 = OpLoad %float %x
%39 = OpFSub %float %37 %38
%40 = OpFMul %float %36 %39
%41 = OpFAdd %float %31 %40
OpStore %x %41
%42 = OpLoad %float %x
%43 = OpLoad %float %y
%44 = OpFDiv %float %42 %43
%45 = OpLoad %float %x
%46 = OpFDiv %float %44 %45
OpStore %y %46
%47 = OpLoad %int %z
%49 = OpSDiv %int %47 %int_2
%50 = OpSMod %int %49 %int_3
%52 = OpShiftLeftLogical %int %50 %int_4
%53 = OpShiftRightArithmetic %int %52 %int_2
%55 = OpShiftLeftLogical %int %53 %int_1
OpStore %z %55
%56 = OpLoad %float %x
%58 = OpFAdd %float %56 %float_12
OpStore %x %58
%59 = OpLoad %float %x
%60 = OpFSub %float %59 %float_12
OpStore %x %60
%61 = OpLoad %float %x
%62 = OpLoad %float %y
%64 = OpFDiv %float %62 %float_10
OpStore %y %64
%65 = OpFMul %float %61 %64
OpStore %x %65
%66 = OpLoad %int %z
%68 = OpBitwiseOr %int %66 %int_0
OpStore %z %68
%69 = OpLoad %int %z
%71 = OpBitwiseAnd %int %69 %int_n1
OpStore %z %71
%72 = OpLoad %int %z
%73 = OpBitwiseXor %int %72 %int_0
OpStore %z %73
%74 = OpLoad %int %z
%75 = OpShiftRightArithmetic %int %74 %int_2
OpStore %z %75
%76 = OpLoad %int %z
%77 = OpShiftLeftLogical %int %76 %int_4
OpStore %z %77
%78 = OpLoad %int %z
%80 = OpSMod %int %78 %int_5
OpStore %z %80
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%84 = OpLoad %float %x
%85 = OpFOrdEqual %bool %84 %float_6
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %float %y
%89 = OpFOrdEqual %bool %88 %float_6
OpBranch %87
%87 = OpLabel
%90 = OpPhi %bool %false %19 %89 %86
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %int %z
%94 = OpIEqual %bool %93 %int_6
OpBranch %92
%92 = OpLabel
%95 = OpPhi %bool %false %87 %94 %91
OpSelectionMerge %100 None
OpBranchConditional %95 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %101
OpStore %96 %103
OpBranch %100
%99 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
OpStore %96 %105
OpBranch %100
%100 = OpLabel
%106 = OpLoad %v4float %96
OpReturnValue %106
OpFunctionEnd
