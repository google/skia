               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %fnGreen_h4bf2 "fnGreen_h4bf2"    ; id %6
               OpName %S "S"                            ; id %38
               OpMemberName %S 0 "i"
               OpName %fnRed_h4ifS "fnRed_h4ifS"    ; id %7
               OpName %main "main"                  ; id %8

               ; Annotations
               OpDecorate %fnGreen_h4bf2 RelaxedPrecision
               OpDecorate %fnRed_h4ifS RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %47 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
         %28 = OpTypeFunction %v4float %_ptr_Function_bool %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
          %S = OpTypeStruct %int
%_ptr_Function_S = OpTypePointer Function %S
         %40 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_float %_ptr_Function_S
      %int_1 = OpConstant %int 1
         %48 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %true = OpConstantTrue %bool
    %int_123 = OpConstant %int 123
%float_3_1400001 = OpConstant %float 3.1400001


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function fnGreen_h4bf2
%fnGreen_h4bf2 = OpFunction %v4float None %28       ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_bool
         %30 = OpFunctionParameter %_ptr_Function_v2float

         %31 = OpLabel
         %32 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %35 =   OpLoad %v4float %32                ; RelaxedPrecision
                 OpReturnValue %35
               OpFunctionEnd


               ; Function fnRed_h4ifS
%fnRed_h4ifS = OpFunction %v4float None %40         ; RelaxedPrecision
         %41 = OpFunctionParameter %_ptr_Function_int
         %42 = OpFunctionParameter %_ptr_Function_float
         %43 = OpFunctionParameter %_ptr_Function_S

         %44 = OpLabel
         %45 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %47 =   OpLoad %v4float %45                ; RelaxedPrecision
                 OpReturnValue %47
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %48         ; RelaxedPrecision
         %49 = OpFunctionParameter %_ptr_Function_v2float

         %50 = OpLabel
         %55 =   OpVariable %_ptr_Function_v4float Function
         %61 =   OpVariable %_ptr_Function_bool Function
         %63 =   OpVariable %_ptr_Function_v2float Function
         %66 =   OpVariable %_ptr_Function_int Function
         %68 =   OpVariable %_ptr_Function_float Function
         %70 =   OpVariable %_ptr_Function_S Function
         %51 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %52 =   OpLoad %v4float %51                ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %52 1    ; RelaxedPrecision
         %54 =   OpFUnordNotEqual %bool %53 %float_0
                 OpSelectionMerge %59 None
                 OpBranchConditional %54 %57 %58

         %57 =     OpLabel
                     OpStore %61 %true
         %62 =       OpLoad %v2float %49
                     OpStore %63 %62
         %64 =       OpFunctionCall %v4float %fnGreen_h4bf2 %61 %63
                     OpStore %55 %64
                     OpBranch %59

         %58 =     OpLabel
                     OpStore %66 %int_123
                     OpStore %68 %float_3_1400001
         %69 =       OpCompositeConstruct %S %int_0
                     OpStore %70 %69
         %71 =       OpFunctionCall %v4float %fnRed_h4ifS %66 %68 %70
                     OpStore %55 %71
                     OpBranch %59

         %59 = OpLabel
         %72 =   OpLoad %v4float %55                ; RelaxedPrecision
                 OpReturnValue %72
               OpFunctionEnd
