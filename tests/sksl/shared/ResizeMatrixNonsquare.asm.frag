               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v3float %float_1 %float_0 %float_0
         %31 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
         %33 = OpConstantComposite %mat2v3float %30 %31
         %34 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %35 = OpConstantComposite %mat3v3float %30 %31 %34
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %44 = OpConstantComposite %v2float %float_1 %float_0
         %45 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
         %47 = OpConstantComposite %mat3v2float %44 %45 %16
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
         %56 = OpConstantComposite %mat4v2float %44 %45 %16 %16
         %57 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat4v3float = OpTypeMatrix %v3float 4
         %59 = OpConstantComposite %mat4v3float %30 %31 %34 %57
         %60 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %61 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %62 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %63 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %64 = OpConstantComposite %mat4v4float %60 %61 %62 %63
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
         %72 = OpConstantComposite %mat2v4float %60 %61
%mat3v4float = OpTypeMatrix %v4float 3
         %74 = OpConstantComposite %mat3v4float %60 %61 %62
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_6 = OpConstant %float 6
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
     %result = OpVariable %_ptr_Function_float Function
          %g = OpVariable %_ptr_Function_mat3v3float Function
          %h = OpVariable %_ptr_Function_mat3v3float Function
          %i = OpVariable %_ptr_Function_mat4v4float Function
          %j = OpVariable %_ptr_Function_mat4v4float Function
          %k = OpVariable %_ptr_Function_mat2v4float Function
          %l = OpVariable %_ptr_Function_mat4v2float Function
         %97 = OpVariable %_ptr_Function_v4float Function
               OpStore %result %float_0
               OpStore %g %35
         %38 = OpAccessChain %_ptr_Function_v3float %g %int_0
         %40 = OpLoad %v3float %38
         %41 = OpCompositeExtract %float %40 0
         %42 = OpFAdd %float %float_0 %41
               OpStore %result %42
               OpStore %h %35
         %48 = OpAccessChain %_ptr_Function_v3float %h %int_0
         %49 = OpLoad %v3float %48
         %50 = OpCompositeExtract %float %49 0
         %51 = OpFAdd %float %42 %50
               OpStore %result %51
               OpStore %i %64
         %65 = OpAccessChain %_ptr_Function_v4float %i %int_0
         %67 = OpLoad %v4float %65
         %68 = OpCompositeExtract %float %67 0
         %69 = OpFAdd %float %51 %68
               OpStore %result %69
               OpStore %j %64
         %75 = OpAccessChain %_ptr_Function_v4float %j %int_0
         %76 = OpLoad %v4float %75
         %77 = OpCompositeExtract %float %76 0
         %78 = OpFAdd %float %69 %77
               OpStore %result %78
               OpStore %k %72
         %81 = OpAccessChain %_ptr_Function_v4float %k %int_0
         %82 = OpLoad %v4float %81
         %83 = OpCompositeExtract %float %82 0
         %84 = OpFAdd %float %78 %83
               OpStore %result %84
         %87 = OpVectorShuffle %v2float %60 %60 0 1
         %88 = OpVectorShuffle %v2float %61 %61 0 1
         %89 = OpCompositeConstruct %mat4v2float %87 %88 %16 %16
               OpStore %l %89
         %90 = OpAccessChain %_ptr_Function_v2float %l %int_0
         %91 = OpLoad %v2float %90
         %92 = OpCompositeExtract %float %91 0
         %93 = OpFAdd %float %84 %92
               OpStore %result %93
         %95 = OpFOrdEqual %bool %93 %float_6
               OpSelectionMerge %100 None
               OpBranchConditional %95 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %103 = OpLoad %v4float %101
               OpStore %97 %103
               OpBranch %100
         %99 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %106 = OpLoad %v4float %104
               OpStore %97 %106
               OpBranch %100
        %100 = OpLabel
        %107 = OpLoad %v4float %97
               OpReturnValue %107
               OpFunctionEnd
