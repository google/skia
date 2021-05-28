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
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%102 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpLoad %float %x
%36 = OpLoad %float %x
%37 = OpFSub %float %35 %36
%38 = OpLoad %float %y
%39 = OpLoad %float %x
%40 = OpFMul %float %38 %39
%41 = OpLoad %float %x
%42 = OpFMul %float %40 %41
%43 = OpLoad %float %y
%44 = OpLoad %float %x
%45 = OpFSub %float %43 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %37 %46
OpStore %x %47
%48 = OpLoad %float %x
%49 = OpLoad %float %y
%50 = OpFDiv %float %48 %49
%51 = OpLoad %float %x
%52 = OpFDiv %float %50 %51
OpStore %y %52
%53 = OpLoad %int %z
%55 = OpSDiv %int %53 %int_2
%56 = OpSMod %int %55 %int_3
%58 = OpShiftLeftLogical %int %56 %int_4
%59 = OpShiftRightArithmetic %int %58 %int_2
%61 = OpShiftLeftLogical %int %59 %int_1
OpStore %z %61
%62 = OpLoad %float %x
%64 = OpFAdd %float %62 %float_12
OpStore %x %64
%65 = OpLoad %float %x
%66 = OpFSub %float %65 %float_12
OpStore %x %66
%67 = OpLoad %float %x
%68 = OpLoad %float %y
%70 = OpFDiv %float %68 %float_10
OpStore %y %70
%71 = OpFMul %float %67 %70
OpStore %x %71
%72 = OpLoad %int %z
%74 = OpBitwiseOr %int %72 %int_0
OpStore %z %74
%75 = OpLoad %int %z
%77 = OpBitwiseAnd %int %75 %int_n1
OpStore %z %77
%78 = OpLoad %int %z
%79 = OpBitwiseXor %int %78 %int_0
OpStore %z %79
%80 = OpLoad %int %z
%81 = OpShiftRightArithmetic %int %80 %int_2
OpStore %z %81
%82 = OpLoad %int %z
%83 = OpShiftLeftLogical %int %82 %int_4
OpStore %z %83
%84 = OpLoad %int %z
%86 = OpSMod %int %84 %int_5
OpStore %z %86
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%90 = OpLoad %float %x
%91 = OpFOrdEqual %bool %90 %float_6
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %float %y
%95 = OpFOrdEqual %bool %94 %float_6
OpBranch %93
%93 = OpLabel
%96 = OpPhi %bool %false %25 %95 %92
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %int %z
%100 = OpIEqual %bool %99 %int_6
OpBranch %98
%98 = OpLabel
%101 = OpPhi %bool %false %93 %100 %97
OpSelectionMerge %106 None
OpBranchConditional %101 %104 %105
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %107
OpStore %102 %109
OpBranch %106
%105 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%111 = OpLoad %v4float %110
OpStore %102 %111
OpBranch %106
%106 = OpLabel
%112 = OpLoad %v4float %102
OpReturnValue %112
OpFunctionEnd
