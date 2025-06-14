               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "testMatrix4x4"
               OpMemberName %_UniformBuffer 1 "testValues"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %main "main"                      ; id %6
               OpName %m44 "m44"                        ; id %22
               OpName %v4 "v4"                          ; id %31

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %m44 RelaxedPrecision
               OpDecorate %v4 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
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
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
         %20 = OpTypeFunction %v4float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_123 = OpConstant %float 123
    %float_0 = OpConstant %float 0
         %26 = OpConstantComposite %v4float %float_123 %float_0 %float_0 %float_0
         %27 = OpConstantComposite %v4float %float_0 %float_123 %float_0 %float_0
         %28 = OpConstantComposite %v4float %float_0 %float_0 %float_123 %float_0
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_123
         %30 = OpConstantComposite %mat4v4float %26 %27 %28 %29
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %36 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %19 =   OpFunctionCall %v4float %main
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision

         %21 = OpLabel
        %m44 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
         %v4 =   OpVariable %_ptr_Function_v4float Function         ; RelaxedPrecision
                 OpStore %m44 %30
                 OpStore %v4 %36
         %37 =   OpVectorTimesScalar %v4float %26 %float_0  ; RelaxedPrecision
         %38 =   OpVectorTimesScalar %v4float %27 %float_1  ; RelaxedPrecision
         %39 =   OpFAdd %v4float %37 %38                    ; RelaxedPrecision
         %40 =   OpVectorTimesScalar %v4float %28 %float_2  ; RelaxedPrecision
         %41 =   OpFAdd %v4float %39 %40                    ; RelaxedPrecision
         %42 =   OpVectorTimesScalar %v4float %29 %float_3  ; RelaxedPrecision
         %43 =   OpFAdd %v4float %41 %42                    ; RelaxedPrecision
         %44 =   OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_0
         %47 =   OpLoad %mat4v4float %44            ; RelaxedPrecision
         %48 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %51 =   OpLoad %v4float %48                ; RelaxedPrecision
         %52 =   OpCompositeExtract %v4float %47 0  ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %51 0    ; RelaxedPrecision
         %54 =   OpVectorTimesScalar %v4float %52 %53   ; RelaxedPrecision
         %55 =   OpCompositeExtract %v4float %47 1      ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %51 1        ; RelaxedPrecision
         %57 =   OpVectorTimesScalar %v4float %55 %56   ; RelaxedPrecision
         %58 =   OpFAdd %v4float %54 %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %v4float %47 2      ; RelaxedPrecision
         %60 =   OpCompositeExtract %float %51 2        ; RelaxedPrecision
         %61 =   OpVectorTimesScalar %v4float %59 %60   ; RelaxedPrecision
         %62 =   OpFAdd %v4float %58 %61                ; RelaxedPrecision
         %63 =   OpCompositeExtract %v4float %47 3      ; RelaxedPrecision
         %64 =   OpCompositeExtract %float %51 3        ; RelaxedPrecision
         %65 =   OpVectorTimesScalar %v4float %63 %64   ; RelaxedPrecision
         %66 =   OpFAdd %v4float %62 %65                ; RelaxedPrecision
         %67 =   OpFAdd %v4float %43 %66                ; RelaxedPrecision
                 OpReturnValue %67
               OpFunctionEnd
