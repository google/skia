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
OpName %x "x"
OpName %r "r"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_n5 = OpConstant %float -5
%float_5 = OpConstant %float 5
%float_1 = OpConstant %float 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
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
%x = OpVariable %_ptr_Function_v4float Function
%r = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %x %32
OpStore %r %float_n5
OpBranch %36
%36 = OpLabel
OpLoopMerge %40 %39 None
OpBranch %37
%37 = OpLabel
%41 = OpLoad %float %r
%43 = OpFOrdLessThan %bool %41 %float_5
OpBranchConditional %43 %38 %40
%38 = OpLabel
%45 = OpLoad %float %r
%44 = OpExtInst %float %1 FClamp %45 %float_0 %float_1
%47 = OpAccessChain %_ptr_Function_float %x %int_0
OpStore %47 %44
%48 = OpLoad %v4float %x
%49 = OpCompositeExtract %float %48 0
%50 = OpFOrdEqual %bool %49 %float_0
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
OpBranch %40
%52 = OpLabel
OpBranch %39
%39 = OpLabel
%53 = OpLoad %float %r
%54 = OpFAdd %float %53 %float_1
OpStore %r %54
OpBranch %36
%40 = OpLabel
OpStore %b %float_5
OpBranch %56
%56 = OpLabel
OpLoopMerge %60 %59 None
OpBranch %57
%57 = OpLabel
%61 = OpLoad %float %b
%62 = OpFOrdGreaterThanEqual %bool %61 %float_0
OpBranchConditional %62 %58 %60
%58 = OpLabel
%63 = OpLoad %float %b
%64 = OpAccessChain %_ptr_Function_float %x %int_2
OpStore %64 %63
%66 = OpLoad %v4float %x
%67 = OpCompositeExtract %float %66 3
%68 = OpFOrdEqual %bool %67 %float_1
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
OpBranch %59
%70 = OpLabel
%71 = OpAccessChain %_ptr_Function_float %x %int_1
OpStore %71 %float_0
OpBranch %59
%59 = OpLabel
%73 = OpLoad %float %b
%74 = OpFSub %float %73 %float_1
OpStore %b %74
OpBranch %56
%60 = OpLabel
%75 = OpLoad %v4float %x
OpReturnValue %75
OpFunctionEnd
