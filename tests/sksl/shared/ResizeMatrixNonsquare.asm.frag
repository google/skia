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
               OpName %g "g"                            ; id %29
               OpName %h "h"                            ; id %46
               OpName %i "i"                            ; id %55
               OpName %j "j"                            ; id %73
               OpName %k "k"                            ; id %82
               OpName %l "l"                            ; id %88

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
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision

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
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v3float %float_1 %float_0 %float_0
         %35 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
         %37 = OpConstantComposite %mat2v3float %34 %35
         %38 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %39 = OpConstantComposite %mat3v3float %34 %35 %38
      %int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %47 = OpConstantComposite %v2float %float_1 %float_0
         %48 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
         %50 = OpConstantComposite %mat3v2float %47 %48 %20
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
         %59 = OpConstantComposite %mat4v2float %47 %48 %20 %20
         %60 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat4v3float = OpTypeMatrix %v3float 4
         %62 = OpConstantComposite %mat4v3float %34 %35 %38 %60
         %63 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %64 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %65 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %66 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %67 = OpConstantComposite %mat4v4float %63 %64 %65 %66
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
         %75 = OpConstantComposite %mat2v4float %63 %64
%mat3v4float = OpTypeMatrix %v4float 3
         %77 = OpConstantComposite %mat3v4float %63 %64 %65
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
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
          %g =   OpVariable %_ptr_Function_mat3v3float Function
          %h =   OpVariable %_ptr_Function_mat3v3float Function
          %i =   OpVariable %_ptr_Function_mat4v4float Function
          %j =   OpVariable %_ptr_Function_mat4v4float Function
          %k =   OpVariable %_ptr_Function_mat2v4float Function
          %l =   OpVariable %_ptr_Function_mat4v2float Function
        %100 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %result %float_0
                 OpStore %g %39
         %41 =   OpAccessChain %_ptr_Function_v3float %g %int_0
         %43 =   OpLoad %v3float %41
         %44 =   OpCompositeExtract %float %43 0
         %45 =   OpFAdd %float %float_0 %44
                 OpStore %result %45
                 OpStore %h %39
         %51 =   OpAccessChain %_ptr_Function_v3float %h %int_0
         %52 =   OpLoad %v3float %51
         %53 =   OpCompositeExtract %float %52 0
         %54 =   OpFAdd %float %45 %53
                 OpStore %result %54
                 OpStore %i %67
         %68 =   OpAccessChain %_ptr_Function_v4float %i %int_0
         %70 =   OpLoad %v4float %68
         %71 =   OpCompositeExtract %float %70 0
         %72 =   OpFAdd %float %54 %71
                 OpStore %result %72
                 OpStore %j %67
         %78 =   OpAccessChain %_ptr_Function_v4float %j %int_0
         %79 =   OpLoad %v4float %78
         %80 =   OpCompositeExtract %float %79 0
         %81 =   OpFAdd %float %72 %80
                 OpStore %result %81
                 OpStore %k %75
         %84 =   OpAccessChain %_ptr_Function_v4float %k %int_0
         %85 =   OpLoad %v4float %84
         %86 =   OpCompositeExtract %float %85 0
         %87 =   OpFAdd %float %81 %86
                 OpStore %result %87
         %90 =   OpVectorShuffle %v2float %63 %63 0 1
         %91 =   OpVectorShuffle %v2float %64 %64 0 1
         %92 =   OpCompositeConstruct %mat4v2float %90 %91 %20 %20
                 OpStore %l %92
         %93 =   OpAccessChain %_ptr_Function_v2float %l %int_0
         %94 =   OpLoad %v2float %93
         %95 =   OpCompositeExtract %float %94 0
         %96 =   OpFAdd %float %87 %95
                 OpStore %result %96
         %98 =   OpFOrdEqual %bool %96 %float_6
                 OpSelectionMerge %103 None
                 OpBranchConditional %98 %101 %102

        %101 =     OpLabel
        %104 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %106 =       OpLoad %v4float %104           ; RelaxedPrecision
                     OpStore %100 %106
                     OpBranch %103

        %102 =     OpLabel
        %107 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %109 =       OpLoad %v4float %107           ; RelaxedPrecision
                     OpStore %100 %109
                     OpBranch %103

        %103 = OpLabel
        %110 =   OpLoad %v4float %100               ; RelaxedPrecision
                 OpReturnValue %110
               OpFunctionEnd
