               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testBlockA "testBlockA"
               OpMemberName %testBlockA 0 "x"
               OpName %testBlockB "testBlockB"
               OpMemberName %testBlockB 0 "y"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpMemberDecorate %testBlockA 0 Offset 0
               OpDecorate %testBlockA Block
               OpDecorate %3 Binding 1
               OpDecorate %3 DescriptorSet 0
               OpMemberDecorate %testBlockB 0 Offset 0
               OpDecorate %testBlockB Block
               OpDecorate %8 Binding 2
               OpDecorate %8 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
 %testBlockA = OpTypeStruct %v2float
%_ptr_Uniform_testBlockA = OpTypePointer Uniform %testBlockA
          %3 = OpVariable %_ptr_Uniform_testBlockA Uniform
 %testBlockB = OpTypeStruct %v2float
%_ptr_Uniform_testBlockB = OpTypePointer Uniform %testBlockB
          %8 = OpVariable %_ptr_Uniform_testBlockB Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %15
         %16 = OpLabel
         %19 = OpAccessChain %_ptr_Uniform_v2float %3 %int_0
         %21 = OpLoad %v2float %19
         %22 = OpCompositeExtract %float %21 0
         %23 = OpCompositeExtract %float %21 1
         %24 = OpAccessChain %_ptr_Uniform_v2float %8 %int_0
         %25 = OpLoad %v2float %24
         %26 = OpCompositeExtract %float %25 0
         %27 = OpCompositeExtract %float %25 1
         %28 = OpCompositeConstruct %v4float %22 %23 %26 %27
               OpStore %sk_FragColor %28
               OpReturn
               OpFunctionEnd
