OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpMemberName %_UniformBuffer 3 "colorBlack"
OpName %_entrypoint "_entrypoint"
OpName %setToColorBlack "setToColorBlack"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %27 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%19 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%28 = OpTypeFunction %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%setToColorBlack = OpFunction %void None %19
%21 = OpFunctionParameter %_ptr_Function_v4float
%22 = OpLabel
%23 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%27 = OpLoad %v4float %23
OpStore %21 %27
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %28
%29 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%41 = OpVariable %_ptr_Function_v4float Function
%90 = OpVariable %_ptr_Function_v4float Function
%34 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %34
OpStore %b %36
%37 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%39 = OpLoad %v4float %37
OpStore %c %39
%40 = OpLoad %v4float %d
OpStore %41 %40
%42 = OpFunctionCall %void %setToColorBlack %41
%43 = OpLoad %v4float %41
OpStore %d %43
%44 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%46 = OpLoad %v4float %44
OpStore %a %46
%47 = OpLoad %v4float %a
%48 = OpLoad %v4float %a
%49 = OpFMul %v4float %47 %48
OpStore %a %49
%50 = OpLoad %v4float %b
%51 = OpLoad %v4float %b
%52 = OpFMul %v4float %50 %51
OpStore %b %52
%53 = OpLoad %v4float %c
%54 = OpLoad %v4float %c
%55 = OpFMul %v4float %53 %54
OpStore %c %55
%56 = OpLoad %v4float %d
%57 = OpLoad %v4float %d
%58 = OpFMul %v4float %56 %57
OpStore %d %58
%60 = OpLoad %v4float %a
%61 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%62 = OpLoad %v4float %61
%63 = OpFOrdEqual %v4bool %60 %62
%65 = OpAll %bool %63
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %v4float %b
%69 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%70 = OpLoad %v4float %69
%71 = OpFOrdEqual %v4bool %68 %70
%72 = OpAll %bool %71
OpBranch %67
%67 = OpLabel
%73 = OpPhi %bool %false %29 %72 %66
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4float %c
%77 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%78 = OpLoad %v4float %77
%79 = OpFOrdEqual %v4bool %76 %78
%80 = OpAll %bool %79
OpBranch %75
%75 = OpLabel
%81 = OpPhi %bool %false %67 %80 %74
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %v4float %d
%85 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%86 = OpLoad %v4float %85
%87 = OpFOrdEqual %v4bool %84 %86
%88 = OpAll %bool %87
OpBranch %83
%83 = OpLabel
%89 = OpPhi %bool %false %75 %88 %82
OpSelectionMerge %93 None
OpBranchConditional %89 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%95 = OpLoad %v4float %94
OpStore %90 %95
OpBranch %93
%92 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%97 = OpLoad %v4float %96
OpStore %90 %97
OpBranch %93
%93 = OpLabel
%98 = OpLoad %v4float %90
OpReturnValue %98
OpFunctionEnd
