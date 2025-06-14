               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %colorBlue "colorBlue"            ; id %27
               OpName %colorGreen "colorGreen"          ; id %37
               OpName %colorRed "colorRed"              ; id %45
               OpName %result "result"                  ; id %53

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
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
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
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4


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
  %colorBlue =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %colorGreen =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %colorRed =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %60 =   OpVariable %_ptr_Function_v4float Function
         %66 =   OpVariable %_ptr_Function_v4float Function
         %73 =   OpVariable %_ptr_Function_v4float Function
         %83 =   OpVariable %_ptr_Function_v4float Function
         %91 =   OpVariable %_ptr_Function_v4float Function
         %99 =   OpVariable %_ptr_Function_v4float Function
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpVectorShuffle %v2float %32 %32 2 3   ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0        ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %33 1        ; RelaxedPrecision
         %36 =   OpCompositeConstruct %v4float %float_0 %float_0 %34 %35    ; RelaxedPrecision
                 OpStore %colorBlue %36
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %38                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 1    ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 =   OpLoad %v4float %41                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 3    ; RelaxedPrecision
         %44 =   OpCompositeConstruct %v4float %float_0 %40 %float_0 %43    ; RelaxedPrecision
                 OpStore %colorGreen %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 0    ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 3    ; RelaxedPrecision
         %52 =   OpCompositeConstruct %v4float %48 %float_0 %float_0 %51    ; RelaxedPrecision
                 OpStore %colorRed %52
         %54 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %55 =   OpLoad %v4float %54                ; RelaxedPrecision
         %56 =   OpFUnordNotEqual %v4bool %55 %36
         %59 =   OpAny %bool %56
                 OpSelectionMerge %63 None
                 OpBranchConditional %59 %61 %62

         %61 =     OpLabel
         %64 =       OpFOrdEqual %v4bool %44 %52
         %65 =       OpAll %bool %64
                     OpSelectionMerge %69 None
                     OpBranchConditional %65 %67 %68

         %67 =         OpLabel
                         OpStore %66 %52
                         OpBranch %69

         %68 =         OpLabel
                         OpStore %66 %44
                         OpBranch %69

         %69 =     OpLabel
         %70 =       OpLoad %v4float %66            ; RelaxedPrecision
                     OpStore %60 %70
                     OpBranch %63

         %62 =     OpLabel
         %71 =       OpFUnordNotEqual %v4bool %52 %44
         %72 =       OpAny %bool %71
                     OpSelectionMerge %76 None
                     OpBranchConditional %72 %74 %75

         %74 =         OpLabel
                         OpStore %73 %36
                         OpBranch %76

         %75 =         OpLabel
         %77 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %78 =           OpLoad %v4float %77        ; RelaxedPrecision
                         OpStore %73 %78
                         OpBranch %76

         %76 =     OpLabel
         %79 =       OpLoad %v4float %73            ; RelaxedPrecision
                     OpStore %60 %79
                     OpBranch %63

         %63 = OpLabel
         %80 =   OpLoad %v4float %60                ; RelaxedPrecision
                 OpStore %result %80
         %81 =   OpFOrdEqual %v4bool %52 %36
         %82 =   OpAll %bool %81
                 OpSelectionMerge %86 None
                 OpBranchConditional %82 %84 %85

         %84 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %83 %88
                     OpBranch %86

         %85 =     OpLabel
         %89 =       OpFUnordNotEqual %v4bool %52 %44
         %90 =       OpAny %bool %89
                     OpSelectionMerge %94 None
                     OpBranchConditional %90 %92 %93

         %92 =         OpLabel
                         OpStore %91 %80
                         OpBranch %94

         %93 =         OpLabel
         %95 =           OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %96 =           OpLoad %v4float %95        ; RelaxedPrecision
         %97 =           OpFOrdEqual %v4bool %52 %96
         %98 =           OpAll %bool %97
                         OpSelectionMerge %102 None
                         OpBranchConditional %98 %100 %101

        %100 =             OpLabel
                             OpStore %99 %36
                             OpBranch %102

        %101 =             OpLabel
                             OpStore %99 %52
                             OpBranch %102

        %102 =         OpLabel
        %103 =           OpLoad %v4float %99        ; RelaxedPrecision
                         OpStore %91 %103
                         OpBranch %94

         %94 =     OpLabel
        %104 =       OpLoad %v4float %91            ; RelaxedPrecision
                     OpStore %83 %104
                     OpBranch %86

         %86 = OpLabel
        %105 =   OpLoad %v4float %83                ; RelaxedPrecision
                 OpReturnValue %105
               OpFunctionEnd
