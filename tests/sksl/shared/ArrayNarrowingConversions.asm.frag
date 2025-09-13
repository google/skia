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
               OpName %i2 "i2"                          ; id %27
               OpName %s2 "s2"                          ; id %33
               OpName %f2 "f2"                          ; id %35
               OpName %h2 "h2"                          ; id %41
               OpName %cf2 "cf2"                        ; id %43

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
               OpDecorate %_arr_int_int_2 ArrayStride 16
               OpDecorate %s2 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %h2 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision

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
      %int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2           ; ArrayStride 16
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
      %int_1 = OpConstant %int 1
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
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
         %i2 =   OpVariable %_ptr_Function__arr_int_int_2 Function
         %s2 =   OpVariable %_ptr_Function__arr_int_int_2 Function  ; RelaxedPrecision
         %f2 =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %h2 =   OpVariable %_ptr_Function__arr_float_int_2 Function    ; RelaxedPrecision
        %cf2 =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %60 =   OpVariable %_ptr_Function_v4float Function
         %32 =   OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
                 OpStore %i2 %32
         %34 =   OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2     ; RelaxedPrecision
                 OpStore %s2 %34
         %40 =   OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
                 OpStore %f2 %40
         %42 =   OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2   ; RelaxedPrecision
                 OpStore %h2 %42
                 OpStore %i2 %34
                 OpStore %s2 %34
                 OpStore %f2 %42
                 OpStore %h2 %42
                 OpStore %cf2 %40
         %47 =   OpLogicalAnd %bool %true %true
                 OpSelectionMerge %49 None
                 OpBranchConditional %47 %48 %49

         %48 =     OpLabel
         %50 =       OpLogicalAnd %bool %true %true
                     OpBranch %49

         %49 = OpLabel
         %51 =   OpPhi %bool %false %26 %50 %48
                 OpSelectionMerge %53 None
                 OpBranchConditional %51 %52 %53

         %52 =     OpLabel
         %54 =       OpLogicalAnd %bool %true %true
                     OpBranch %53

         %53 = OpLabel
         %55 =   OpPhi %bool %false %49 %54 %52
                 OpSelectionMerge %57 None
                 OpBranchConditional %55 %56 %57

         %56 =     OpLabel
         %58 =       OpLogicalAnd %bool %true %true
                     OpBranch %57

         %57 = OpLabel
         %59 =   OpPhi %bool %false %53 %58 %56
                 OpSelectionMerge %64 None
                 OpBranchConditional %59 %62 %63

         %62 =     OpLabel
         %65 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =       OpLoad %v4float %65            ; RelaxedPrecision
                     OpStore %60 %68
                     OpBranch %64

         %63 =     OpLabel
         %69 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %70 =       OpLoad %v4float %69            ; RelaxedPrecision
                     OpStore %60 %70
                     OpBranch %64

         %64 = OpLabel
         %71 =   OpLoad %v4float %60                ; RelaxedPrecision
                 OpReturnValue %71
               OpFunctionEnd
