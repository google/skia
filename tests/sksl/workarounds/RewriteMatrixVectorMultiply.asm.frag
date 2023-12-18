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
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
         %35 = OpAccessChain %_ptr_Function_v4float %m44 %int_0
         %36 = OpLoad %v4float %35
         %37 = OpVectorTimesScalar %v4float %36 %float_0
         %39 = OpAccessChain %_ptr_Function_v4float %m44 %int_1
         %40 = OpLoad %v4float %39
         %41 = OpVectorTimesScalar %v4float %40 %float_1
         %42 = OpFAdd %v4float %37 %41
         %44 = OpAccessChain %_ptr_Function_v4float %m44 %int_2
         %45 = OpLoad %v4float %44
         %46 = OpVectorTimesScalar %v4float %45 %float_2
         %47 = OpFAdd %v4float %42 %46
         %49 = OpAccessChain %_ptr_Function_v4float %m44 %int_3
         %50 = OpLoad %v4float %49
         %51 = OpVectorTimesScalar %v4float %50 %float_3
         %52 = OpFAdd %v4float %47 %51
         %53 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %55 = OpAccessChain %_ptr_Uniform_v4float %53 %int_0
         %57 = OpLoad %v4float %55
         %58 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %59 = OpLoad %v4float %58
         %60 = OpCompositeExtract %float %59 0
         %61 = OpVectorTimesScalar %v4float %57 %60
         %62 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %63 = OpAccessChain %_ptr_Uniform_v4float %62 %int_1
         %64 = OpLoad %v4float %63
         %65 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %66 = OpLoad %v4float %65
         %67 = OpCompositeExtract %float %66 1
         %68 = OpVectorTimesScalar %v4float %64 %67
         %69 = OpFAdd %v4float %61 %68
         %70 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %71 = OpAccessChain %_ptr_Uniform_v4float %70 %int_2
         %72 = OpLoad %v4float %71
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %74 = OpLoad %v4float %73
         %75 = OpCompositeExtract %float %74 2
         %76 = OpVectorTimesScalar %v4float %72 %75
         %77 = OpFAdd %v4float %69 %76
         %78 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %79 = OpAccessChain %_ptr_Uniform_v4float %78 %int_3
         %80 = OpLoad %v4float %79
         %81 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %82 = OpLoad %v4float %81
         %83 = OpCompositeExtract %float %82 3
         %84 = OpVectorTimesScalar %v4float %80 %83
         %85 = OpFAdd %v4float %77 %84
         %86 = OpFAdd %v4float %52 %85
               OpReturnValue %86
               OpFunctionEnd
