OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_342391 = OpConstant %int 342391
%int_2000000000 = OpConstant %int 2000000000
%int_n2000000000 = OpConstant %int -2000000000
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_int Function
%54 = OpVariable %_ptr_Function_v4float Function
OpStore %i1 %int_1
OpStore %i2 %int_342391
OpStore %i3 %int_2000000000
OpStore %i4 %int_n2000000000
%37 = OpLoad %int %i1
%38 = OpIEqual %bool %37 %int_1
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %int %i2
%42 = OpIEqual %bool %41 %int_342391
OpBranch %40
%40 = OpLabel
%43 = OpPhi %bool %false %25 %42 %39
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%46 = OpLoad %int %i3
%47 = OpIEqual %bool %46 %int_2000000000
OpBranch %45
%45 = OpLabel
%48 = OpPhi %bool %false %40 %47 %44
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%51 = OpLoad %int %i4
%52 = OpIEqual %bool %51 %int_n2000000000
OpBranch %50
%50 = OpLabel
%53 = OpPhi %bool %false %45 %52 %49
OpSelectionMerge %58 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %59
OpStore %54 %62
OpBranch %58
%57 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%64 = OpLoad %v4float %63
OpStore %54 %64
OpBranch %58
%58 = OpLabel
%65 = OpLoad %v4float %54
OpReturnValue %65
OpFunctionEnd
