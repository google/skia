OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %main "main"
OpName %i "i"
OpName %i_0 "i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_5 = OpConstant %float 5
%float_0_75 = OpConstant %float 0.75
%27 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%_ptr_Function_int = OpTypePointer Function %int
%int_10 = OpConstant %int 10
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
%float_0_25 = OpConstant %float 0.25
%int_2 = OpConstant %int 2
%int_100 = OpConstant %int 100
%main = OpFunction %void None %14
%15 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
%16 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%20 = OpLoad %float %16
%22 = OpFOrdGreaterThan %bool %20 %float_5
OpSelectionMerge %25 None
OpBranchConditional %22 %23 %24
%23 = OpLabel
OpStore %sk_FragColor %27
OpBranch %25
%24 = OpLabel
OpKill
%25 = OpLabel
OpStore %i %int_0
OpBranch %30
%30 = OpLabel
OpLoopMerge %34 %33 None
OpBranch %31
%31 = OpLabel
%35 = OpLoad %int %i
%37 = OpSLessThan %bool %35 %int_10
OpBranchConditional %37 %32 %34
%32 = OpLabel
%38 = OpLoad %v4float %sk_FragColor
%40 = OpVectorTimesScalar %v4float %38 %float_0_5
OpStore %sk_FragColor %40
%41 = OpLoad %int %i
%43 = OpIAdd %int %41 %int_1
OpStore %i %43
OpBranch %33
%33 = OpLabel
OpBranch %30
%34 = OpLabel
OpBranch %44
%44 = OpLabel
OpLoopMerge %48 %47 None
OpBranch %45
%45 = OpLabel
%49 = OpLoad %v4float %sk_FragColor
%51 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%52 = OpFAdd %v4float %49 %51
OpStore %sk_FragColor %52
OpBranch %46
%46 = OpLabel
OpBranch %47
%47 = OpLabel
%53 = OpLoad %v4float %sk_FragColor
%54 = OpCompositeExtract %float %53 0
%55 = OpFOrdLessThan %bool %54 %float_0_75
OpBranchConditional %55 %44 %48
%48 = OpLabel
OpStore %i_0 %int_0
OpBranch %57
%57 = OpLabel
OpLoopMerge %61 %60 None
OpBranch %58
%58 = OpLabel
%62 = OpLoad %int %i_0
%63 = OpSLessThan %bool %62 %int_10
OpBranchConditional %63 %59 %61
%59 = OpLabel
%64 = OpLoad %int %i_0
%66 = OpSMod %int %64 %int_2
%67 = OpIEqual %bool %66 %int_1
OpSelectionMerge %70 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
OpBranch %61
%69 = OpLabel
%71 = OpLoad %int %i_0
%73 = OpSGreaterThan %bool %71 %int_100
OpSelectionMerge %76 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
OpReturn
%75 = OpLabel
OpBranch %60
%76 = OpLabel
OpBranch %70
%70 = OpLabel
OpBranch %60
%60 = OpLabel
%77 = OpLoad %int %i_0
%78 = OpIAdd %int %77 %int_1
OpStore %i_0 %78
OpBranch %57
%61 = OpLabel
OpReturn
OpFunctionEnd
