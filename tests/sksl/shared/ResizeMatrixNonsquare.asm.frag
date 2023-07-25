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
               OpName %result "result"
               OpName %g "g"
               OpName %h "h"
               OpName %i "i"
               OpName %j "j"
               OpName %k "k"
               OpName %l "l"
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
         %47 = OpConstantComposite %v2float %float_1 %float_0
         %48 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
         %50 = OpConstantComposite %mat3v2float %47 %48 %19
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
         %59 = OpConstantComposite %mat4v2float %47 %48 %19 %19
         %60 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat4v3float = OpTypeMatrix %v3float 4
         %62 = OpConstantComposite %mat4v3float %33 %34 %37 %60
         %63 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %64 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %65 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %66 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %67 = OpConstantComposite %mat4v4float %63 %64 %65 %66
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
         %75 = OpConstantComposite %mat2v4float %63 %64
%mat3v4float = OpTypeMatrix %v4float 3
         %77 = OpConstantComposite %mat3v4float %63 %64 %65
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
         %99 = OpVariable %_ptr_Function_v4float Function
               OpStore %result %float_0
               OpStore %g %38
         %41 = OpAccessChain %_ptr_Function_v3float %g %int_0
         %43 = OpLoad %v3float %41
         %44 = OpCompositeExtract %float %43 0
         %45 = OpFAdd %float %float_0 %44
               OpStore %result %45
               OpStore %h %38
         %51 = OpAccessChain %_ptr_Function_v3float %h %int_0
         %52 = OpLoad %v3float %51
         %53 = OpCompositeExtract %float %52 0
         %54 = OpFAdd %float %45 %53
               OpStore %result %54
               OpStore %i %67
         %68 = OpAccessChain %_ptr_Function_v4float %i %int_0
         %70 = OpLoad %v4float %68
         %71 = OpCompositeExtract %float %70 0
         %72 = OpFAdd %float %54 %71
               OpStore %result %72
               OpStore %j %67
         %78 = OpAccessChain %_ptr_Function_v4float %j %int_0
         %79 = OpLoad %v4float %78
         %80 = OpCompositeExtract %float %79 0
         %81 = OpFAdd %float %72 %80
               OpStore %result %81
               OpStore %k %75
         %84 = OpAccessChain %_ptr_Function_v4float %k %int_0
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 0
         %87 = OpFAdd %float %81 %86
               OpStore %result %87
         %90 = OpVectorShuffle %v2float %63 %63 0 1
         %91 = OpVectorShuffle %v2float %64 %64 0 1
         %92 = OpCompositeConstruct %mat4v2float %90 %91 %19 %19
               OpStore %l %92
         %93 = OpAccessChain %_ptr_Function_v2float %l %int_0
         %94 = OpLoad %v2float %93
         %95 = OpCompositeExtract %float %94 0
         %96 = OpFAdd %float %87 %95
               OpStore %result %96
         %98 = OpFOrdEqual %bool %96 %float_6
               OpSelectionMerge %102 None
               OpBranchConditional %98 %100 %101
        %100 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %105 = OpLoad %v4float %103
               OpStore %99 %105
               OpBranch %102
        %101 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %108 = OpLoad %v4float %106
               OpStore %99 %108
               OpBranch %102
        %102 = OpLabel
        %109 = OpLoad %v4float %99
               OpReturnValue %109
               OpFunctionEnd
