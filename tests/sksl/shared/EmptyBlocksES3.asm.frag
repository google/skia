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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
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
OpStore %color %28
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 0
%36 = OpFOrdEqual %bool %34 %float_1
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%39 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %39 %float_1
OpBranch %38
%38 = OpLabel
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%43 = OpLoad %v4float %42
%44 = OpCompositeExtract %float %43 0
%46 = OpFOrdEqual %bool %44 %float_2
OpSelectionMerge %49 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
OpBranch %49
%48 = OpLabel
%50 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %50 %float_1
OpBranch %49
%49 = OpLabel
OpBranch %52
%52 = OpLabel
OpLoopMerge %56 %55 None
OpBranch %53
%53 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 0
%60 = OpFOrdEqual %bool %59 %float_2
OpBranchConditional %60 %54 %56
%54 = OpLabel
OpBranch %55
%55 = OpLabel
OpBranch %52
%56 = OpLabel
OpBranch %61
%61 = OpLabel
OpLoopMerge %65 %64 None
OpBranch %62
%62 = OpLabel
OpBranch %63
%63 = OpLabel
OpBranch %64
%64 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpFOrdEqual %bool %68 %float_2
OpBranchConditional %69 %61 %65
%65 = OpLabel
%70 = OpLoad %v4float %color
OpReturnValue %70
OpFunctionEnd
