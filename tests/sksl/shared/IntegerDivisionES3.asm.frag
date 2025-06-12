               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %zero "zero"                      ; id %23
               OpName %one "one"                        ; id %32
               OpName %x "x"                            ; id %37
               OpName %y "y"                            ; id %47
               OpName %_0_x "_0_x"                      ; id %56
               OpName %_1_result "_1_result"            ; id %58

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
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
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %int_100 = OpConstant %int 100
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886


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
       %zero =   OpVariable %_ptr_Function_int Function
        %one =   OpVariable %_ptr_Function_int Function
          %x =   OpVariable %_ptr_Function_int Function
          %y =   OpVariable %_ptr_Function_int Function
       %_0_x =   OpVariable %_ptr_Function_int Function
  %_1_result =   OpVariable %_ptr_Function_int Function
         %26 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 =   OpLoad %v4float %26                ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 0    ; RelaxedPrecision
         %31 =   OpConvertFToS %int %30
                 OpStore %zero %31
         %33 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 =   OpLoad %v4float %33                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 1    ; RelaxedPrecision
         %36 =   OpConvertFToS %int %35
                 OpStore %one %36
                 OpStore %x %31
                 OpBranch %38

         %38 = OpLabel
                 OpLoopMerge %42 %41 None
                 OpBranch %39

         %39 =     OpLabel
         %43 =       OpLoad %int %x
         %45 =       OpSLessThan %bool %43 %int_100
                     OpBranchConditional %45 %40 %42

         %40 =         OpLabel
         %48 =           OpLoad %int %one
                         OpStore %y %48
                         OpBranch %49

         %49 =         OpLabel
                         OpLoopMerge %53 %52 None
                         OpBranch %50

         %50 =             OpLabel
         %54 =               OpLoad %int %y
         %55 =               OpSLessThan %bool %54 %int_100
                             OpBranchConditional %55 %51 %53

         %51 =                 OpLabel
         %57 =                   OpLoad %int %x
                                 OpStore %_0_x %57
                                 OpStore %_1_result %int_0
                                 OpBranch %59

         %59 =                 OpLabel
                                 OpLoopMerge %63 %62 None
                                 OpBranch %60

         %60 =                     OpLabel
         %64 =                       OpLoad %int %_0_x
         %65 =                       OpLoad %int %y
         %66 =                       OpSGreaterThanEqual %bool %64 %65
                                     OpBranchConditional %66 %61 %63

         %61 =                         OpLabel
         %68 =                           OpLoad %int %_1_result
         %69 =                           OpIAdd %int %68 %int_1
                                         OpStore %_1_result %69
         %70 =                           OpLoad %int %_0_x
         %71 =                           OpLoad %int %y
         %72 =                           OpISub %int %70 %71
                                         OpStore %_0_x %72
                                         OpBranch %62

         %62 =                   OpLabel
                                   OpBranch %59

         %63 =                 OpLabel
         %73 =                   OpLoad %int %x
         %74 =                   OpLoad %int %y
         %75 =                   OpSDiv %int %73 %74
         %76 =                   OpLoad %int %_1_result
         %77 =                   OpINotEqual %bool %75 %76
                                 OpSelectionMerge %79 None
                                 OpBranchConditional %77 %78 %79

         %78 =                     OpLabel
         %81 =                       OpLoad %int %x
         %82 =                       OpConvertSToF %float %81
         %84 =                       OpFMul %float %82 %float_0_00392156886
         %85 =                       OpLoad %int %y
         %86 =                       OpConvertSToF %float %85
         %87 =                       OpFMul %float %86 %float_0_00392156886
         %88 =                       OpCompositeConstruct %v4float %float_1 %84 %87 %float_1    ; RelaxedPrecision
                                     OpReturnValue %88

         %79 =                 OpLabel
                                 OpBranch %52

         %52 =           OpLabel
         %89 =             OpLoad %int %y
         %90 =             OpIAdd %int %89 %int_1
                           OpStore %y %90
                           OpBranch %49

         %53 =         OpLabel
                         OpBranch %41

         %41 =   OpLabel
         %91 =     OpLoad %int %x
         %92 =     OpIAdd %int %91 %int_1
                   OpStore %x %92
                   OpBranch %38

         %42 = OpLabel
         %93 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %94 =   OpLoad %v4float %93                ; RelaxedPrecision
                 OpReturnValue %94
               OpFunctionEnd
