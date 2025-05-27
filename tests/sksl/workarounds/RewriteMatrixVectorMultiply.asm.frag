               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix4x4"
               OpMemberName %_UniformBuffer 1 "testValues"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %m44 "m44"
               OpName %v4 "v4"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 64
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %m44 RelaxedPrecision
               OpDecorate %v4 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpTypeFunction %v4float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_123 = OpConstant %float 123
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v4float %float_123 %float_0 %float_0 %float_0
         %23 = OpConstantComposite %v4float %float_0 %float_123 %float_0 %float_0
         %24 = OpConstantComposite %v4float %float_0 %float_0 %float_123 %float_0
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_123
         %26 = OpConstantComposite %mat4v4float %22 %23 %24 %25
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %32 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %15 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %15
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %16
         %17 = OpLabel
        %m44 = OpVariable %_ptr_Function_mat4v4float Function
         %v4 = OpVariable %_ptr_Function_v4float Function
               OpStore %m44 %26
               OpStore %v4 %32
         %33 = OpVectorTimesScalar %v4float %22 %float_0
         %34 = OpVectorTimesScalar %v4float %23 %float_1
         %35 = OpFAdd %v4float %33 %34
         %36 = OpVectorTimesScalar %v4float %24 %float_2
         %37 = OpFAdd %v4float %35 %36
         %38 = OpVectorTimesScalar %v4float %25 %float_3
         %39 = OpFAdd %v4float %37 %38
         %40 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %44 = OpLoad %mat4v4float %40
         %45 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %48 = OpLoad %v4float %45
         %49 = OpCompositeExtract %v4float %44 0
         %50 = OpCompositeExtract %float %48 0
         %51 = OpVectorTimesScalar %v4float %49 %50
         %52 = OpCompositeExtract %v4float %44 1
         %53 = OpCompositeExtract %float %48 1
         %54 = OpVectorTimesScalar %v4float %52 %53
         %55 = OpFAdd %v4float %51 %54
         %56 = OpCompositeExtract %v4float %44 2
         %57 = OpCompositeExtract %float %48 2
         %58 = OpVectorTimesScalar %v4float %56 %57
         %59 = OpFAdd %v4float %55 %58
         %60 = OpCompositeExtract %v4float %44 3
         %61 = OpCompositeExtract %float %48 3
         %62 = OpVectorTimesScalar %v4float %60 %61
         %63 = OpFAdd %v4float %59 %62
         %64 = OpFAdd %v4float %39 %63
               OpReturnValue %64
               OpFunctionEnd
