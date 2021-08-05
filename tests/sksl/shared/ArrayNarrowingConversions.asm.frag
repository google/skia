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
OpName %i2 "i2"
OpName %s2 "s2"
OpName %f2 "f2"
OpName %h2 "h2"
OpName %cf2 "cf2"
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
OpDecorate %_arr_int_int_2 ArrayStride 16
OpDecorate %s2 RelaxedPrecision
OpDecorate %_arr_int_int_2_0 ArrayStride 16
OpDecorate %36 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %_arr_float_int_2_0 ArrayStride 16
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_arr_int_int_2_0 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2_0 = OpTypePointer Function %_arr_int_int_2_0
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2_0
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
%i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%s2 = OpVariable %_ptr_Function__arr_int_int_2_0 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2_0 Function
%cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%95 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%36 = OpCompositeConstruct %_arr_int_int_2_0 %int_1 %int_2
OpStore %s2 %36
%42 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %42
%46 = OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
OpStore %h2 %46
%48 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %cf2 %48
%50 = OpLoad %_arr_int_int_2 %i2
%51 = OpLoad %_arr_int_int_2_0 %s2
%52 = OpCompositeExtract %int %50 0
%53 = OpCompositeExtract %int %51 0
%54 = OpIEqual %bool %52 %53
%55 = OpCompositeExtract %int %50 1
%56 = OpCompositeExtract %int %51 1
%57 = OpIEqual %bool %55 %56
%58 = OpLogicalAnd %bool %57 %54
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %_arr_float_int_2 %f2
%62 = OpLoad %_arr_float_int_2_0 %h2
%63 = OpCompositeExtract %float %61 0
%64 = OpCompositeExtract %float %62 0
%65 = OpFOrdEqual %bool %63 %64
%66 = OpCompositeExtract %float %61 1
%67 = OpCompositeExtract %float %62 1
%68 = OpFOrdEqual %bool %66 %67
%69 = OpLogicalAnd %bool %68 %65
OpBranch %60
%60 = OpLabel
%70 = OpPhi %bool %false %25 %69 %59
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %_arr_int_int_2 %i2
%74 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
%75 = OpCompositeExtract %int %73 0
%76 = OpCompositeExtract %int %74 0
%77 = OpIEqual %bool %75 %76
%78 = OpCompositeExtract %int %73 1
%79 = OpCompositeExtract %int %74 1
%80 = OpIEqual %bool %78 %79
%81 = OpLogicalAnd %bool %80 %77
OpBranch %72
%72 = OpLabel
%82 = OpPhi %bool %false %60 %81 %71
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %_arr_float_int_2_0 %h2
%86 = OpLoad %_arr_float_int_2 %cf2
%87 = OpCompositeExtract %float %85 0
%88 = OpCompositeExtract %float %86 0
%89 = OpFOrdEqual %bool %87 %88
%90 = OpCompositeExtract %float %85 1
%91 = OpCompositeExtract %float %86 1
%92 = OpFOrdEqual %bool %90 %91
%93 = OpLogicalAnd %bool %92 %89
OpBranch %84
%84 = OpLabel
%94 = OpPhi %bool %false %72 %93 %83
OpSelectionMerge %99 None
OpBranchConditional %94 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%103 = OpLoad %v4float %100
OpStore %95 %103
OpBranch %99
%98 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
OpStore %95 %105
OpBranch %99
%99 = OpLabel
%106 = OpLoad %v4float %95
OpReturnValue %106
OpFunctionEnd
