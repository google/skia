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
               OpName %foo_ff2 "foo_ff2"                ; id %6
               OpName %bar_vf "bar_vf"                  ; id %7
               OpName %y "y"                            ; id %38
               OpName %main "main"                      ; id %8
               OpName %x "x"                            ; id %60

               ; Annotations
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
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision

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
         %26 = OpTypeFunction %float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
         %35 = OpTypeFunction %void %_ptr_Function_float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
      %int_1 = OpConstant %int 1
         %57 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_10 = OpConstant %float 10
  %float_200 = OpConstant %float 200
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function foo_ff2
    %foo_ff2 = OpFunction %float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
         %29 =   OpLoad %v2float %27
         %30 =   OpCompositeExtract %float %29 0
         %31 =   OpLoad %v2float %27
         %32 =   OpCompositeExtract %float %31 1
         %33 =   OpFMul %float %30 %32
                 OpReturnValue %33
               OpFunctionEnd


               ; Function bar_vf
     %bar_vf = OpFunction %void None %35
         %36 = OpFunctionParameter %_ptr_Function_float

         %37 = OpLabel
          %y =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %55 =   OpVariable %_ptr_Function_v2float Function
         %42 =   OpLoad %float %36
         %44 =   OpAccessChain %_ptr_Function_float %y %int_0
                 OpStore %44 %42
         %45 =   OpLoad %float %36
         %47 =   OpFMul %float %45 %float_2
         %49 =   OpAccessChain %_ptr_Function_float %y %int_1
                 OpStore %49 %47
         %50 =   OpAccessChain %_ptr_Function_float %y %int_0
         %51 =   OpLoad %float %50
         %52 =   OpAccessChain %_ptr_Function_float %y %int_1
         %53 =   OpLoad %float %52
         %54 =   OpCompositeConstruct %v2float %51 %53
                 OpStore %55 %54
         %56 =   OpFunctionCall %float %foo_ff2 %55
                 OpStore %36 %56
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %57         ; RelaxedPrecision
         %58 = OpFunctionParameter %_ptr_Function_v2float

         %59 = OpLabel
          %x =   OpVariable %_ptr_Function_float Function
         %62 =   OpVariable %_ptr_Function_float Function
         %68 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %x %float_10
                 OpStore %62 %float_10
         %63 =   OpFunctionCall %void %bar_vf %62
         %64 =   OpLoad %float %62
                 OpStore %x %64
         %66 =   OpFOrdEqual %bool %64 %float_200
                 OpSelectionMerge %72 None
                 OpBranchConditional %66 %70 %71

         %70 =     OpLabel
         %73 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %75 =       OpLoad %v4float %73            ; RelaxedPrecision
                     OpStore %68 %75
                     OpBranch %72

         %71 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %68 %77
                     OpBranch %72

         %72 = OpLabel
         %78 =   OpLoad %v4float %68                ; RelaxedPrecision
                 OpReturnValue %78
               OpFunctionEnd
