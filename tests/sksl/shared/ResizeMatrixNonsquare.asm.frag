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
OpName %result "result"
OpName %g "g"
OpName %h "h"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpName %l "l"
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
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
%36 = OpConstantComposite %mat2v3float %33 %34
%37 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%38 = OpConstantComposite %mat3v3float %33 %34 %37
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%48 = OpConstantComposite %v2float %float_1 %float_0
%49 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
%51 = OpConstantComposite %mat3v2float %48 %49 %19
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%61 = OpConstantComposite %mat4v2float %48 %49 %19 %19
%62 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat4v3float = OpTypeMatrix %v3float 4
%64 = OpConstantComposite %mat4v3float %33 %34 %37 %62
%65 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%66 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%67 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%69 = OpConstantComposite %mat4v4float %65 %66 %67 %68
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
%78 = OpConstantComposite %mat2v4float %65 %66
%mat3v4float = OpTypeMatrix %v4float 3
%80 = OpConstantComposite %mat3v4float %65 %66 %67
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%result = OpVariable %_ptr_Function_float Function
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%106 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
OpStore %g %38
%39 = OpLoad %float %result
%42 = OpAccessChain %_ptr_Function_v3float %g %int_0
%44 = OpLoad %v3float %42
%45 = OpCompositeExtract %float %44 0
%46 = OpFAdd %float %39 %45
OpStore %result %46
OpStore %h %38
%52 = OpLoad %float %result
%53 = OpAccessChain %_ptr_Function_v3float %h %int_0
%54 = OpLoad %v3float %53
%55 = OpCompositeExtract %float %54 0
%56 = OpFAdd %float %52 %55
OpStore %result %56
OpStore %i %69
%70 = OpLoad %float %result
%71 = OpAccessChain %_ptr_Function_v4float %i %int_0
%73 = OpLoad %v4float %71
%74 = OpCompositeExtract %float %73 0
%75 = OpFAdd %float %70 %74
OpStore %result %75
OpStore %j %69
%81 = OpLoad %float %result
%82 = OpAccessChain %_ptr_Function_v4float %j %int_0
%83 = OpLoad %v4float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpFAdd %float %81 %84
OpStore %result %85
OpStore %k %78
%88 = OpLoad %float %result
%89 = OpAccessChain %_ptr_Function_v4float %k %int_0
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 0
%92 = OpFAdd %float %88 %91
OpStore %result %92
%95 = OpVectorShuffle %v2float %65 %65 0 1
%96 = OpVectorShuffle %v2float %66 %66 0 1
%97 = OpCompositeConstruct %mat4v2float %95 %96 %19 %19
OpStore %l %97
%98 = OpLoad %float %result
%99 = OpAccessChain %_ptr_Function_v2float %l %int_0
%100 = OpLoad %v2float %99
%101 = OpCompositeExtract %float %100 0
%102 = OpFAdd %float %98 %101
OpStore %result %102
%103 = OpLoad %float %result
%105 = OpFOrdEqual %bool %103 %float_6
OpSelectionMerge %109 None
OpBranchConditional %105 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%112 = OpLoad %v4float %110
OpStore %106 %112
OpBranch %109
%108 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%115 = OpLoad %v4float %113
OpStore %106 %115
OpBranch %109
%109 = OpLabel
%116 = OpLoad %v4float %106
OpReturnValue %116
OpFunctionEnd
