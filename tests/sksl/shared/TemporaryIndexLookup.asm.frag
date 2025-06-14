               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %15
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %17
               OpName %GetTestMatrix_f33 "GetTestMatrix_f33"    ; id %6
               OpName %main "main"                              ; id %7
               OpName %expected "expected"                      ; id %36
               OpName %i "i"                                    ; id %38
               OpName %j "j"                                    ; id %50

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %76 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %27 = OpTypeFunction %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
         %33 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %19

         %20 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function GetTestMatrix_f33
%GetTestMatrix_f33 = OpFunction %mat3v3float None %27

         %28 = OpLabel
         %29 =   OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
         %32 =   OpLoad %mat3v3float %29
                 OpReturnValue %32
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %33         ; RelaxedPrecision
         %34 = OpFunctionParameter %_ptr_Function_v2float

         %35 = OpLabel
   %expected =   OpVariable %_ptr_Function_float Function
          %i =   OpVariable %_ptr_Function_int Function
          %j =   OpVariable %_ptr_Function_int Function
         %61 =   OpVariable %_ptr_Function_mat3v3float Function
                 OpStore %expected %float_0
                 OpStore %i %int_0
                 OpBranch %41

         %41 = OpLabel
                 OpLoopMerge %45 %44 None
                 OpBranch %42

         %42 =     OpLabel
         %46 =       OpLoad %int %i
         %48 =       OpSLessThan %bool %46 %int_3
                     OpBranchConditional %48 %43 %45

         %43 =         OpLabel
                         OpStore %j %int_0
                         OpBranch %51

         %51 =         OpLabel
                         OpLoopMerge %55 %54 None
                         OpBranch %52

         %52 =             OpLabel
         %56 =               OpLoad %int %j
         %57 =               OpSLessThan %bool %56 %int_3
                             OpBranchConditional %57 %53 %55

         %53 =                 OpLabel
         %58 =                   OpLoad %float %expected
         %60 =                   OpFAdd %float %58 %float_1
                                 OpStore %expected %60
         %63 =                   OpFunctionCall %mat3v3float %GetTestMatrix_f33
                                 OpStore %61 %63
         %64 =                   OpLoad %int %i
         %65 =                   OpAccessChain %_ptr_Function_v3float %61 %64
         %67 =                   OpLoad %v3float %65
         %68 =                   OpLoad %int %j
         %69 =                   OpVectorExtractDynamic %float %67 %68
         %70 =                   OpFUnordNotEqual %bool %69 %60
                                 OpSelectionMerge %72 None
                                 OpBranchConditional %70 %71 %72

         %71 =                     OpLabel
         %73 =                       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %76 =                       OpLoad %v4float %73    ; RelaxedPrecision
                                     OpReturnValue %76

         %72 =                 OpLabel
                                 OpBranch %54

         %54 =           OpLabel
         %77 =             OpLoad %int %j
         %78 =             OpIAdd %int %77 %int_1
                           OpStore %j %78
                           OpBranch %51

         %55 =         OpLabel
                         OpBranch %44

         %44 =   OpLabel
         %79 =     OpLoad %int %i
         %80 =     OpIAdd %int %79 %int_1
                   OpStore %i %80
                   OpBranch %41

         %45 = OpLabel
         %81 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %82 =   OpLoad %v4float %81                ; RelaxedPrecision
                 OpReturnValue %82
               OpFunctionEnd
