               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"
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
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %v3float = OpTypeVector %float 3
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %11
         %12 = OpLabel
         %13 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %17 = OpLoad %v4float %13
         %18 = OpVectorShuffle %v3float %17 %17 0 1 2
         %20 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %22 = OpLoad %v4float %20
         %23 = OpVectorShuffle %v3float %22 %22 0 1 2
         %24 = OpFAdd %v3float %18 %23
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %28 = OpLoad %v4float %27
         %29 = OpVectorShuffle %v3float %28 %28 0 1 2
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %31 = OpLoad %v4float %30
         %32 = OpCompositeExtract %float %31 3
         %33 = OpVectorTimesScalar %v3float %29 %32
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %35 = OpLoad %v4float %34
         %36 = OpVectorShuffle %v3float %35 %35 0 1 2
         %37 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %37
         %39 = OpCompositeExtract %float %38 3
         %40 = OpVectorTimesScalar %v3float %36 %39
         %26 = OpExtInst %v3float %1 FMin %33 %40
         %41 = OpVectorTimesScalar %v3float %26 %float_2
         %42 = OpFSub %v3float %24 %41
         %43 = OpCompositeExtract %float %42 0
         %44 = OpCompositeExtract %float %42 1
         %45 = OpCompositeExtract %float %42 2
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpCompositeExtract %float %47 3
         %50 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpCompositeExtract %float %51 3
         %53 = OpFSub %float %float_1 %52
         %54 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %55 = OpLoad %v4float %54
         %56 = OpCompositeExtract %float %55 3
         %57 = OpFMul %float %53 %56
         %58 = OpFAdd %float %48 %57
         %59 = OpCompositeConstruct %v4float %43 %44 %45 %58
               OpStore %sk_FragColor %59
               OpReturn
               OpFunctionEnd
