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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
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
OpDecorate %54 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%false = OpConstantFalse %bool
%int_2 = OpConstant %int 2
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%82 = OpVariable %_ptr_Function_v4float Function
%25 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%29 = OpLoad %v4float %25
OpStore %b %29
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%32 = OpLoad %v4float %30
OpStore %c %32
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%35 = OpLoad %v4float %33
OpStore %d %35
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%39 = OpLoad %v4float %37
OpStore %a %39
%40 = OpLoad %v4float %a
%41 = OpLoad %v4float %a
%42 = OpFMul %v4float %40 %41
OpStore %a %42
%43 = OpLoad %v4float %b
%44 = OpLoad %v4float %b
%45 = OpFMul %v4float %43 %44
OpStore %b %45
%46 = OpLoad %v4float %c
%47 = OpLoad %v4float %c
%48 = OpFMul %v4float %46 %47
OpStore %c %48
%49 = OpLoad %v4float %d
%50 = OpLoad %v4float %d
%51 = OpFMul %v4float %49 %50
OpStore %d %51
%52 = OpLoad %v4float %a
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%54 = OpLoad %v4float %53
%55 = OpFOrdEqual %v4bool %52 %54
%57 = OpAll %bool %55
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpLoad %v4float %b
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpFOrdEqual %v4bool %60 %62
%64 = OpAll %bool %63
OpBranch %59
%59 = OpLabel
%65 = OpPhi %bool %false %19 %64 %58
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %v4float %c
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%70 = OpLoad %v4float %69
%71 = OpFOrdEqual %v4bool %68 %70
%72 = OpAll %bool %71
OpBranch %67
%67 = OpLabel
%73 = OpPhi %bool %false %59 %72 %66
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4float %d
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%78 = OpLoad %v4float %77
%79 = OpFOrdEqual %v4bool %76 %78
%80 = OpAll %bool %79
OpBranch %75
%75 = OpLabel
%81 = OpPhi %bool %false %67 %80 %74
OpSelectionMerge %85 None
OpBranchConditional %81 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%87 = OpLoad %v4float %86
OpStore %82 %87
OpBranch %85
%84 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%89 = OpLoad %v4float %88
OpStore %82 %89
OpBranch %85
%85 = OpLabel
%90 = OpLoad %v4float %82
OpReturnValue %90
OpFunctionEnd
