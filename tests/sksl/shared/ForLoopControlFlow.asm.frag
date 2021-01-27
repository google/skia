OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %colorWhite "colorWhite"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %r "r"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %colorWhite RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
%_ptr_Private_v4float = OpTypePointer Private %v4float
%colorWhite = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%17 = OpTypeFunction %void
%20 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%24 = OpLoad %v4float %colorWhite
OpStore %x %24
OpStore %r %float_n5
OpBranch %28
%28 = OpLabel
OpLoopMerge %32 %31 None
OpBranch %29
%29 = OpLabel
%33 = OpLoad %float %r
%35 = OpFOrdLessThan %bool %33 %float_5
OpBranchConditional %35 %30 %32
%30 = OpLabel
%37 = OpLoad %float %r
%36 = OpExtInst %float %1 FClamp %37 %float_0 %float_1
%40 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %40 %36
%43 = OpLoad %v4float %x
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %44 %float_0
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
OpBranch %32
%47 = OpLabel
OpBranch %31
%31 = OpLabel
%48 = OpLoad %float %r
%49 = OpFAdd %float %48 %float_1
OpStore %r %49
OpBranch %28
%32 = OpLabel
OpStore %b %float_5
OpBranch %51
%51 = OpLabel
OpLoopMerge %55 %54 None
OpBranch %52
%52 = OpLabel
%56 = OpLoad %float %b
%57 = OpFOrdGreaterThanEqual %bool %56 %float_0
OpBranchConditional %57 %53 %55
%53 = OpLabel
%58 = OpLoad %float %b
%59 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %59 %58
%61 = OpLoad %v4float %x
%62 = OpCompositeExtract %float %61 3
%63 = OpFOrdEqual %bool %62 %float_1
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
OpBranch %54
%65 = OpLabel
%66 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %66 %float_0
OpBranch %54
%54 = OpLabel
%68 = OpLoad %float %b
%69 = OpFSub %float %68 %float_1
OpStore %b %69
OpBranch %51
%55 = OpLabel
%70 = OpLoad %v4float %x
OpReturnValue %70
OpFunctionEnd
