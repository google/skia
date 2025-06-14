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
               OpName %out_param_func1_vh "out_param_func1_vh"  ; id %6
               OpName %out_param_func2_ih "out_param_func2_ih"  ; id %7
               OpName %main "main"                              ; id %8
               OpName %testArray "testArray"                    ; id %46

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
               OpDecorate %28 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %testArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision

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
%_ptr_Function_float = OpTypePointer Function %float
         %27 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %35 = OpTypeFunction %int %_ptr_Function_float
      %int_1 = OpConstant %int 1
         %43 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function out_param_func1_vh
%out_param_func1_vh = OpFunction %void None %27
         %28 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %29 = OpLabel
         %30 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 1    ; RelaxedPrecision
                 OpStore %28 %34
                 OpReturn
               OpFunctionEnd


               ; Function out_param_func2_ih
%out_param_func2_ih = OpFunction %int None %35
         %36 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %37 = OpLabel
         %38 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %40 =   OpLoad %v4float %38                ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %40 0    ; RelaxedPrecision
                 OpStore %36 %41
         %42 =   OpConvertFToS %int %41
                 OpReturnValue %42
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %43         ; RelaxedPrecision
         %44 = OpFunctionParameter %_ptr_Function_v2float

         %45 = OpLabel
  %testArray =   OpVariable %_ptr_Function__arr_float_int_2 Function    ; RelaxedPrecision
         %51 =   OpVariable %_ptr_Function_float Function               ; RelaxedPrecision
         %56 =   OpVariable %_ptr_Function_float Function               ; RelaxedPrecision
         %71 =   OpVariable %_ptr_Function_v4float Function
         %50 =   OpAccessChain %_ptr_Function_float %testArray %int_0
         %52 =   OpFunctionCall %int %out_param_func2_ih %51
         %53 =   OpLoad %float %51                  ; RelaxedPrecision
                 OpStore %50 %53
         %54 =   OpAccessChain %_ptr_Function_float %testArray %52
         %55 =   OpLoad %float %54                  ; RelaxedPrecision
                 OpStore %56 %55
         %57 =   OpFunctionCall %void %out_param_func1_vh %56
         %58 =   OpLoad %float %56                  ; RelaxedPrecision
                 OpStore %54 %58
         %61 =   OpAccessChain %_ptr_Function_float %testArray %int_0
         %62 =   OpLoad %float %61                  ; RelaxedPrecision
         %64 =   OpFOrdEqual %bool %62 %float_1
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %67 =       OpAccessChain %_ptr_Function_float %testArray %int_1
         %68 =       OpLoad %float %67              ; RelaxedPrecision
         %69 =       OpFOrdEqual %bool %68 %float_1
                     OpBranch %66

         %66 = OpLabel
         %70 =   OpPhi %bool %false %45 %69 %65
                 OpSelectionMerge %75 None
                 OpBranchConditional %70 %73 %74

         %73 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %71 %77
                     OpBranch %75

         %74 =     OpLabel
         %78 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %79 =       OpLoad %v4float %78            ; RelaxedPrecision
                     OpStore %71 %79
                     OpBranch %75

         %75 = OpLabel
         %80 =   OpLoad %v4float %71                ; RelaxedPrecision
                 OpReturnValue %80
               OpFunctionEnd
