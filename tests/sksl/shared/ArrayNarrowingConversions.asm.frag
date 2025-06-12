               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %i2 "i2"                          ; id %23
               OpName %s2 "s2"                          ; id %30
               OpName %f2 "f2"                          ; id %32
               OpName %h2 "h2"                          ; id %38
               OpName %cf2 "cf2"                        ; id %40

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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %_arr_int_int_2 ArrayStride 16
               OpDecorate %s2 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %h2 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
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
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
         %i2 =   OpVariable %_ptr_Function__arr_int_int_2 Function
         %s2 =   OpVariable %_ptr_Function__arr_int_int_2 Function  ; RelaxedPrecision
         %f2 =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %h2 =   OpVariable %_ptr_Function__arr_float_int_2 Function    ; RelaxedPrecision
        %cf2 =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %57 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
                 OpStore %i2 %29
         %31 =   OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2     ; RelaxedPrecision
                 OpStore %s2 %31
         %37 =   OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
                 OpStore %f2 %37
         %39 =   OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2   ; RelaxedPrecision
                 OpStore %h2 %39
                 OpStore %i2 %31
                 OpStore %s2 %31
                 OpStore %f2 %39
                 OpStore %h2 %39
                 OpStore %cf2 %37
         %44 =   OpLogicalAnd %bool %true %true
                 OpSelectionMerge %46 None
                 OpBranchConditional %44 %45 %46

         %45 =     OpLabel
         %47 =       OpLogicalAnd %bool %true %true
                     OpBranch %46

         %46 = OpLabel
         %48 =   OpPhi %bool %false %22 %47 %45
                 OpSelectionMerge %50 None
                 OpBranchConditional %48 %49 %50

         %49 =     OpLabel
         %51 =       OpLogicalAnd %bool %true %true
                     OpBranch %50

         %50 = OpLabel
         %52 =   OpPhi %bool %false %46 %51 %49
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %55 =       OpLogicalAnd %bool %true %true
                     OpBranch %54

         %54 = OpLabel
         %56 =   OpPhi %bool %false %50 %55 %53
                 OpSelectionMerge %61 None
                 OpBranchConditional %56 %59 %60

         %59 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 =       OpLoad %v4float %62            ; RelaxedPrecision
                     OpStore %57 %65
                     OpBranch %61

         %60 =     OpLabel
         %66 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %67 =       OpLoad %v4float %66            ; RelaxedPrecision
                     OpStore %57 %67
                     OpBranch %61

         %61 = OpLabel
         %68 =   OpLoad %v4float %57                ; RelaxedPrecision
                 OpReturnValue %68
               OpFunctionEnd
