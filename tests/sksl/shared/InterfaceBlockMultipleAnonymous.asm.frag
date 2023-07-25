               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testBlockA "testBlockA"
               OpMemberName %testBlockA 0 "x"
               OpName %testBlockB "testBlockB"
               OpMemberName %testBlockB 0 "y"
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
 %testBlockA = OpTypeStruct %v2float
%_ptr_Uniform_testBlockA = OpTypePointer Uniform %testBlockA
          %3 = OpVariable %_ptr_Uniform_testBlockA Uniform
 %testBlockB = OpTypeStruct %v2float
%_ptr_Uniform_testBlockB = OpTypePointer Uniform %testBlockB
          %8 = OpVariable %_ptr_Uniform_testBlockB Uniform
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
       %main = OpFunction %void None %18
         %19 = OpLabel
         %22 = OpAccessChain %_ptr_Uniform_v2float %3 %int_0
         %24 = OpLoad %v2float %22
         %25 = OpCompositeExtract %float %24 0
         %26 = OpCompositeExtract %float %24 1
         %27 = OpAccessChain %_ptr_Uniform_v2float %8 %int_0
         %28 = OpLoad %v2float %27
         %29 = OpCompositeExtract %float %28 0
         %30 = OpCompositeExtract %float %28 1
         %31 = OpCompositeConstruct %v4float %25 %26 %29 %30
               OpStore %sk_FragColor %31
               OpReturn
               OpFunctionEnd
