               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %_0_x "_0_x"                      ; id %27
               OpName %_1_x "_1_x"                      ; id %35

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %50 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %36 = OpConstantComposite %v2float %float_1 %float_2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %42 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
       %_0_x =   OpVariable %_ptr_Function_float Function
       %_1_x =   OpVariable %_ptr_Function_v2float Function
                 OpStore %_0_x %float_1
         %30 =   OpExtInst %float %5 Length %float_1
                 OpStore %_0_x %30
         %31 =   OpExtInst %float %5 Distance %30 %float_2
                 OpStore %_0_x %31
         %33 =   OpFMul %float %31 %float_2
                 OpStore %_0_x %33
         %34 =   OpExtInst %float %5 Normalize %33
                 OpStore %_0_x %34
                 OpStore %_1_x %36
         %37 =   OpExtInst %float %5 Length %36
         %38 =   OpCompositeConstruct %v2float %37 %37
                 OpStore %_1_x %38
         %39 =   OpExtInst %float %5 Distance %38 %42
         %43 =   OpCompositeConstruct %v2float %39 %39
                 OpStore %_1_x %43
         %44 =   OpDot %float %43 %42
         %45 =   OpCompositeConstruct %v2float %44 %44
                 OpStore %_1_x %45
         %46 =   OpExtInst %v2float %5 Normalize %45
                 OpStore %_1_x %46
         %47 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %47                ; RelaxedPrecision
                 OpReturnValue %50
               OpFunctionEnd
