               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %ifElseTest_h4h4h4h4 "ifElseTest_h4h4h4h4"    ; id %6
               OpName %result "result"                              ; id %31
               OpName %main "main"                                  ; id %7

               ; Annotations
               OpDecorate %ifElseTest_h4h4h4h4 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %32 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
         %88 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_1 = OpConstant %float 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function ifElseTest_h4h4h4h4
%ifElseTest_h4h4h4h4 = OpFunction %v4float None %26     ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %30 = OpLabel
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %result %32
         %33 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %37 =   OpLoad %v4float %27                ; RelaxedPrecision
         %38 =   OpFUnordNotEqual %v4bool %36 %37
         %41 =   OpAny %bool %38
                 OpSelectionMerge %44 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
         %45 =       OpLoad %v4float %28            ; RelaxedPrecision
         %46 =       OpLoad %v4float %29            ; RelaxedPrecision
         %47 =       OpFOrdEqual %v4bool %45 %46
         %48 =       OpAll %bool %47
                     OpSelectionMerge %51 None
                     OpBranchConditional %48 %49 %50

         %49 =         OpLabel
         %52 =           OpLoad %v4float %29        ; RelaxedPrecision
                         OpStore %result %52
                         OpBranch %51

         %50 =         OpLabel
         %53 =           OpLoad %v4float %28        ; RelaxedPrecision
                         OpStore %result %53
                         OpBranch %51

         %51 =     OpLabel
                     OpBranch %44

         %43 =     OpLabel
         %54 =       OpLoad %v4float %29            ; RelaxedPrecision
         %55 =       OpLoad %v4float %28            ; RelaxedPrecision
         %56 =       OpFUnordNotEqual %v4bool %54 %55
         %57 =       OpAny %bool %56
                     OpSelectionMerge %60 None
                     OpBranchConditional %57 %58 %59

         %58 =         OpLabel
         %61 =           OpLoad %v4float %27        ; RelaxedPrecision
                         OpStore %result %61
                         OpBranch %60

         %59 =         OpLabel
         %62 =           OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %63 =           OpLoad %v4float %62        ; RelaxedPrecision
                         OpStore %result %63
                         OpBranch %60

         %60 =     OpLabel
                     OpBranch %44

         %44 = OpLabel
         %64 =   OpLoad %v4float %29                ; RelaxedPrecision
         %65 =   OpLoad %v4float %27                ; RelaxedPrecision
         %66 =   OpFOrdEqual %v4bool %64 %65
         %67 =   OpAll %bool %66
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
                     OpReturnValue %71

         %69 = OpLabel
         %72 =   OpLoad %v4float %29                ; RelaxedPrecision
         %73 =   OpLoad %v4float %28                ; RelaxedPrecision
         %74 =   OpFUnordNotEqual %v4bool %72 %73
         %75 =   OpAny %bool %74
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %78 =       OpLoad %v4float %result        ; RelaxedPrecision
                     OpReturnValue %78

         %77 = OpLabel
         %79 =   OpLoad %v4float %29                ; RelaxedPrecision
         %80 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %81 =   OpLoad %v4float %80                ; RelaxedPrecision
         %82 =   OpFOrdEqual %v4bool %79 %81
         %83 =   OpAll %bool %82
                 OpSelectionMerge %85 None
                 OpBranchConditional %83 %84 %85

         %84 =     OpLabel
         %86 =       OpLoad %v4float %27            ; RelaxedPrecision
                     OpReturnValue %86

         %85 = OpLabel
         %87 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpReturnValue %87
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %88         ; RelaxedPrecision
         %89 = OpFunctionParameter %_ptr_Function_v2float

         %90 = OpLabel
         %96 =   OpVariable %_ptr_Function_v4float Function
        %101 =   OpVariable %_ptr_Function_v4float Function
        %106 =   OpVariable %_ptr_Function_v4float Function
         %91 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %92 =   OpLoad %v4float %91                ; RelaxedPrecision
         %93 =   OpCompositeExtract %float %92 2    ; RelaxedPrecision
         %95 =   OpCompositeConstruct %v4float %float_0 %float_0 %93 %float_1   ; RelaxedPrecision
                 OpStore %96 %95
         %97 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %98 =   OpLoad %v4float %97                ; RelaxedPrecision
         %99 =   OpCompositeExtract %float %98 1    ; RelaxedPrecision
        %100 =   OpCompositeConstruct %v4float %float_0 %99 %float_0 %float_1   ; RelaxedPrecision
                 OpStore %101 %100
        %102 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %103 =   OpLoad %v4float %102               ; RelaxedPrecision
        %104 =   OpCompositeExtract %float %103 0   ; RelaxedPrecision
        %105 =   OpCompositeConstruct %v4float %104 %float_0 %float_0 %float_1  ; RelaxedPrecision
                 OpStore %106 %105
        %107 =   OpFunctionCall %v4float %ifElseTest_h4h4h4h4 %96 %101 %106
                 OpReturnValue %107
               OpFunctionEnd
