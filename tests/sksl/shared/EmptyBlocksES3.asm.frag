OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpName %counter "counter"
OpName %counter_0 "counter"
OpName %counter_1 "counter"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %color RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
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
%color = OpVariable %_ptr_Function_v4float Function
%counter = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%counter_1 = OpVariable %_ptr_Function_int Function
OpStore %color %28
OpStore %counter %int_0
OpBranch %33
%33 = OpLabel
OpLoopMerge %37 %36 None
OpBranch %34
%34 = OpLabel
%38 = OpLoad %int %counter
%40 = OpSLessThan %bool %38 %int_10
OpBranchConditional %40 %35 %37
%35 = OpLabel
OpBranch %36
%36 = OpLabel
%42 = OpLoad %int %counter
%43 = OpIAdd %int %42 %int_1
OpStore %counter %43
OpBranch %33
%37 = OpLabel
OpStore %counter_0 %int_0
OpBranch %45
%45 = OpLabel
OpLoopMerge %49 %48 None
OpBranch %46
%46 = OpLabel
%50 = OpLoad %int %counter_0
%51 = OpSLessThan %bool %50 %int_10
OpBranchConditional %51 %47 %49
%47 = OpLabel
OpBranch %48
%48 = OpLabel
%52 = OpLoad %int %counter_0
%53 = OpIAdd %int %52 %int_1
OpStore %counter_0 %53
OpBranch %45
%49 = OpLabel
OpStore %counter_1 %int_0
OpBranch %55
%55 = OpLabel
OpLoopMerge %59 %58 None
OpBranch %56
%56 = OpLabel
%60 = OpLoad %int %counter_1
%61 = OpSLessThan %bool %60 %int_10
OpBranchConditional %61 %57 %59
%57 = OpLabel
OpBranch %58
%58 = OpLabel
%62 = OpLoad %int %counter_1
%63 = OpIAdd %int %62 %int_1
OpStore %counter_1 %63
OpBranch %55
%59 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %64
%67 = OpCompositeExtract %float %66 0
%69 = OpFOrdEqual %bool %67 %float_1
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %72 %float_1
OpBranch %71
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeExtract %float %75 0
%78 = OpFOrdEqual %bool %76 %float_2
OpSelectionMerge %81 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
OpBranch %81
%80 = OpLabel
%82 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %82 %float_1
OpBranch %81
%81 = OpLabel
OpBranch %84
%84 = OpLabel
OpLoopMerge %88 %87 None
OpBranch %85
%85 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %91 %float_2
OpBranchConditional %92 %86 %88
%86 = OpLabel
OpBranch %87
%87 = OpLabel
OpBranch %84
%88 = OpLabel
OpBranch %93
%93 = OpLabel
OpLoopMerge %97 %96 None
OpBranch %94
%94 = OpLabel
OpBranch %95
%95 = OpLabel
OpBranch %96
%96 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 0
%101 = OpFOrdEqual %bool %100 %float_2
OpBranchConditional %101 %93 %97
%97 = OpLabel
%102 = OpLoad %v4float %color
OpReturnValue %102
OpFunctionEnd
