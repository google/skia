OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpMemberName %_UniformBuffer 3 "colorBlack"
OpName %_entrypoint_v "_entrypoint_v"
OpName %setToColorBlack_vh4 "setToColorBlack_vh4"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %d RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%24 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%33 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%setToColorBlack_vh4 = OpFunction %void None %24
%26 = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%32 = OpLoad %v4float %28
OpStore %26 %32
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %33
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%46 = OpVariable %_ptr_Function_v4float Function
%95 = OpVariable %_ptr_Function_v4float Function
%40 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%42 = OpLoad %v4float %40
OpStore %b %42
%43 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%45 = OpLoad %v4float %43
OpStore %c %45
%47 = OpFunctionCall %void %setToColorBlack_vh4 %46
%48 = OpLoad %v4float %46
OpStore %d %48
%49 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%51 = OpLoad %v4float %49
OpStore %a %51
%52 = OpLoad %v4float %a
%53 = OpLoad %v4float %a
%54 = OpFMul %v4float %52 %53
OpStore %a %54
%55 = OpLoad %v4float %b
%56 = OpLoad %v4float %b
%57 = OpFMul %v4float %55 %56
OpStore %b %57
%58 = OpLoad %v4float %c
%59 = OpLoad %v4float %c
%60 = OpFMul %v4float %58 %59
OpStore %c %60
%61 = OpLoad %v4float %d
%62 = OpLoad %v4float %d
%63 = OpFMul %v4float %61 %62
OpStore %d %63
%65 = OpLoad %v4float %a
%66 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%67 = OpLoad %v4float %66
%68 = OpFOrdEqual %v4bool %65 %67
%70 = OpAll %bool %68
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %b
%74 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%75 = OpLoad %v4float %74
%76 = OpFOrdEqual %v4bool %73 %75
%77 = OpAll %bool %76
OpBranch %72
%72 = OpLabel
%78 = OpPhi %bool %false %35 %77 %71
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4float %c
%82 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%83 = OpLoad %v4float %82
%84 = OpFOrdEqual %v4bool %81 %83
%85 = OpAll %bool %84
OpBranch %80
%80 = OpLabel
%86 = OpPhi %bool %false %72 %85 %79
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %v4float %d
%90 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%91 = OpLoad %v4float %90
%92 = OpFOrdEqual %v4bool %89 %91
%93 = OpAll %bool %92
OpBranch %88
%88 = OpLabel
%94 = OpPhi %bool %false %80 %93 %87
OpSelectionMerge %98 None
OpBranchConditional %94 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%100 = OpLoad %v4float %99
OpStore %95 %100
OpBranch %98
%97 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%102 = OpLoad %v4float %101
OpStore %95 %102
OpBranch %98
%98 = OpLabel
%103 = OpLoad %v4float %95
OpReturnValue %103
OpFunctionEnd
