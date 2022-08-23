OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "ah"
OpMemberName %_UniformBuffer 1 "bh"
OpMemberName %_UniformBuffer 2 "af"
OpMemberName %_UniformBuffer 3 "bf"
OpName %cross_length_2d_ff2f2 "cross_length_2d_ff2f2"
OpName %cross_length_2d_hh2h2 "cross_length_2d_hh2h2"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 16
OpMemberDecorate %_UniformBuffer 3 Offset 24
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
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
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v2float = OpTypePointer Function %v2float
%17 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%void = OpTypeVoid
%68 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%cross_length_2d_ff2f2 = OpFunction %float None %17
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpFunctionParameter %_ptr_Function_v2float
%20 = OpLabel
%22 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %26 None
OpBranchConditional %false %24 %25
%24 = OpLabel
%28 = OpLoad %v2float %18
%29 = OpLoad %v2float %19
%31 = OpCompositeConstruct %mat2v2float %28 %29
%27 = OpExtInst %float %1 Determinant %31
OpStore %22 %27
OpBranch %26
%25 = OpLabel
%32 = OpLoad %v2float %18
%33 = OpCompositeExtract %float %32 0
%34 = OpLoad %v2float %19
%35 = OpCompositeExtract %float %34 1
%36 = OpFMul %float %33 %35
%37 = OpLoad %v2float %18
%38 = OpCompositeExtract %float %37 1
%39 = OpLoad %v2float %19
%40 = OpCompositeExtract %float %39 0
%41 = OpFMul %float %38 %40
%42 = OpFSub %float %36 %41
OpStore %22 %42
OpBranch %26
%26 = OpLabel
%43 = OpLoad %float %22
OpReturnValue %43
OpFunctionEnd
%cross_length_2d_hh2h2 = OpFunction %float None %17
%44 = OpFunctionParameter %_ptr_Function_v2float
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpLabel
%47 = OpVariable %_ptr_Function_float Function
OpSelectionMerge %50 None
OpBranchConditional %false %48 %49
%48 = OpLabel
%52 = OpLoad %v2float %44
%53 = OpLoad %v2float %45
%54 = OpCompositeConstruct %mat2v2float %52 %53
%51 = OpExtInst %float %1 Determinant %54
OpStore %47 %51
OpBranch %50
%49 = OpLabel
%55 = OpLoad %v2float %44
%56 = OpCompositeExtract %float %55 0
%57 = OpLoad %v2float %45
%58 = OpCompositeExtract %float %57 1
%59 = OpFMul %float %56 %58
%60 = OpLoad %v2float %44
%61 = OpCompositeExtract %float %60 1
%62 = OpLoad %v2float %45
%63 = OpCompositeExtract %float %62 0
%64 = OpFMul %float %61 %63
%65 = OpFSub %float %59 %64
OpStore %47 %65
OpBranch %50
%50 = OpLabel
%66 = OpLoad %float %47
OpReturnValue %66
OpFunctionEnd
%main = OpFunction %void None %68
%69 = OpLabel
%75 = OpVariable %_ptr_Function_v2float Function
%79 = OpVariable %_ptr_Function_v2float Function
%86 = OpVariable %_ptr_Function_v2float Function
%90 = OpVariable %_ptr_Function_v2float Function
%70 = OpAccessChain %_ptr_Uniform_v2float %12 %int_0
%74 = OpLoad %v2float %70
OpStore %75 %74
%76 = OpAccessChain %_ptr_Uniform_v2float %12 %int_1
%78 = OpLoad %v2float %76
OpStore %79 %78
%80 = OpFunctionCall %float %cross_length_2d_hh2h2 %75 %79
%81 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %81 %80
%83 = OpAccessChain %_ptr_Uniform_v2float %12 %int_2
%85 = OpLoad %v2float %83
OpStore %86 %85
%87 = OpAccessChain %_ptr_Uniform_v2float %12 %int_3
%89 = OpLoad %v2float %87
OpStore %90 %89
%91 = OpFunctionCall %float %cross_length_2d_ff2f2 %86 %90
%92 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %92 %91
OpReturn
OpFunctionEnd
