               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"  ; id %6
               OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"                      ; id %7
               OpName %result "result"                                                  ; id %65
               OpName %main "main"                                                      ; id %8

               ; Annotations
               OpDecorate %blend_overlay_component_Qhh2h2 RelaxedPrecision
               OpDecorate %blend_overlay_h4h4h4 RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %18 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %61 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
       %void = OpTypeVoid
        %118 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0


               ; Function blend_overlay_component_Qhh2h2
%blend_overlay_component_Qhh2h2 = OpFunction %float None %18    ; RelaxedPrecision
         %19 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision
         %20 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision

         %21 = OpLabel
         %30 =   OpVariable %_ptr_Function_float Function
         %23 =   OpLoad %v2float %20                ; RelaxedPrecision
         %24 =   OpCompositeExtract %float %23 0    ; RelaxedPrecision
         %25 =   OpFMul %float %float_2 %24         ; RelaxedPrecision
         %26 =   OpLoad %v2float %20                ; RelaxedPrecision
         %27 =   OpCompositeExtract %float %26 1    ; RelaxedPrecision
         %28 =   OpFOrdLessThanEqual %bool %25 %27
                 OpSelectionMerge %34 None
                 OpBranchConditional %28 %32 %33

         %32 =     OpLabel
         %35 =       OpLoad %v2float %19            ; RelaxedPrecision
         %36 =       OpCompositeExtract %float %35 0    ; RelaxedPrecision
         %37 =       OpFMul %float %float_2 %36         ; RelaxedPrecision
         %38 =       OpLoad %v2float %20                ; RelaxedPrecision
         %39 =       OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %40 =       OpFMul %float %37 %39              ; RelaxedPrecision
                     OpStore %30 %40
                     OpBranch %34

         %33 =     OpLabel
         %41 =       OpLoad %v2float %19            ; RelaxedPrecision
         %42 =       OpCompositeExtract %float %41 1    ; RelaxedPrecision
         %43 =       OpLoad %v2float %20                ; RelaxedPrecision
         %44 =       OpCompositeExtract %float %43 1    ; RelaxedPrecision
         %45 =       OpFMul %float %42 %44              ; RelaxedPrecision
         %46 =       OpLoad %v2float %20                ; RelaxedPrecision
         %47 =       OpCompositeExtract %float %46 1    ; RelaxedPrecision
         %48 =       OpLoad %v2float %20                ; RelaxedPrecision
         %49 =       OpCompositeExtract %float %48 0    ; RelaxedPrecision
         %50 =       OpFSub %float %47 %49              ; RelaxedPrecision
         %51 =       OpFMul %float %float_2 %50         ; RelaxedPrecision
         %52 =       OpLoad %v2float %19                ; RelaxedPrecision
         %53 =       OpCompositeExtract %float %52 1    ; RelaxedPrecision
         %54 =       OpLoad %v2float %19                ; RelaxedPrecision
         %55 =       OpCompositeExtract %float %54 0    ; RelaxedPrecision
         %56 =       OpFSub %float %53 %55              ; RelaxedPrecision
         %57 =       OpFMul %float %51 %56              ; RelaxedPrecision
         %58 =       OpFSub %float %45 %57              ; RelaxedPrecision
                     OpStore %30 %58
                     OpBranch %34

         %34 = OpLabel
         %59 =   OpLoad %float %30                  ; RelaxedPrecision
                 OpReturnValue %59
               OpFunctionEnd


               ; Function blend_overlay_h4h4h4
