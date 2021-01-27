### Compilation failed:

error: SPIR-V validation error: ID 123456[%123456] has not been defined
  %22 = OpLoad %v4float %123456

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorGreen"
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
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
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
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%73 = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %123456
OpStore %x %22
OpStore %r %float_n5
OpBranch %26
%26 = OpLabel
OpLoopMerge %30 %29 None
OpBranch %27
%27 = OpLabel
%31 = OpLoad %float %r
%33 = OpFOrdLessThan %bool %31 %float_5
OpBranchConditional %33 %28 %30
%28 = OpLabel
%35 = OpLoad %float %r
%34 = OpExtInst %float %1 FClamp %35 %float_0 %float_1
%38 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %38 %34
%41 = OpLoad %v4float %x
%42 = OpCompositeExtract %float %41 0
%43 = OpFOrdEqual %bool %42 %float_0
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
OpBranch %30
%45 = OpLabel
OpBranch %29
%29 = OpLabel
%46 = OpLoad %float %r
%47 = OpFAdd %float %46 %float_1
OpStore %r %47
OpBranch %26
%30 = OpLabel
OpStore %b %float_5
OpBranch %49
%49 = OpLabel
OpLoopMerge %53 %52 None
OpBranch %50
%50 = OpLabel
%54 = OpLoad %float %b
%55 = OpFOrdGreaterThanEqual %bool %54 %float_0
OpBranchConditional %55 %51 %53
%51 = OpLabel
%56 = OpLoad %float %b
%57 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %57 %56
%59 = OpLoad %v4float %x
%60 = OpCompositeExtract %float %59 3
%61 = OpFOrdEqual %bool %60 %float_1
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
OpBranch %52
%63 = OpLabel
%64 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %64 %float_0
OpBranch %52
%52 = OpLabel
%66 = OpLoad %float %b
%67 = OpFSub %float %66 %float_1
OpStore %b %67
OpBranch %49
%53 = OpLabel
%68 = OpLoad %v4float %x
%69 = OpLoad %v4float %123456
%70 = OpFOrdEqual %v4bool %68 %69
%72 = OpAll %bool %70
OpSelectionMerge %76 None
OpBranchConditional %72 %74 %75
%74 = OpLabel
%77 = OpLoad %v4float %x
OpStore %73 %77
OpBranch %76
%75 = OpLabel
%78 = OpLoad %v4float %123456
OpStore %73 %78
OpBranch %76
%76 = OpLabel
%79 = OpLoad %v4float %73
OpReturnValue %79
OpFunctionEnd

1 error
