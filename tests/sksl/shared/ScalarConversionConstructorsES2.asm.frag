OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %f "f"
OpName %i "i"
OpName %b "b"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_bool = OpTypePointer Function %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
%92 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 1
OpStore %f %27
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%31 = OpLoad %v4float %30
%32 = OpCompositeExtract %float %31 1
%33 = OpConvertFToS %int %32
OpStore %i %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %36
%38 = OpCompositeExtract %float %37 1
%39 = OpFUnordNotEqual %bool %38 %float_0
OpStore %b %39
%42 = OpLoad %float %f
OpStore %f1 %42
%44 = OpLoad %int %i
%45 = OpConvertSToF %float %44
OpStore %f2 %45
%47 = OpLoad %bool %b
%48 = OpSelect %float %47 %float_1 %float_0
OpStore %f3 %48
%51 = OpLoad %float %f
%52 = OpConvertFToS %int %51
OpStore %i1 %52
%54 = OpLoad %int %i
OpStore %i2 %54
%56 = OpLoad %bool %b
%57 = OpSelect %int %56 %int_1 %int_0
OpStore %i3 %57
%60 = OpLoad %float %f
%61 = OpFUnordNotEqual %bool %60 %float_0
OpStore %b1 %61
%63 = OpLoad %int %i
%64 = OpINotEqual %bool %63 %int_0
OpStore %b2 %64
%66 = OpLoad %bool %b
OpStore %b3 %66
%67 = OpLoad %float %f1
%68 = OpLoad %float %f2
%69 = OpFAdd %float %67 %68
%70 = OpLoad %float %f3
%71 = OpFAdd %float %69 %70
%72 = OpLoad %int %i1
%73 = OpConvertSToF %float %72
%74 = OpFAdd %float %71 %73
%75 = OpLoad %int %i2
%76 = OpConvertSToF %float %75
%77 = OpFAdd %float %74 %76
%78 = OpLoad %int %i3
%79 = OpConvertSToF %float %78
%80 = OpFAdd %float %77 %79
%81 = OpLoad %bool %b1
%82 = OpSelect %float %81 %float_1 %float_0
%83 = OpFAdd %float %80 %82
%84 = OpLoad %bool %b2
%85 = OpSelect %float %84 %float_1 %float_0
%86 = OpFAdd %float %83 %85
%87 = OpLoad %bool %b3
%88 = OpSelect %float %87 %float_1 %float_0
%89 = OpFAdd %float %86 %88
%91 = OpFOrdEqual %bool %89 %float_9
OpSelectionMerge %96 None
OpBranchConditional %91 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%98 = OpLoad %v4float %97
OpStore %92 %98
OpBranch %96
%95 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%100 = OpLoad %v4float %99
OpStore %92 %100
OpBranch %96
%96 = OpLabel
%101 = OpLoad %v4float %92
OpReturnValue %101
OpFunctionEnd