%blend_overlay_h4h4h4 = OpFunction %v4float None %61    ; RelaxedPrecision
         %62 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %63 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %64 = OpLabel
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %68 =   OpVariable %_ptr_Function_v2float Function
         %71 =   OpVariable %_ptr_Function_v2float Function
         %75 =   OpVariable %_ptr_Function_v2float Function
         %78 =   OpVariable %_ptr_Function_v2float Function
         %82 =   OpVariable %_ptr_Function_v2float Function
         %85 =   OpVariable %_ptr_Function_v2float Function
         %66 =   OpLoad %v4float %62                ; RelaxedPrecision
         %67 =   OpVectorShuffle %v2float %66 %66 0 3   ; RelaxedPrecision
                 OpStore %68 %67
         %69 =   OpLoad %v4float %63                ; RelaxedPrecision
         %70 =   OpVectorShuffle %v2float %69 %69 0 3   ; RelaxedPrecision
                 OpStore %71 %70
         %72 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %68 %71
         %73 =   OpLoad %v4float %62                ; RelaxedPrecision
         %74 =   OpVectorShuffle %v2float %73 %73 1 3   ; RelaxedPrecision
                 OpStore %75 %74
         %76 =   OpLoad %v4float %63                ; RelaxedPrecision
         %77 =   OpVectorShuffle %v2float %76 %76 1 3   ; RelaxedPrecision
                 OpStore %78 %77
         %79 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %75 %78
         %80 =   OpLoad %v4float %62                ; RelaxedPrecision
         %81 =   OpVectorShuffle %v2float %80 %80 2 3   ; RelaxedPrecision
                 OpStore %82 %81
         %83 =   OpLoad %v4float %63                ; RelaxedPrecision
         %84 =   OpVectorShuffle %v2float %83 %83 2 3   ; RelaxedPrecision
                 OpStore %85 %84
         %86 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %82 %85
         %87 =   OpLoad %v4float %62                ; RelaxedPrecision
         %88 =   OpCompositeExtract %float %87 3    ; RelaxedPrecision
         %90 =   OpLoad %v4float %62                ; RelaxedPrecision
         %91 =   OpCompositeExtract %float %90 3    ; RelaxedPrecision
         %92 =   OpFSub %float %float_1 %91         ; RelaxedPrecision
         %93 =   OpLoad %v4float %63                ; RelaxedPrecision
         %94 =   OpCompositeExtract %float %93 3    ; RelaxedPrecision
         %95 =   OpFMul %float %92 %94              ; RelaxedPrecision
         %96 =   OpFAdd %float %88 %95              ; RelaxedPrecision
         %97 =   OpCompositeConstruct %v4float %72 %79 %86 %96  ; RelaxedPrecision
                 OpStore %result %97
         %98 =   OpLoad %v4float %result            ; RelaxedPrecision
         %99 =   OpVectorShuffle %v3float %98 %98 0 1 2     ; RelaxedPrecision
        %101 =   OpLoad %v4float %63                        ; RelaxedPrecision
        %102 =   OpVectorShuffle %v3float %101 %101 0 1 2   ; RelaxedPrecision
        %103 =   OpLoad %v4float %62                        ; RelaxedPrecision
        %104 =   OpCompositeExtract %float %103 3           ; RelaxedPrecision
        %105 =   OpFSub %float %float_1 %104                ; RelaxedPrecision
        %106 =   OpVectorTimesScalar %v3float %102 %105     ; RelaxedPrecision
        %107 =   OpLoad %v4float %62                        ; RelaxedPrecision
        %108 =   OpVectorShuffle %v3float %107 %107 0 1 2   ; RelaxedPrecision
        %109 =   OpLoad %v4float %63                        ; RelaxedPrecision
        %110 =   OpCompositeExtract %float %109 3           ; RelaxedPrecision
        %111 =   OpFSub %float %float_1 %110                ; RelaxedPrecision
        %112 =   OpVectorTimesScalar %v3float %108 %111     ; RelaxedPrecision
        %113 =   OpFAdd %v3float %106 %112                  ; RelaxedPrecision
        %114 =   OpFAdd %v3float %99 %113                   ; RelaxedPrecision
        %115 =   OpLoad %v4float %result                    ; RelaxedPrecision
        %116 =   OpVectorShuffle %v4float %115 %114 4 5 6 3     ; RelaxedPrecision
                 OpStore %result %116
                 OpReturnValue %116
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %118

        %119 = OpLabel
        %124 =   OpVariable %_ptr_Function_v4float Function
        %128 =   OpVariable %_ptr_Function_v4float Function
        %120 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %123 =   OpLoad %v4float %120               ; RelaxedPrecision
                 OpStore %124 %123
        %125 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %127 =   OpLoad %v4float %125               ; RelaxedPrecision
                 OpStore %128 %127
        %129 =   OpFunctionCall %v4float %blend_overlay_h4h4h4 %124 %128
                 OpStore %sk_FragColor %129
                 OpReturn
               OpFunctionEnd
