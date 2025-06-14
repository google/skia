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
               OpName %u1 "u1"                          ; id %27
               OpName %u2 "u2"                          ; id %33
               OpName %u3 "u3"                          ; id %36
               OpName %u4 "u4"                          ; id %39
               OpName %u5 "u5"                          ; id %42

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
               OpDecorate %u5 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision

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
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
%uint_305441741 = OpConstant %uint 305441741
%uint_2147483646 = OpConstant %uint 2147483646
%uint_4294967294 = OpConstant %uint 4294967294
 %uint_65534 = OpConstant %uint 65534
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
         %u1 =   OpVariable %_ptr_Function_uint Function
         %u2 =   OpVariable %_ptr_Function_uint Function
         %u3 =   OpVariable %_ptr_Function_uint Function
         %u4 =   OpVariable %_ptr_Function_uint Function
         %u5 =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
                 OpStore %u1 %uint_0
         %32 =   OpIAdd %uint %uint_0 %uint_1
                 OpStore %u1 %32
                 OpStore %u2 %uint_305441741
         %35 =   OpIAdd %uint %uint_305441741 %uint_1
                 OpStore %u2 %35
                 OpStore %u3 %uint_2147483646
         %38 =   OpIAdd %uint %uint_2147483646 %uint_1
                 OpStore %u3 %38
                 OpStore %u4 %uint_4294967294
         %41 =   OpIAdd %uint %uint_4294967294 %uint_1
                 OpStore %u4 %41
                 OpStore %u5 %uint_65534
         %44 =   OpIAdd %uint %uint_65534 %uint_1   ; RelaxedPrecision
                 OpStore %u5 %44
         %45 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %48 =   OpLoad %v4float %45                ; RelaxedPrecision
                 OpReturnValue %48
               OpFunctionEnd
