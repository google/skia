               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %9
               OpMemberName %_UniformBuffer 0 "testMatrix4x4"
               OpMemberName %_UniformBuffer 1 "testValues"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %2
               OpName %m44 "m44"                        ; id %18
               OpName %v4 "v4"                          ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 64
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %m44 RelaxedPrecision
               OpDecorate %v4 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
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
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpTypeFunction %v4float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_123 = OpConstant %float 123
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v4float %float_123 %float_0 %float_0 %float_0
         %23 = OpConstantComposite %v4float %float_0 %float_123 %float_0 %float_0
         %24 = OpConstantComposite %v4float %float_0 %float_0 %float_123 %float_0
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_123
         %26 = OpConstantComposite %mat4v4float %22 %23 %24 %25
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %32 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %15 =   OpFunctionCall %v4float %main
                 OpStore %sk_FragColor %15
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %16         ; RelaxedPrecision

         %17 = OpLabel
        %m44 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
         %v4 =   OpVariable %_ptr_Function_v4float Function         ; RelaxedPrecision
                 OpStore %m44 %26
                 OpStore %v4 %32
         %33 =   OpVectorTimesScalar %v4float %22 %float_0  ; RelaxedPrecision
         %34 =   OpVectorTimesScalar %v4float %23 %float_1  ; RelaxedPrecision
         %35 =   OpFAdd %v4float %33 %34                    ; RelaxedPrecision
         %36 =   OpVectorTimesScalar %v4float %24 %float_2  ; RelaxedPrecision
         %37 =   OpFAdd %v4float %35 %36                    ; RelaxedPrecision
         %38 =   OpVectorTimesScalar %v4float %25 %float_3  ; RelaxedPrecision
         %39 =   OpFAdd %v4float %37 %38                    ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %44 =   OpLoad %mat4v4float %40            ; RelaxedPrecision
         %45 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %48 =   OpLoad %v4float %45                ; RelaxedPrecision
         %49 =   OpCompositeExtract %v4float %44 0  ; RelaxedPrecision
         %50 =   OpCompositeExtract %float %48 0    ; RelaxedPrecision
         %51 =   OpVectorTimesScalar %v4float %49 %50   ; RelaxedPrecision
         %52 =   OpCompositeExtract %v4float %44 1      ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %48 1        ; RelaxedPrecision
         %54 =   OpVectorTimesScalar %v4float %52 %53   ; RelaxedPrecision
         %55 =   OpFAdd %v4float %51 %54                ; RelaxedPrecision
         %56 =   OpCompositeExtract %v4float %44 2      ; RelaxedPrecision
         %57 =   OpCompositeExtract %float %48 2        ; RelaxedPrecision
         %58 =   OpVectorTimesScalar %v4float %56 %57   ; RelaxedPrecision
         %59 =   OpFAdd %v4float %55 %58                ; RelaxedPrecision
         %60 =   OpCompositeExtract %v4float %44 3      ; RelaxedPrecision
         %61 =   OpCompositeExtract %float %48 3        ; RelaxedPrecision
         %62 =   OpVectorTimesScalar %v4float %60 %61   ; RelaxedPrecision
         %63 =   OpFAdd %v4float %59 %62                ; RelaxedPrecision
         %64 =   OpFAdd %v4float %39 %63                ; RelaxedPrecision
                 OpReturnValue %64
               OpFunctionEnd
