               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %one "one"                        ; id %29
               OpName %two "two"                        ; id %38
               OpName %three "three"                    ; id %43
               OpName %four "four"                      ; id %47
               OpName %five "five"                      ; id %52

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %four RelaxedPrecision
               OpDecorate %five RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
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
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5       ; ArrayStride 16
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_5 = OpConstant %float 5
   %float_17 = OpConstant %float 17
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


               ; Function main
       %main = OpFunction %v4float None %26         ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
        %one =   OpVariable %_ptr_Function_float Function
        %two =   OpVariable %_ptr_Function_float Function
      %three =   OpVariable %_ptr_Function_float Function
       %four =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %five =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %68 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %35 =   OpAccessChain %_ptr_Uniform_float %31 %int_0
         %37 =   OpLoad %float %35
                 OpStore %one %37
         %39 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %41 =   OpAccessChain %_ptr_Uniform_float %39 %int_1
         %42 =   OpLoad %float %41
                 OpStore %two %42
         %44 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %45 =   OpAccessChain %_ptr_Uniform_float %44 %int_2
         %46 =   OpLoad %float %45
                 OpStore %three %46
         %48 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %50 =   OpAccessChain %_ptr_Uniform_float %48 %int_3
         %51 =   OpLoad %float %50
                 OpStore %four %51
         %53 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %55 =   OpAccessChain %_ptr_Uniform_float %53 %int_4
         %56 =   OpLoad %float %55
                 OpStore %five %56
         %59 =   OpExtInst %float %5 Fma %37 %42 %46
         %61 =   OpFOrdEqual %bool %59 %float_5
                 OpSelectionMerge %63 None
                 OpBranchConditional %61 %62 %63

         %62 =     OpLabel
         %64 =       OpExtInst %float %5 Fma %46 %51 %56    ; RelaxedPrecision
         %66 =       OpFOrdEqual %bool %64 %float_17
                     OpBranch %63

         %63 = OpLabel
         %67 =   OpPhi %bool %false %28 %66 %62
                 OpSelectionMerge %72 None
                 OpBranchConditional %67 %70 %71

         %70 =     OpLabel
         %73 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %75 =       OpLoad %v4float %73            ; RelaxedPrecision
                     OpStore %68 %75
                     OpBranch %72

         %71 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %68 %77
                     OpBranch %72

         %72 = OpLabel
         %78 =   OpLoad %v4float %68                ; RelaxedPrecision
                 OpReturnValue %78
               OpFunctionEnd
