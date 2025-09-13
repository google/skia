               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %out_param_func2_ih "out_param_func2_ih"  ; id %6
               OpName %main "main"                              ; id %7
               OpName %testArray "testArray"                    ; id %38

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %testArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %26 = OpTypeFunction %int %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function out_param_func2_ih
%out_param_func2_ih = OpFunction %int None %26
         %27 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %28 = OpLabel
         %29 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 0    ; RelaxedPrecision
                 OpStore %27 %33
         %34 =   OpConvertFToS %int %33
                 OpReturnValue %34
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %35         ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float

         %37 = OpLabel
  %testArray =   OpVariable %_ptr_Function__arr_float_int_2 Function    ; RelaxedPrecision
         %44 =   OpVariable %_ptr_Function_float Function               ; RelaxedPrecision
         %61 =   OpVariable %_ptr_Function_v4float Function
         %43 =   OpAccessChain %_ptr_Function_float %testArray %int_0
         %45 =   OpFunctionCall %int %out_param_func2_ih %44
         %46 =   OpLoad %float %44                  ; RelaxedPrecision
                 OpStore %43 %46
         %47 =   OpAccessChain %_ptr_Function_float %testArray %45
         %48 =   OpLoad %float %47                  ; RelaxedPrecision
         %51 =   OpAccessChain %_ptr_Function_float %testArray %int_0
         %52 =   OpLoad %float %51                  ; RelaxedPrecision
         %54 =   OpFOrdEqual %bool %52 %float_1
                 OpSelectionMerge %56 None
                 OpBranchConditional %54 %55 %56

         %55 =     OpLabel
         %57 =       OpAccessChain %_ptr_Function_float %testArray %int_1
         %58 =       OpLoad %float %57              ; RelaxedPrecision
         %59 =       OpFOrdEqual %bool %58 %float_1
                     OpBranch %56

         %56 = OpLabel
         %60 =   OpPhi %bool %false %37 %59 %55
                 OpSelectionMerge %65 None
                 OpBranchConditional %60 %63 %64

         %63 =     OpLabel
         %66 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %67 =       OpLoad %v4float %66            ; RelaxedPrecision
                     OpStore %61 %67
                     OpBranch %65

         %64 =     OpLabel
         %68 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %69 =       OpLoad %v4float %68            ; RelaxedPrecision
                     OpStore %61 %69
                     OpBranch %65

         %65 = OpLabel
         %70 =   OpLoad %v4float %61                ; RelaxedPrecision
                 OpReturnValue %70
               OpFunctionEnd
