OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %xy "xy"
OpName %zw "zw"
OpName %tolerance "tolerance"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_0_015625 = OpConstant %float 0.015625
%43 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
%false = OpConstantFalse %bool
%float_n1 = OpConstant %float -1
%50 = OpConstantComposite %v2float %float_n1 %float_0
%v2bool = OpTypeVector %bool 2
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%61 = OpConstantComposite %v2float %float_0_75 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
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
%xy = OpVariable %_ptr_Function_uint Function
%zw = OpVariable %_ptr_Function_uint Function
%tolerance = OpVariable %_ptr_Function_v2float Function
%64 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%34 = OpLoad %v4float %30
%35 = OpVectorShuffle %v2float %34 %34 0 1
%29 = OpExtInst %uint %1 PackSnorm2x16 %35
OpStore %xy %29
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%39 = OpLoad %v4float %38
%40 = OpVectorShuffle %v2float %39 %39 2 3
%37 = OpExtInst %uint %1 PackSnorm2x16 %40
OpStore %zw %37
OpStore %tolerance %43
%48 = OpExtInst %v2float %1 UnpackSnorm2x16 %29
%51 = OpFSub %v2float %48 %50
%47 = OpExtInst %v2float %1 FAbs %51
%46 = OpFOrdLessThan %v2bool %47 %43
%45 = OpAll %bool %46
OpSelectionMerge %54 None
OpBranchConditional %45 %53 %54
%53 = OpLabel
%58 = OpExtInst %v2float %1 UnpackSnorm2x16 %37
%62 = OpFSub %v2float %58 %61
%57 = OpExtInst %v2float %1 FAbs %62
%56 = OpFOrdLessThan %v2bool %57 %43
%55 = OpAll %bool %56
OpBranch %54
%54 = OpLabel
%63 = OpPhi %bool %false %25 %55 %53
OpSelectionMerge %68 None
OpBranchConditional %63 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%71 = OpLoad %v4float %69
OpStore %64 %71
OpBranch %68
%67 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%74 = OpLoad %v4float %72
OpStore %64 %74
OpBranch %68
%68 = OpLabel
%75 = OpLoad %v4float %64
OpReturnValue %75
OpFunctionEnd
