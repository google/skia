               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %4
               OpName %_UniformBuffer "_UniformBuffer"  ; id %9
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %ifElseTest_h4h4h4h4 "ifElseTest_h4h4h4h4"    ; id %2
               OpName %result "result"                              ; id %27
               OpName %main "main"                                  ; id %3

               ; Annotations
               OpDecorate %ifElseTest_h4h4h4h4 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %22 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
         %85 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_1 = OpConstant %float 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function ifElseTest_h4h4h4h4
%ifElseTest_h4h4h4h4 = OpFunction %v4float None %22     ; RelaxedPrecision
         %23 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %24 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %26 = OpLabel
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %result %28
         %29 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %33 =   OpLoad %v4float %29                ; RelaxedPrecision
         %34 =   OpLoad %v4float %23                ; RelaxedPrecision
         %35 =   OpFUnordNotEqual %v4bool %33 %34
         %38 =   OpAny %bool %35
                 OpSelectionMerge %41 None
                 OpBranchConditional %38 %39 %40

         %39 =     OpLabel
         %42 =       OpLoad %v4float %24            ; RelaxedPrecision
         %43 =       OpLoad %v4float %25            ; RelaxedPrecision
         %44 =       OpFOrdEqual %v4bool %42 %43
         %45 =       OpAll %bool %44
                     OpSelectionMerge %48 None
                     OpBranchConditional %45 %46 %47

         %46 =         OpLabel
         %49 =           OpLoad %v4float %25        ; RelaxedPrecision
                         OpStore %result %49
                         OpBranch %48

         %47 =         OpLabel
         %50 =           OpLoad %v4float %24        ; RelaxedPrecision
                         OpStore %result %50
                         OpBranch %48

         %48 =     OpLabel
                     OpBranch %41

         %40 =     OpLabel
         %51 =       OpLoad %v4float %25            ; RelaxedPrecision
         %52 =       OpLoad %v4float %24            ; RelaxedPrecision
         %53 =       OpFUnordNotEqual %v4bool %51 %52
         %54 =       OpAny %bool %53
                     OpSelectionMerge %57 None
                     OpBranchConditional %54 %55 %56

         %55 =         OpLabel
         %58 =           OpLoad %v4float %23        ; RelaxedPrecision
                         OpStore %result %58
                         OpBranch %57

         %56 =         OpLabel
         %59 =           OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %60 =           OpLoad %v4float %59        ; RelaxedPrecision
                         OpStore %result %60
                         OpBranch %57

         %57 =     OpLabel
                     OpBranch %41

         %41 = OpLabel
         %61 =   OpLoad %v4float %25                ; RelaxedPrecision
         %62 =   OpLoad %v4float %23                ; RelaxedPrecision
         %63 =   OpFOrdEqual %v4bool %61 %62
         %64 =   OpAll %bool %63
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
                     OpReturnValue %68

         %66 = OpLabel
         %69 =   OpLoad %v4float %25                ; RelaxedPrecision
         %70 =   OpLoad %v4float %24                ; RelaxedPrecision
         %71 =   OpFUnordNotEqual %v4bool %69 %70
         %72 =   OpAny %bool %71
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %75 =       OpLoad %v4float %result        ; RelaxedPrecision
                     OpReturnValue %75

         %74 = OpLabel
         %76 =   OpLoad %v4float %25                ; RelaxedPrecision
         %77 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %78 =   OpLoad %v4float %77                ; RelaxedPrecision
         %79 =   OpFOrdEqual %v4bool %76 %78
         %80 =   OpAll %bool %79
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %83 =       OpLoad %v4float %23            ; RelaxedPrecision
                     OpReturnValue %83

         %82 = OpLabel
         %84 =   OpLoad %v4float %25                ; RelaxedPrecision
                 OpReturnValue %84
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %85         ; RelaxedPrecision
         %86 = OpFunctionParameter %_ptr_Function_v2float

         %87 = OpLabel
         %93 =   OpVariable %_ptr_Function_v4float Function
         %98 =   OpVariable %_ptr_Function_v4float Function
        %103 =   OpVariable %_ptr_Function_v4float Function
         %88 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %89 =   OpLoad %v4float %88                ; RelaxedPrecision
         %90 =   OpCompositeExtract %float %89 2    ; RelaxedPrecision
         %92 =   OpCompositeConstruct %v4float %float_0 %float_0 %90 %float_1   ; RelaxedPrecision
                 OpStore %93 %92
         %94 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %95 =   OpLoad %v4float %94                ; RelaxedPrecision
         %96 =   OpCompositeExtract %float %95 1    ; RelaxedPrecision
         %97 =   OpCompositeConstruct %v4float %float_0 %96 %float_0 %float_1   ; RelaxedPrecision
                 OpStore %98 %97
         %99 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %100 =   OpLoad %v4float %99                ; RelaxedPrecision
        %101 =   OpCompositeExtract %float %100 0   ; RelaxedPrecision
        %102 =   OpCompositeConstruct %v4float %101 %float_0 %float_0 %float_1  ; RelaxedPrecision
                 OpStore %103 %102
        %104 =   OpFunctionCall %v4float %ifElseTest_h4h4h4h4 %93 %98 %103
                 OpReturnValue %104
               OpFunctionEnd
