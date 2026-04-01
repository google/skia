               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "testMatrixArray"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %main "main"                      ; id %6
               OpName %index "index"                    ; id %31

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_mat3v3float_int_3 ArrayStride 48
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 144
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 160
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %58 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
      %int_3 = OpConstant %int 3
%_arr_mat3v3float_int_3 = OpTypeArray %mat3v3float %int_3   ; ArrayStride 48
%_UniformBuffer = OpTypeStruct %_arr_mat3v3float_int_3 %v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %28 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%_ptr_Uniform__arr_mat3v3float_int_3 = OpTypePointer Uniform %_arr_mat3v3float_int_3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
    %float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %25 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %25 %24
         %27 =   OpFunctionCall %v4float %main %25
                 OpStore %sk_FragColor %27
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %28         ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_v2float

         %30 = OpLabel
      %index =   OpVariable %_ptr_Function_int Function
                 OpStore %index %int_0
                 OpBranch %34

         %34 = OpLabel
                 OpLoopMerge %38 %37 None
                 OpBranch %35

         %35 =     OpLabel
         %39 =       OpLoad %int %index
         %40 =       OpSLessThan %bool %39 %int_3
                     OpBranchConditional %40 %36 %38

         %36 =         OpLabel
         %42 =           OpAccessChain %_ptr_Uniform__arr_mat3v3float_int_3 %11 %int_0
         %44 =           OpLoad %int %index
         %45 =           OpLoad %int %index
         %46 =           OpAccessChain %_ptr_Uniform_v3float %42 %44 %45
         %48 =           OpLoad %v3float %46
         %49 =           OpLoad %int %index
         %50 =           OpVectorExtractDynamic %float %48 %49
         %52 =           OpFUnordNotEqual %bool %50 %float_1
                         OpSelectionMerge %54 None
                         OpBranchConditional %52 %53 %54

         %53 =             OpLabel
         %55 =               OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %58 =               OpLoad %v4float %55    ; RelaxedPrecision
                             OpReturnValue %58

         %54 =         OpLabel
                         OpBranch %37

         %37 =   OpLabel
         %60 =     OpLoad %int %index
         %61 =     OpIAdd %int %60 %int_1
                   OpStore %index %61
                   OpBranch %34

         %38 = OpLabel
         %62 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %63 =   OpLoad %v4float %62                ; RelaxedPrecision
                 OpReturnValue %63
               OpFunctionEnd
