               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %result "result"                  ; id %27
               OpName %a "a"                            ; id %29
               OpName %b "b"                            ; id %47
               OpName %c "c"                            ; id %61
               OpName %d "d"                            ; id %72
               OpName %e "e"                            ; id %80
               OpName %f "f"                            ; id %87

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %108 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
         %34 = OpConstantComposite %v3float %float_1 %float_0 %float_0
         %35 = OpConstantComposite %v3float %float_0 %float_1 %float_0
         %36 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
         %38 = OpConstantComposite %mat3v3float %34 %35 %36
      %int_0 = OpConstant %int 0
         %48 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %49 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %51 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
         %53 = OpConstantComposite %mat4v4float %48 %49 %50 %51
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %73 = OpConstantComposite %v2float %float_1 %float_0
         %74 = OpConstantComposite %v2float %float_0 %float_1
         %75 = OpConstantComposite %mat2v2float %73 %74
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_6 = OpConstant %float 6
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1


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
     %result =   OpVariable %_ptr_Function_float Function
          %a =   OpVariable %_ptr_Function_mat2v2float Function
          %b =   OpVariable %_ptr_Function_mat2v2float Function
          %c =   OpVariable %_ptr_Function_mat3v3float Function
          %d =   OpVariable %_ptr_Function_mat3v3float Function
          %e =   OpVariable %_ptr_Function_mat4v4float Function
          %f =   OpVariable %_ptr_Function_mat2v2float Function
        %102 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %result %float_0
         %39 =   OpVectorShuffle %v2float %34 %34 0 1
         %40 =   OpVectorShuffle %v2float %35 %35 0 1
         %41 =   OpCompositeConstruct %mat2v2float %39 %40
                 OpStore %a %41
         %43 =   OpAccessChain %_ptr_Function_v2float %a %int_0
         %44 =   OpLoad %v2float %43
         %45 =   OpCompositeExtract %float %44 0
         %46 =   OpFAdd %float %float_0 %45
                 OpStore %result %46
         %54 =   OpVectorShuffle %v2float %48 %48 0 1
         %55 =   OpVectorShuffle %v2float %49 %49 0 1
         %56 =   OpCompositeConstruct %mat2v2float %54 %55
                 OpStore %b %56
         %57 =   OpAccessChain %_ptr_Function_v2float %b %int_0
         %58 =   OpLoad %v2float %57
         %59 =   OpCompositeExtract %float %58 0
         %60 =   OpFAdd %float %46 %59
                 OpStore %result %60
         %63 =   OpVectorShuffle %v3float %48 %48 0 1 2
         %64 =   OpVectorShuffle %v3float %49 %49 0 1 2
         %65 =   OpVectorShuffle %v3float %50 %50 0 1 2
         %66 =   OpCompositeConstruct %mat3v3float %63 %64 %65
                 OpStore %c %66
         %67 =   OpAccessChain %_ptr_Function_v3float %c %int_0
         %69 =   OpLoad %v3float %67
         %70 =   OpCompositeExtract %float %69 0
         %71 =   OpFAdd %float %60 %70
                 OpStore %result %71
                 OpStore %d %38
         %76 =   OpAccessChain %_ptr_Function_v3float %d %int_0
         %77 =   OpLoad %v3float %76
         %78 =   OpCompositeExtract %float %77 0
         %79 =   OpFAdd %float %71 %78
                 OpStore %result %79
                 OpStore %e %53
         %82 =   OpAccessChain %_ptr_Function_v4float %e %int_0
         %84 =   OpLoad %v4float %82
         %85 =   OpCompositeExtract %float %84 0
         %86 =   OpFAdd %float %79 %85
                 OpStore %result %86
         %88 =   OpVectorShuffle %v3float %48 %48 0 1 2
         %89 =   OpVectorShuffle %v3float %49 %49 0 1 2
         %90 =   OpVectorShuffle %v3float %50 %50 0 1 2
         %91 =   OpCompositeConstruct %mat3v3float %88 %89 %90
         %92 =   OpVectorShuffle %v2float %88 %88 0 1
         %93 =   OpVectorShuffle %v2float %89 %89 0 1
         %94 =   OpCompositeConstruct %mat2v2float %92 %93
                 OpStore %f %94
         %95 =   OpAccessChain %_ptr_Function_v2float %f %int_0
         %96 =   OpLoad %v2float %95
         %97 =   OpCompositeExtract %float %96 0
         %98 =   OpFAdd %float %86 %97
                 OpStore %result %98
        %100 =   OpFOrdEqual %bool %98 %float_6
                 OpSelectionMerge %105 None
                 OpBranchConditional %100 %103 %104

        %103 =     OpLabel
        %106 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %108 =       OpLoad %v4float %106           ; RelaxedPrecision
                     OpStore %102 %108
                     OpBranch %105

        %104 =     OpLabel
        %109 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %111 =       OpLoad %v4float %109           ; RelaxedPrecision
                     OpStore %102 %111
                     OpBranch %105

        %105 = OpLabel
        %112 =   OpLoad %v4float %102               ; RelaxedPrecision
                 OpReturnValue %112
               OpFunctionEnd
