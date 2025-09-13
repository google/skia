               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"  ; id %6
               OpName %main "main"                                                      ; id %7
               OpName %_0_result "_0_result"                                            ; id %62

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
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
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %_0_result RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
       %void = OpTypeVoid
         %60 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3


               ; Function blend_overlay_component_Qhh2h2
%blend_overlay_component_Qhh2h2 = OpFunction %float None %17    ; RelaxedPrecision
         %18 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision
         %19 = OpFunctionParameter %_ptr_Function_v2float       ; RelaxedPrecision

         %20 = OpLabel
         %29 =   OpVariable %_ptr_Function_float Function
         %22 =   OpLoad %v2float %19                ; RelaxedPrecision
         %23 =   OpCompositeExtract %float %22 0    ; RelaxedPrecision
         %24 =   OpFMul %float %float_2 %23         ; RelaxedPrecision
         %25 =   OpLoad %v2float %19                ; RelaxedPrecision
         %26 =   OpCompositeExtract %float %25 1    ; RelaxedPrecision
         %27 =   OpFOrdLessThanEqual %bool %24 %26
                 OpSelectionMerge %33 None
                 OpBranchConditional %27 %31 %32

         %31 =     OpLabel
         %34 =       OpLoad %v2float %18            ; RelaxedPrecision
         %35 =       OpCompositeExtract %float %34 0    ; RelaxedPrecision
         %36 =       OpFMul %float %float_2 %35         ; RelaxedPrecision
         %37 =       OpLoad %v2float %19                ; RelaxedPrecision
         %38 =       OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =       OpFMul %float %36 %38              ; RelaxedPrecision
                     OpStore %29 %39
                     OpBranch %33

         %32 =     OpLabel
         %40 =       OpLoad %v2float %18            ; RelaxedPrecision
         %41 =       OpCompositeExtract %float %40 1    ; RelaxedPrecision
         %42 =       OpLoad %v2float %19                ; RelaxedPrecision
         %43 =       OpCompositeExtract %float %42 1    ; RelaxedPrecision
         %44 =       OpFMul %float %41 %43              ; RelaxedPrecision
         %45 =       OpLoad %v2float %19                ; RelaxedPrecision
         %46 =       OpCompositeExtract %float %45 1    ; RelaxedPrecision
         %47 =       OpLoad %v2float %19                ; RelaxedPrecision
         %48 =       OpCompositeExtract %float %47 0    ; RelaxedPrecision
         %49 =       OpFSub %float %46 %48              ; RelaxedPrecision
         %50 =       OpFMul %float %float_2 %49         ; RelaxedPrecision
         %51 =       OpLoad %v2float %18                ; RelaxedPrecision
         %52 =       OpCompositeExtract %float %51 1    ; RelaxedPrecision
         %53 =       OpLoad %v2float %18                ; RelaxedPrecision
         %54 =       OpCompositeExtract %float %53 0    ; RelaxedPrecision
         %55 =       OpFSub %float %52 %54              ; RelaxedPrecision
         %56 =       OpFMul %float %50 %55              ; RelaxedPrecision
         %57 =       OpFSub %float %44 %56              ; RelaxedPrecision
                     OpStore %29 %57
                     OpBranch %33

         %33 = OpLabel
         %58 =   OpLoad %float %29                  ; RelaxedPrecision
                 OpReturnValue %58
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %60

         %61 = OpLabel
  %_0_result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %69 =   OpVariable %_ptr_Function_v2float Function
         %74 =   OpVariable %_ptr_Function_v2float Function
         %79 =   OpVariable %_ptr_Function_v2float Function
         %83 =   OpVariable %_ptr_Function_v2float Function
         %88 =   OpVariable %_ptr_Function_v2float Function
         %92 =   OpVariable %_ptr_Function_v2float Function
         %64 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %67 =   OpLoad %v4float %64                ; RelaxedPrecision
         %68 =   OpVectorShuffle %v2float %67 %67 0 3   ; RelaxedPrecision
                 OpStore %69 %68
         %70 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %72 =   OpLoad %v4float %70                ; RelaxedPrecision
         %73 =   OpVectorShuffle %v2float %72 %72 0 3   ; RelaxedPrecision
                 OpStore %74 %73
         %75 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %69 %74
         %76 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %77 =   OpLoad %v4float %76                ; RelaxedPrecision
         %78 =   OpVectorShuffle %v2float %77 %77 1 3   ; RelaxedPrecision
                 OpStore %79 %78
         %80 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %81 =   OpLoad %v4float %80                ; RelaxedPrecision
         %82 =   OpVectorShuffle %v2float %81 %81 1 3   ; RelaxedPrecision
                 OpStore %83 %82
         %84 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %79 %83
         %85 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %86 =   OpLoad %v4float %85                ; RelaxedPrecision
         %87 =   OpVectorShuffle %v2float %86 %86 2 3   ; RelaxedPrecision
                 OpStore %88 %87
         %89 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %90 =   OpLoad %v4float %89                ; RelaxedPrecision
         %91 =   OpVectorShuffle %v2float %90 %90 2 3   ; RelaxedPrecision
                 OpStore %92 %91
         %93 =   OpFunctionCall %float %blend_overlay_component_Qhh2h2 %88 %92
         %94 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %95 =   OpLoad %v4float %94                ; RelaxedPrecision
         %96 =   OpCompositeExtract %float %95 3    ; RelaxedPrecision
         %98 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %99 =   OpLoad %v4float %98                ; RelaxedPrecision
        %100 =   OpCompositeExtract %float %99 3    ; RelaxedPrecision
        %101 =   OpFSub %float %float_1 %100        ; RelaxedPrecision
        %102 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %103 =   OpLoad %v4float %102               ; RelaxedPrecision
        %104 =   OpCompositeExtract %float %103 3   ; RelaxedPrecision
        %105 =   OpFMul %float %101 %104            ; RelaxedPrecision
        %106 =   OpFAdd %float %96 %105             ; RelaxedPrecision
        %107 =   OpCompositeConstruct %v4float %75 %84 %93 %106     ; RelaxedPrecision
                 OpStore %_0_result %107
        %108 =   OpLoad %v4float %_0_result         ; RelaxedPrecision
        %109 =   OpVectorShuffle %v3float %108 %108 0 1 2   ; RelaxedPrecision
        %111 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %112 =   OpLoad %v4float %111               ; RelaxedPrecision
        %113 =   OpVectorShuffle %v3float %112 %112 0 1 2   ; RelaxedPrecision
        %114 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %115 =   OpLoad %v4float %114               ; RelaxedPrecision
        %116 =   OpCompositeExtract %float %115 3   ; RelaxedPrecision
        %117 =   OpFSub %float %float_1 %116        ; RelaxedPrecision
        %118 =   OpVectorTimesScalar %v3float %113 %117     ; RelaxedPrecision
        %119 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %120 =   OpLoad %v4float %119               ; RelaxedPrecision
        %121 =   OpVectorShuffle %v3float %120 %120 0 1 2   ; RelaxedPrecision
        %122 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %123 =   OpLoad %v4float %122               ; RelaxedPrecision
        %124 =   OpCompositeExtract %float %123 3   ; RelaxedPrecision
        %125 =   OpFSub %float %float_1 %124        ; RelaxedPrecision
        %126 =   OpVectorTimesScalar %v3float %121 %125     ; RelaxedPrecision
        %127 =   OpFAdd %v3float %118 %126                  ; RelaxedPrecision
        %128 =   OpFAdd %v3float %109 %127                  ; RelaxedPrecision
        %129 =   OpLoad %v4float %_0_result                 ; RelaxedPrecision
        %130 =   OpVectorShuffle %v4float %129 %128 4 5 6 3     ; RelaxedPrecision
                 OpStore %_0_result %130
                 OpStore %sk_FragColor %130
                 OpReturn
               OpFunctionEnd
