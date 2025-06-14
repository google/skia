               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"                  ; id %6
               OpName %i "i"                        ; id %29
               OpName %i_0 "i"                      ; id %57

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
    %float_5 = OpConstant %float 5
       %bool = OpTypeBool
 %float_0_75 = OpConstant %float 0.75
         %28 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%_ptr_Function_int = OpTypePointer Function %int
     %int_10 = OpConstant %int 10
  %float_0_5 = OpConstant %float 0.5
      %int_1 = OpConstant %int 1
 %float_0_25 = OpConstant %float 0.25
         %52 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
      %int_2 = OpConstant %int 2
    %int_100 = OpConstant %int 100


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
        %i_0 =   OpVariable %_ptr_Function_int Function
         %17 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %20 =   OpLoad %float %17
         %22 =   OpFOrdGreaterThan %bool %20 %float_5
                 OpSelectionMerge %26 None
                 OpBranchConditional %22 %24 %25

         %24 =     OpLabel
                     OpStore %sk_FragColor %28
                     OpBranch %26

         %25 =     OpLabel
                     OpKill

         %26 = OpLabel
                 OpStore %i %int_0
                 OpBranch %31

         %31 = OpLabel
                 OpLoopMerge %35 %34 None
                 OpBranch %32

         %32 =     OpLabel
         %36 =       OpLoad %int %i
         %38 =       OpSLessThan %bool %36 %int_10
                     OpBranchConditional %38 %33 %35

         %33 =         OpLabel
         %39 =           OpLoad %v4float %sk_FragColor  ; RelaxedPrecision
         %41 =           OpVectorTimesScalar %v4float %39 %float_0_5    ; RelaxedPrecision
                         OpStore %sk_FragColor %41
         %42 =           OpLoad %int %i
         %44 =           OpIAdd %int %42 %int_1
                         OpStore %i %44
                         OpBranch %34

         %34 =   OpLabel
                   OpBranch %31

         %35 = OpLabel
                 OpBranch %45

         %45 = OpLabel
                 OpLoopMerge %49 %48 None
                 OpBranch %46

         %46 =     OpLabel
         %50 =       OpLoad %v4float %sk_FragColor  ; RelaxedPrecision
         %53 =       OpFAdd %v4float %50 %52        ; RelaxedPrecision
                     OpStore %sk_FragColor %53
                     OpBranch %47

         %47 =     OpLabel
                     OpBranch %48

         %48 =   OpLabel
         %54 =     OpLoad %v4float %sk_FragColor    ; RelaxedPrecision
         %55 =     OpCompositeExtract %float %54 0  ; RelaxedPrecision
         %56 =     OpFOrdLessThan %bool %55 %float_0_75
                   OpBranchConditional %56 %45 %49

         %49 = OpLabel
                 OpStore %i_0 %int_0
                 OpBranch %58

         %58 = OpLabel
                 OpLoopMerge %62 %61 None
                 OpBranch %59

         %59 =     OpLabel
         %63 =       OpLoad %int %i_0
         %64 =       OpSLessThan %bool %63 %int_10
                     OpBranchConditional %64 %60 %62

         %60 =         OpLabel
         %65 =           OpLoad %int %i_0
         %67 =           OpSMod %int %65 %int_2
         %68 =           OpIEqual %bool %67 %int_1
                         OpSelectionMerge %71 None
                         OpBranchConditional %68 %69 %70

         %69 =             OpLabel
                             OpBranch %62

         %70 =             OpLabel
         %72 =               OpLoad %int %i_0
         %74 =               OpSGreaterThan %bool %72 %int_100
                             OpSelectionMerge %77 None
                             OpBranchConditional %74 %75 %76

         %75 =                 OpLabel
                                 OpReturn

         %76 =                 OpLabel
                                 OpBranch %61

         %77 =             OpLabel
                             OpBranch %71

         %71 =         OpLabel
                         OpBranch %61

         %61 =   OpLabel
         %78 =     OpLoad %int %i_0
         %79 =     OpIAdd %int %78 %int_1
                   OpStore %i_0 %79
                   OpBranch %58

         %62 = OpLabel
                 OpReturn
               OpFunctionEnd
