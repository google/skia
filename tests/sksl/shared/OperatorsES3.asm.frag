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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%w = OpVariable %_ptr_Function_v2int Function
%94 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpFSub %float %float_1 %float_1
%36 = OpFMul %float %float_2 %float_1
%37 = OpFMul %float %36 %float_1
%38 = OpFSub %float %float_2 %float_1
%39 = OpFMul %float %37 %38
%40 = OpFAdd %float %35 %39
OpStore %x %40
%41 = OpFDiv %float %40 %float_2
%42 = OpFDiv %float %41 %40
OpStore %y %42
%44 = OpSDiv %int %int_3 %int_2
%45 = OpSMod %int %44 %int_3
%47 = OpShiftLeftLogical %int %45 %int_4
%48 = OpShiftRightArithmetic %int %47 %int_2
%50 = OpShiftLeftLogical %int %48 %int_1
OpStore %z %50
%52 = OpFAdd %float %40 %float_12
OpStore %x %52
%53 = OpFSub %float %52 %float_12
OpStore %x %53
%55 = OpFDiv %float %42 %float_10
OpStore %y %55
%56 = OpFMul %float %53 %55
OpStore %x %56
%58 = OpBitwiseOr %int %50 %int_0
OpStore %z %58
%60 = OpBitwiseAnd %int %58 %int_n1
OpStore %z %60
%61 = OpBitwiseXor %int %60 %int_0
OpStore %z %61
%62 = OpShiftRightArithmetic %int %61 %int_2
OpStore %z %62
%63 = OpShiftLeftLogical %int %62 %int_4
OpStore %z %63
%65 = OpSMod %int %63 %int_5
OpStore %z %65
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%71 = OpNot %int %int_5
%72 = OpCompositeConstruct %v2int %71 %71
OpStore %w %72
%73 = OpNot %v2int %72
OpStore %w %73
%75 = OpCompositeExtract %int %73 0
%76 = OpIEqual %bool %75 %int_5
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpCompositeExtract %int %73 1
%80 = OpIEqual %bool %79 %int_5
OpBranch %78
%78 = OpLabel
%81 = OpPhi %bool %false %25 %80 %77
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpFOrdEqual %bool %float_6 %float_6
OpBranch %83
%83 = OpLabel
%85 = OpPhi %bool %false %78 %84 %82
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpFOrdEqual %bool %float_6 %float_6
OpBranch %87
%87 = OpLabel
%89 = OpPhi %bool %false %83 %88 %86
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpIEqual %bool %int_6 %int_6
OpBranch %91
%91 = OpLabel
%93 = OpPhi %bool %false %87 %92 %90
OpSelectionMerge %98 None
OpBranchConditional %93 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%101 = OpLoad %v4float %99
OpStore %94 %101
OpBranch %98
%97 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%103 = OpLoad %v4float %102
OpStore %94 %103
OpBranch %98
%98 = OpLabel
%104 = OpLoad %v4float %94
OpReturnValue %104
OpFunctionEnd
