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
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
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
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
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
%87 = OpVariable %_ptr_Function_v4float Function
%34 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %34
OpStore %b %36
%37 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%39 = OpLoad %v4float %37
OpStore %c %39
%40 = OpFunctionCall %void %setToColorBlack %d
%41 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%43 = OpLoad %v4float %41
OpStore %a %43
%44 = OpLoad %v4float %a
%45 = OpLoad %v4float %a
%46 = OpFMul %v4float %44 %45
OpStore %a %46
%47 = OpLoad %v4float %b
%48 = OpLoad %v4float %b
%49 = OpFMul %v4float %47 %48
OpStore %b %49
%50 = OpLoad %v4float %c
%51 = OpLoad %v4float %c
%52 = OpFMul %v4float %50 %51
OpStore %c %52
%53 = OpLoad %v4float %d
%54 = OpLoad %v4float %d
%55 = OpFMul %v4float %53 %54
OpStore %d %55
%57 = OpLoad %v4float %a
%58 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%59 = OpLoad %v4float %58
%60 = OpFOrdEqual %v4bool %57 %59
%62 = OpAll %bool %60
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%65 = OpLoad %v4float %b
%66 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%67 = OpLoad %v4float %66
%68 = OpFOrdEqual %v4bool %65 %67
%69 = OpAll %bool %68
OpBranch %64
%64 = OpLabel
%70 = OpPhi %bool %false %29 %69 %63
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %c
%74 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%75 = OpLoad %v4float %74
%76 = OpFOrdEqual %v4bool %73 %75
%77 = OpAll %bool %76
OpBranch %72
%72 = OpLabel
%78 = OpPhi %bool %false %64 %77 %71
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4float %d
%82 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%83 = OpLoad %v4float %82
%84 = OpFOrdEqual %v4bool %81 %83
%85 = OpAll %bool %84
OpBranch %80
%80 = OpLabel
%86 = OpPhi %bool %false %72 %85 %79
OpSelectionMerge %90 None
OpBranchConditional %86 %88 %89
%88 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%92 = OpLoad %v4float %91
OpStore %87 %92
OpBranch %90
%89 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%94 = OpLoad %v4float %93
OpStore %87 %94
OpBranch %90
%90 = OpLabel
%95 = OpLoad %v4float %87
OpReturnValue %95
OpFunctionEnd
