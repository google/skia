               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %_0_x "_0_x"                      ; id %23
               OpName %_1_x "_1_x"                      ; id %31

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %47 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %32 = OpConstantComposite %v2float %float_1 %float_2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %38 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
       %_0_x =   OpVariable %_ptr_Function_float Function
       %_1_x =   OpVariable %_ptr_Function_v2float Function
                 OpStore %_0_x %float_1
         %26 =   OpExtInst %float %1 Length %float_1
                 OpStore %_0_x %26
         %27 =   OpExtInst %float %1 Distance %26 %float_2
                 OpStore %_0_x %27
         %29 =   OpFMul %float %27 %float_2
                 OpStore %_0_x %29
         %30 =   OpExtInst %float %1 Normalize %29
                 OpStore %_0_x %30
                 OpStore %_1_x %32
         %33 =   OpExtInst %float %1 Length %32
         %34 =   OpCompositeConstruct %v2float %33 %33
                 OpStore %_1_x %34
         %35 =   OpExtInst %float %1 Distance %34 %38
         %39 =   OpCompositeConstruct %v2float %35 %35
                 OpStore %_1_x %39
         %40 =   OpDot %float %39 %38
         %41 =   OpCompositeConstruct %v2float %40 %40
                 OpStore %_1_x %41
         %42 =   OpExtInst %v2float %1 Normalize %41
                 OpStore %_1_x %42
         %43 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 =   OpLoad %v4float %43                ; RelaxedPrecision
                 OpReturnValue %47
               OpFunctionEnd
