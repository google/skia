               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %f "f"                            ; id %27
               OpName %h "h"                            ; id %36
               OpName %i3 "i3"                          ; id %37
               OpName %s3 "s3"                          ; id %48
               OpName %h2x2 "h2x2"                      ; id %49
               OpName %f2x2 "f2x2"                      ; id %64

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %h RelaxedPrecision
               OpDecorate %_arr_v3int_int_3 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4       ; ArrayStride 16
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %v3int = OpTypeVector %int 3
      %int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3       ; ArrayStride 16
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
      %int_1 = OpConstant %int 1
         %43 = OpConstantComposite %v3int %int_1 %int_1 %int_1
      %int_2 = OpConstant %int 2
         %45 = OpConstantComposite %v3int %int_2 %int_2 %int_2
         %46 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2   ; ArrayStride 32
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
         %53 = OpConstantComposite %v2float %float_1 %float_2
         %54 = OpConstantComposite %v2float %float_3 %float_4
         %55 = OpConstantComposite %mat2v2float %53 %54
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %60 = OpConstantComposite %v2float %float_5 %float_6
         %61 = OpConstantComposite %v2float %float_7 %float_8
         %62 = OpConstantComposite %mat2v2float %60 %61
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
          %f =   OpVariable %_ptr_Function__arr_float_int_4 Function
          %h =   OpVariable %_ptr_Function__arr_float_int_4 Function    ; RelaxedPrecision
         %i3 =   OpVariable %_ptr_Function__arr_v3int_int_3 Function
         %s3 =   OpVariable %_ptr_Function__arr_v3int_int_3 Function
       %h2x2 =   OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
       %f2x2 =   OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
         %91 =   OpVariable %_ptr_Function_v4float Function
         %35 =   OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
                 OpStore %f %35
                 OpStore %h %35
                 OpStore %f %35
                 OpStore %h %35
         %47 =   OpCompositeConstruct %_arr_v3int_int_3 %43 %45 %46
                 OpStore %i3 %47
                 OpStore %s3 %47
                 OpStore %i3 %47
                 OpStore %s3 %47
         %63 =   OpCompositeConstruct %_arr_mat2v2float_int_2 %55 %62
                 OpStore %h2x2 %63
                 OpStore %f2x2 %63
                 OpStore %f2x2 %63
                 OpStore %h2x2 %63
         %68 =   OpLogicalAnd %bool %true %true
         %69 =   OpLogicalAnd %bool %true %68
         %70 =   OpLogicalAnd %bool %true %69
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %73 =       OpLogicalAnd %bool %true %true
         %74 =       OpLogicalAnd %bool %true %73
                     OpBranch %72

         %72 = OpLabel
         %75 =   OpPhi %bool %false %26 %74 %71
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpFOrdEqual %v2bool %53 %53
         %80 =       OpAll %bool %79
         %81 =       OpFOrdEqual %v2bool %54 %54
         %82 =       OpAll %bool %81
         %83 =       OpLogicalAnd %bool %80 %82
         %84 =       OpFOrdEqual %v2bool %60 %60
         %85 =       OpAll %bool %84
         %86 =       OpFOrdEqual %v2bool %61 %61
         %87 =       OpAll %bool %86
         %88 =       OpLogicalAnd %bool %85 %87
         %89 =       OpLogicalAnd %bool %88 %83
                     OpBranch %77

         %77 = OpLabel
         %90 =   OpPhi %bool %false %72 %89 %76
                 OpSelectionMerge %95 None
                 OpBranchConditional %90 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %99 =       OpLoad %v4float %96            ; RelaxedPrecision
                     OpStore %91 %99
                     OpBranch %95

         %94 =     OpLabel
        %100 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %101 =       OpLoad %v4float %100           ; RelaxedPrecision
                     OpStore %91 %101
                     OpBranch %95

         %95 = OpLabel
        %102 =   OpLoad %v4float %91                ; RelaxedPrecision
                 OpReturnValue %102
               OpFunctionEnd
