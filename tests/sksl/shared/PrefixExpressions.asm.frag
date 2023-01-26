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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpName %val "val"
OpName %mask "mask"
OpName %imask "imask"
OpName %one "one"
OpName %m "m"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %one RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %m RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%uint_0 = OpConstant %uint 0
%74 = OpConstantComposite %v2uint %uint_0 %uint_0
%v2bool = OpTypeVector %bool 2
%_ptr_Function_float = OpTypePointer Function %float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%ok = OpVariable %_ptr_Function_bool Function
%val = OpVariable %_ptr_Function_uint Function
%mask = OpVariable %_ptr_Function_v2uint Function
%imask = OpVariable %_ptr_Function_v2int Function
%one = OpVariable %_ptr_Function_float Function
%m = OpVariable %_ptr_Function_mat4v4float Function
%92 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpSelectionMerge %31 None
OpBranchConditional %true %30 %31
%30 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%40 = OpFOrdEqual %bool %38 %float_1
%32 = OpLogicalNot %bool %40
OpBranch %31
%31 = OpLabel
%41 = OpPhi %bool %false %25 %32 %30
OpStore %ok %41
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 0
%48 = OpConvertFToU %uint %47
OpStore %val %48
%52 = OpNot %uint %48
%53 = OpCompositeConstruct %v2uint %48 %52
OpStore %mask %53
%57 = OpNot %v2uint %53
%58 = OpCompositeExtract %uint %57 0
%59 = OpBitcast %int %58
%60 = OpCompositeExtract %uint %57 1
%61 = OpBitcast %int %60
%62 = OpCompositeConstruct %v2int %59 %61
OpStore %imask %62
%63 = OpNot %v2uint %53
%64 = OpNot %v2int %62
%65 = OpCompositeExtract %int %64 0
%66 = OpBitcast %uint %65
%67 = OpCompositeExtract %int %64 1
%68 = OpBitcast %uint %67
%69 = OpCompositeConstruct %v2uint %66 %68
%70 = OpBitwiseAnd %v2uint %63 %69
OpStore %mask %70
OpSelectionMerge %72 None
OpBranchConditional %41 %71 %72
%71 = OpLabel
%75 = OpIEqual %v2bool %70 %74
%77 = OpAll %bool %75
OpBranch %72
%72 = OpLabel
%78 = OpPhi %bool %false %31 %77 %71
OpStore %ok %78
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %81
%83 = OpCompositeExtract %float %82 0
OpStore %one %83
%87 = OpCompositeConstruct %v4float %83 %float_0 %float_0 %float_0
%88 = OpCompositeConstruct %v4float %float_0 %83 %float_0 %float_0
%89 = OpCompositeConstruct %v4float %float_0 %float_0 %83 %float_0
%90 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %83
%91 = OpCompositeConstruct %mat4v4float %87 %88 %89 %90
OpStore %m %91
OpSelectionMerge %96 None
OpBranchConditional %78 %94 %95
%94 = OpLabel
%97 = OpFNegate %v4float %87
%98 = OpFNegate %v4float %88
%99 = OpFNegate %v4float %89
%100 = OpFNegate %v4float %90
%101 = OpCompositeConstruct %mat4v4float %97 %98 %99 %100
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %102
%104 = OpFNegate %v4float %103
%105 = OpMatrixTimesVector %v4float %101 %104
OpStore %92 %105
OpBranch %96
%95 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%108 = OpLoad %v4float %106
OpStore %92 %108
OpBranch %96
%96 = OpLabel
%109 = OpLoad %v4float %92
OpReturnValue %109
OpFunctionEnd
