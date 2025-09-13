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
               OpName %zero "zero"                      ; id %27
               OpName %one "one"                        ; id %35
               OpName %x "x"                            ; id %40
               OpName %y "y"                            ; id %50
               OpName %_0_x "_0_x"                      ; id %59
               OpName %_1_result "_1_result"            ; id %61

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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision

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
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %int_100 = OpConstant %int 100
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886


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
       %zero =   OpVariable %_ptr_Function_int Function
        %one =   OpVariable %_ptr_Function_int Function
          %x =   OpVariable %_ptr_Function_int Function
          %y =   OpVariable %_ptr_Function_int Function
       %_0_x =   OpVariable %_ptr_Function_int Function
  %_1_result =   OpVariable %_ptr_Function_int Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 0    ; RelaxedPrecision
         %34 =   OpConvertFToS %int %33
                 OpStore %zero %34
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %37 =   OpLoad %v4float %36                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %39 =   OpConvertFToS %int %38
                 OpStore %one %39
                 OpStore %x %34
                 OpBranch %41

         %41 = OpLabel
                 OpLoopMerge %45 %44 None
                 OpBranch %42

         %42 =     OpLabel
         %46 =       OpLoad %int %x
         %48 =       OpSLessThan %bool %46 %int_100
                     OpBranchConditional %48 %43 %45

         %43 =         OpLabel
         %51 =           OpLoad %int %one
                         OpStore %y %51
                         OpBranch %52

         %52 =         OpLabel
                         OpLoopMerge %56 %55 None
                         OpBranch %53

         %53 =             OpLabel
         %57 =               OpLoad %int %y
         %58 =               OpSLessThan %bool %57 %int_100
                             OpBranchConditional %58 %54 %56

         %54 =                 OpLabel
         %60 =                   OpLoad %int %x
                                 OpStore %_0_x %60
                                 OpStore %_1_result %int_0
                                 OpBranch %62

         %62 =                 OpLabel
                                 OpLoopMerge %66 %65 None
                                 OpBranch %63

         %63 =                     OpLabel
         %67 =                       OpLoad %int %_0_x
         %68 =                       OpLoad %int %y
         %69 =                       OpSGreaterThanEqual %bool %67 %68
                                     OpBranchConditional %69 %64 %66

         %64 =                         OpLabel
         %71 =                           OpLoad %int %_1_result
         %72 =                           OpIAdd %int %71 %int_1
                                         OpStore %_1_result %72
         %73 =                           OpLoad %int %_0_x
         %74 =                           OpLoad %int %y
         %75 =                           OpISub %int %73 %74
                                         OpStore %_0_x %75
                                         OpBranch %65

         %65 =                   OpLabel
                                   OpBranch %62

         %66 =                 OpLabel
         %76 =                   OpLoad %int %x
         %77 =                   OpLoad %int %y
         %78 =                   OpSDiv %int %76 %77
         %79 =                   OpLoad %int %_1_result
         %80 =                   OpINotEqual %bool %78 %79
                                 OpSelectionMerge %82 None
                                 OpBranchConditional %80 %81 %82

         %81 =                     OpLabel
         %84 =                       OpLoad %int %x
         %85 =                       OpConvertSToF %float %84
         %87 =                       OpFMul %float %85 %float_0_00392156886
         %88 =                       OpLoad %int %y
         %89 =                       OpConvertSToF %float %88
         %90 =                       OpFMul %float %89 %float_0_00392156886
         %91 =                       OpCompositeConstruct %v4float %float_1 %87 %90 %float_1    ; RelaxedPrecision
                                     OpReturnValue %91

         %82 =                 OpLabel
                                 OpBranch %55

         %55 =           OpLabel
         %92 =             OpLoad %int %y
         %93 =             OpIAdd %int %92 %int_1
                           OpStore %y %93
                           OpBranch %52

         %56 =         OpLabel
                         OpBranch %44

         %44 =   OpLabel
         %94 =     OpLoad %int %x
         %95 =     OpIAdd %int %94 %int_1
                   OpStore %x %95
                   OpBranch %41

         %45 = OpLabel
         %96 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %97 =   OpLoad %v4float %96                ; RelaxedPrecision
                 OpReturnValue %97
               OpFunctionEnd
