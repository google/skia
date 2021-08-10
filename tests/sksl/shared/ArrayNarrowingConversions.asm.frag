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
OpDecorate %35 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
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
%_ptr_Function__arr_int_int_2_0 = OpTypePointer Function %_arr_int_int_2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2
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
%97 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%35 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %s2 %35
%41 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %41
%44 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %h2 %44
%45 = OpLoad %_arr_int_int_2 %s2
OpStore %i2 %45
%46 = OpLoad %_arr_int_int_2 %i2
OpStore %s2 %46
%47 = OpLoad %_arr_float_int_2 %h2
OpStore %f2 %47
%48 = OpLoad %_arr_float_int_2 %f2
OpStore %h2 %48
%50 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %cf2 %50
%52 = OpLoad %_arr_int_int_2 %i2
%53 = OpLoad %_arr_int_int_2 %s2
%54 = OpCompositeExtract %int %52 0
%55 = OpCompositeExtract %int %53 0
%56 = OpIEqual %bool %54 %55
%57 = OpCompositeExtract %int %52 1
%58 = OpCompositeExtract %int %53 1
%59 = OpIEqual %bool %57 %58
%60 = OpLogicalAnd %bool %59 %56
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpLoad %_arr_float_int_2 %f2
%64 = OpLoad %_arr_float_int_2 %h2
%65 = OpCompositeExtract %float %63 0
%66 = OpCompositeExtract %float %64 0
%67 = OpFOrdEqual %bool %65 %66
%68 = OpCompositeExtract %float %63 1
%69 = OpCompositeExtract %float %64 1
%70 = OpFOrdEqual %bool %68 %69
%71 = OpLogicalAnd %bool %70 %67
OpBranch %62
%62 = OpLabel
%72 = OpPhi %bool %false %25 %71 %61
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %_arr_int_int_2 %i2
%76 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
%77 = OpCompositeExtract %int %75 0
%78 = OpCompositeExtract %int %76 0
%79 = OpIEqual %bool %77 %78
%80 = OpCompositeExtract %int %75 1
%81 = OpCompositeExtract %int %76 1
%82 = OpIEqual %bool %80 %81
%83 = OpLogicalAnd %bool %82 %79
OpBranch %74
%74 = OpLabel
%84 = OpPhi %bool %false %62 %83 %73
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %_arr_float_int_2 %h2
%88 = OpLoad %_arr_float_int_2 %cf2
%89 = OpCompositeExtract %float %87 0
%90 = OpCompositeExtract %float %88 0
%91 = OpFOrdEqual %bool %89 %90
%92 = OpCompositeExtract %float %87 1
%93 = OpCompositeExtract %float %88 1
%94 = OpFOrdEqual %bool %92 %93
%95 = OpLogicalAnd %bool %94 %91
OpBranch %86
%86 = OpLabel
%96 = OpPhi %bool %false %74 %95 %85
OpSelectionMerge %101 None
OpBranchConditional %96 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%105 = OpLoad %v4float %102
OpStore %97 %105
OpBranch %101
%100 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%107 = OpLoad %v4float %106
OpStore %97 %107
OpBranch %101
%101 = OpLabel
%108 = OpLoad %v4float %97
OpReturnValue %108
OpFunctionEnd
