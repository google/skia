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
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_bool = OpTypePointer Function %bool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%97 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpCompositeExtract %float %32 1
OpStore %f %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %36
%38 = OpCompositeExtract %float %37 1
%39 = OpConvertFToS %int %38
OpStore %i %39
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%43 = OpLoad %v4float %42
%44 = OpCompositeExtract %float %43 1
%45 = OpFUnordNotEqual %bool %44 %float_0
OpStore %b %45
%47 = OpLoad %float %f
OpStore %f1 %47
%49 = OpLoad %int %i
%50 = OpConvertSToF %float %49
OpStore %f2 %50
%52 = OpLoad %bool %b
%53 = OpSelect %float %52 %float_1 %float_0
OpStore %f3 %53
%56 = OpLoad %float %f
%57 = OpConvertFToS %int %56
OpStore %i1 %57
%59 = OpLoad %int %i
OpStore %i2 %59
%61 = OpLoad %bool %b
%62 = OpSelect %int %61 %int_1 %int_0
OpStore %i3 %62
%65 = OpLoad %float %f
%66 = OpFUnordNotEqual %bool %65 %float_0
OpStore %b1 %66
%68 = OpLoad %int %i
%69 = OpINotEqual %bool %68 %int_0
OpStore %b2 %69
%71 = OpLoad %bool %b
OpStore %b3 %71
%72 = OpLoad %float %f1
%73 = OpLoad %float %f2
%74 = OpFAdd %float %72 %73
%75 = OpLoad %float %f3
%76 = OpFAdd %float %74 %75
%77 = OpLoad %int %i1
%78 = OpConvertSToF %float %77
%79 = OpFAdd %float %76 %78
%80 = OpLoad %int %i2
%81 = OpConvertSToF %float %80
%82 = OpFAdd %float %79 %81
%83 = OpLoad %int %i3
%84 = OpConvertSToF %float %83
%85 = OpFAdd %float %82 %84
%86 = OpLoad %bool %b1
%87 = OpSelect %float %86 %float_1 %float_0
%88 = OpFAdd %float %85 %87
%89 = OpLoad %bool %b2
%90 = OpSelect %float %89 %float_1 %float_0
%91 = OpFAdd %float %88 %90
%92 = OpLoad %bool %b3
%93 = OpSelect %float %92 %float_1 %float_0
%94 = OpFAdd %float %91 %93
%96 = OpFOrdEqual %bool %94 %float_9
OpSelectionMerge %101 None
OpBranchConditional %96 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %102
OpStore %97 %103
OpBranch %101
%100 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
OpStore %97 %105
OpBranch %101
%101 = OpLabel
%106 = OpLoad %v4float %97
OpReturnValue %106
OpFunctionEnd
