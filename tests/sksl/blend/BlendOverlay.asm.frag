               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %4
               OpName %_UniformBuffer "_UniformBuffer"  ; id %9
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"  ; id %2
               OpName %main "main"                                                      ; id %3
               OpName %_0_result "_0_result"                                            ; id %58

               ; Annotations
               OpDecorate %blend_overlay_component_Qhh2h2 RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
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
               OpDecorate %_0_result RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %13 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
       %void = OpTypeVoid
         %56 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3


               ; Function blend_overlay_component_Qhh2h2
%blend_overlay_component_Qhh2h2 = OpFunction %float None %13    ; RelaxedPrecision
         %14 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision
         %15 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision

         %16 = OpLabel
         %25 =   OpVariable %_ptr_Function_float Function
         %18 =   OpLoad %v2float %15                ; RelaxedPrecision
         %19 =   OpCompositeExtract %float %18 0    ; RelaxedPrecision
         %20 =   OpFMul %float %float_2 %19         ; RelaxedPrecision
         %21 =   OpLoad %v2float %15                ; RelaxedPrecision
         %22 =   OpCompositeExtract %float %21 1    ; RelaxedPrecision
         %23 =   OpFOrdLessThanEqual %bool %20 %22
                 OpSelectionMerge %29 None
                 OpBranchConditional %23 %27 %28

         %27 =     OpLabel
         %30 =       OpLoad %v2float %14            ; RelaxedPrecision
         %31 =       OpCompositeExtract %float %30 0    ; RelaxedPrecision
         %32 =       OpFMul %float %float_2 %31         ; RelaxedPrecision
         %33 =       OpLoad %v2float %15                ; RelaxedPrecision
         %34 =       OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %35 =       OpFMul %float %32 %34              ; RelaxedPrecision
                     OpStore %25 %35
                     OpBranch %29

         %28 =     OpLabel
         %36 =       OpLoad %v2float %14            ; RelaxedPrecision
         %37 =       OpCompositeExtract %float %36 1    ; RelaxedPrecision
         %38 =       OpLoad %v2float %15                ; RelaxedPrecision
         %39 =       OpCompositeExtract %float %38 1    ; RelaxedPrecision
         %40 =       OpFMul %float %37 %39              ; RelaxedPrecision
         %41 =       OpLoad %v2float %15                ; RelaxedPrecision
         %42 =       OpCompositeExtract %float %41 1    ; RelaxedPrecision
         %43 =       OpLoad %v2float %15                ; RelaxedPrecision
         %44 =       OpCompositeExtract %float %43 0    ; RelaxedPrecision
         %45 =       OpFSub %float %42 %44              ; RelaxedPrecision
         %46 =       OpFMul %float %float_2 %45         ; RelaxedPrecision
         %47 =       OpLoad %v2float %14                ; RelaxedPrecision
         %48 =       OpCompositeExtract %float %47 1    ; RelaxedPrecision
         %49 =       OpLoad %v2float %14                ; RelaxedPrecision
         %50 =       OpCompositeExtract %float %49 0    ; RelaxedPrecision
         %51 =       OpFSub %float %48 %50              ; RelaxedPrecision
         %52 =       OpFMul %float %46 %51              ; RelaxedPrecision
         %53 =       OpFSub %float %40 %52              ; RelaxedPrecision
                     OpStore %25 %53
                     OpBranch %29

         %29 = OpLabel
         %54 =   OpLoad %float %25                  ; RelaxedPrecision
                 OpReturnValue %54
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %56

         %57 = OpLabel
  %_0_result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %66 =   OpVariable %_ptr_Function_v2float Function
         %71 =   OpVariable %_ptr_Function_v2float Function
         %76 =   OpVariable %_ptr_Function_v2float Function
         %80 =   OpVariable %_ptr_Function_v2float Function
         %85 =   OpVariable %_ptr_Function_v2float Function
         %89 =   OpVariable %_ptr_Function_v2float Function
         %60 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %64 =   OpLoad %v4float %60                ; RelaxedPrecision
         %65 =   OpVectorShuffle %v2float %64 %64 0 3   ; RelaxedPrecision
                 OpStore %66 %65
         %67 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %69 =   OpLoad %v4float %67                ; RelaxedPrecision
         %70 =   OpVectorShuffle %v2float %69 %69 0 3   ; RelaxedPrecision
                 OpStore %71 %70
         %72 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %66 %71
         %73 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %74 =   OpLoad %v4float %73                ; RelaxedPrecision
         %75 =   OpVectorShuffle %v2float %74 %74 1 3   ; RelaxedPrecision
                 OpStore %76 %75
         %77 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %78 =   OpLoad %v4float %77                ; RelaxedPrecision
         %79 =   OpVectorShuffle %v2float %78 %78 1 3   ; RelaxedPrecision
                 OpStore %80 %79
         %81 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %76 %80
         %82 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %83 =   OpLoad %v4float %82                ; RelaxedPrecision
         %84 =   OpVectorShuffle %v2float %83 %83 2 3   ; RelaxedPrecision
                 OpStore %85 %84
         %86 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %87 =   OpLoad %v4float %86                ; RelaxedPrecision
         %88 =   OpVectorShuffle %v2float %87 %87 2 3   ; RelaxedPrecision
                 OpStore %89 %88
         %90 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %85 %89
         %91 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %92 =   OpLoad %v4float %91                ; RelaxedPrecision
         %93 =   OpCompositeExtract %float %92 3    ; RelaxedPrecision
         %95 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %96 =   OpLoad %v4float %95                ; RelaxedPrecision
         %97 =   OpCompositeExtract %float %96 3    ; RelaxedPrecision
         %98 =   OpFSub %float %float_1 %97         ; RelaxedPrecision
         %99 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %100 =   OpLoad %v4float %99                ; RelaxedPrecision
        %101 =   OpCompositeExtract %float %100 3   ; RelaxedPrecision
        %102 =   OpFMul %float %98 %101             ; RelaxedPrecision
        %103 =   OpFAdd %float %93 %102             ; RelaxedPrecision
        %104 =   OpCompositeConstruct %v4float %72 %81 %90 %103     ; RelaxedPrecision
                 OpStore %_0_result %104
        %105 =   OpLoad %v4float %_0_result         ; RelaxedPrecision
        %106 =   OpVectorShuffle %v3float %105 %105 0 1 2   ; RelaxedPrecision
        %108 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %109 =   OpLoad %v4float %108               ; RelaxedPrecision
        %110 =   OpVectorShuffle %v3float %109 %109 0 1 2   ; RelaxedPrecision
        %111 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %112 =   OpLoad %v4float %111               ; RelaxedPrecision
        %113 =   OpCompositeExtract %float %112 3   ; RelaxedPrecision
        %114 =   OpFSub %float %float_1 %113        ; RelaxedPrecision
        %115 =   OpVectorTimesScalar %v3float %110 %114     ; RelaxedPrecision
        %116 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %117 =   OpLoad %v4float %116               ; RelaxedPrecision
        %118 =   OpVectorShuffle %v3float %117 %117 0 1 2   ; RelaxedPrecision
        %119 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %120 =   OpLoad %v4float %119               ; RelaxedPrecision
        %121 =   OpCompositeExtract %float %120 3   ; RelaxedPrecision
        %122 =   OpFSub %float %float_1 %121        ; RelaxedPrecision
        %123 =   OpVectorTimesScalar %v3float %118 %122     ; RelaxedPrecision
        %124 =   OpFAdd %v3float %115 %123                  ; RelaxedPrecision
        %125 =   OpFAdd %v3float %106 %124                  ; RelaxedPrecision
        %126 =   OpLoad %v4float %_0_result                 ; RelaxedPrecision
        %127 =   OpVectorShuffle %v4float %126 %125 4 5 6 3     ; RelaxedPrecision
                 OpStore %_0_result %127
                 OpStore %sk_FragColor %127
                 OpReturn
               OpFunctionEnd
