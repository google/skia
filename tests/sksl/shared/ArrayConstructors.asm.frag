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
               OpName %test1 "test1"                    ; id %27
               OpName %test2 "test2"                    ; id %36
               OpName %test3 "test3"                    ; id %43

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
               OpDecorate %_arr_v2float_int_2 ArrayStride 16
               OpDecorate %_arr_mat4v4float_int_1 ArrayStride 64
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision

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
      %int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2   ; ArrayStride 16
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
         %40 = OpConstantComposite %v2float %float_1 %float_2
         %41 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
      %int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1   ; ArrayStride 64
%_ptr_Function__arr_mat4v4float_int_1 = OpTypePointer Function %_arr_mat4v4float_int_1
   %float_16 = OpConstant %float 16
         %49 = OpConstantComposite %v4float %float_16 %float_0 %float_0 %float_0
         %50 = OpConstantComposite %v4float %float_0 %float_16 %float_0 %float_0
         %51 = OpConstantComposite %v4float %float_0 %float_0 %float_16 %float_0
         %52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_16
         %53 = OpConstantComposite %mat4v4float %49 %50 %51 %52
      %int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %float_24 = OpConstant %float 24
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


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
      %test1 =   OpVariable %_ptr_Function__arr_float_int_4 Function
      %test2 =   OpVariable %_ptr_Function__arr_v2float_int_2 Function
      %test3 =   OpVariable %_ptr_Function__arr_mat4v4float_int_1 Function
         %72 =   OpVariable %_ptr_Function_v4float Function
         %35 =   OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
                 OpStore %test1 %35
         %42 =   OpCompositeConstruct %_arr_v2float_int_2 %40 %41
                 OpStore %test2 %42
         %54 =   OpCompositeConstruct %_arr_mat4v4float_int_1 %53
                 OpStore %test3 %54
         %56 =   OpAccessChain %_ptr_Function_float %test1 %int_3
         %58 =   OpLoad %float %56
         %59 =   OpAccessChain %_ptr_Function_v2float %test2 %int_1
         %60 =   OpLoad %v2float %59
         %61 =   OpCompositeExtract %float %60 1
         %62 =   OpFAdd %float %58 %61
         %64 =   OpAccessChain %_ptr_Function_v4float %test3 %int_0 %int_3
         %66 =   OpLoad %v4float %64
         %67 =   OpCompositeExtract %float %66 3
         %68 =   OpFAdd %float %62 %67
         %70 =   OpFOrdEqual %bool %68 %float_24
                 OpSelectionMerge %75 None
                 OpBranchConditional %70 %73 %74

         %73 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %78 =       OpLoad %v4float %76            ; RelaxedPrecision
                     OpStore %72 %78
                     OpBranch %75

         %74 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %80 =       OpLoad %v4float %79            ; RelaxedPrecision
                     OpStore %72 %80
                     OpBranch %75

         %75 = OpLabel
         %81 =   OpLoad %v4float %72                ; RelaxedPrecision
                 OpReturnValue %81
               OpFunctionEnd
