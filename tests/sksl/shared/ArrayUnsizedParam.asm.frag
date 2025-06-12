               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testStorageBuffer "testStorageBuffer"    ; id %14
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %S "S"                        ; id %17
               OpMemberName %S 0 "y"
               OpName %testStorageBufferStruct "testStorageBufferStruct"    ; id %19
               OpMemberName %testStorageBufferStruct 0 "testArrStruct"
               OpName %sk_FragColor "sk_FragColor"  ; id %21
               OpName %unsizedInParameterA_ff_testArr "unsizedInParameterA_ff_testArr"  ; id %2
               OpName %unsizedInParameterB_fS_testArrStruct "unsizedInParameterB_fS_testArrStruct"  ; id %3
               OpName %unsizedInParameterC_ff_testArr "unsizedInParameterC_ff_testArr"              ; id %4
               OpName %unsizedInParameterD_fS_testArrStruct "unsizedInParameterD_fS_testArrStruct"  ; id %5
               OpName %unsizedInParameterE_ff_testArr "unsizedInParameterE_ff_testArr"              ; id %6
               OpName %unsizedInParameterF_fS_testArrStruct "unsizedInParameterF_fS_testArrStruct"  ; id %7
               OpName %getColor_h4f_testArr "getColor_h4f_testArr"                                  ; id %8
               OpName %getColor_helper_h4f_testArr "getColor_helper_h4f_testArr"                    ; id %9
               OpName %main "main"                                                                  ; id %10

               ; Annotations
               OpDecorate %getColor_h4f_testArr RelaxedPrecision
               OpDecorate %getColor_helper_h4f_testArr RelaxedPrecision
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %testStorageBufferStruct 0 Offset 0
               OpMemberDecorate %testStorageBufferStruct 0 RelaxedPrecision
               OpDecorate %testStorageBufferStruct BufferBlock
               OpDecorate %16 Binding 1
               OpDecorate %16 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %56 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float      ; ArrayStride 4
%testStorageBuffer = OpTypeStruct %_runtimearr_float    ; BufferBlock
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %11 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform   ; Binding 0, DescriptorSet 0
          %S = OpTypeStruct %float
%_runtimearr_S = OpTypeRuntimeArray %S              ; ArrayStride 4
%testStorageBufferStruct = OpTypeStruct %_runtimearr_S  ; BufferBlock
%_ptr_Uniform_testStorageBufferStruct = OpTypePointer Uniform %testStorageBufferStruct
         %16 = OpVariable %_ptr_Uniform_testStorageBufferStruct Uniform     ; Binding 1, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %24 = OpTypeFunction %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_0 = OpConstant %float 0
         %43 = OpTypeFunction %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %60 = OpTypeFunction %void


               ; Function unsizedInParameterA_ff_testArr
%unsizedInParameterA_ff_testArr = OpFunction %float None %24

         %25 = OpLabel
         %28 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_0
         %30 =   OpLoad %float %28
                 OpReturnValue %30
               OpFunctionEnd


               ; Function unsizedInParameterB_fS_testArrStruct
%unsizedInParameterB_fS_testArrStruct = OpFunction %float None %24

         %31 = OpLabel
         %32 =   OpAccessChain %_ptr_Uniform_float %16 %int_0 %int_0 %int_0
         %33 =   OpLoad %float %32
                 OpReturnValue %33
               OpFunctionEnd


               ; Function unsizedInParameterC_ff_testArr
%unsizedInParameterC_ff_testArr = OpFunction %float None %24

         %34 = OpLabel
         %35 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_0
         %36 =   OpLoad %float %35
                 OpReturnValue %36
               OpFunctionEnd


               ; Function unsizedInParameterD_fS_testArrStruct
%unsizedInParameterD_fS_testArrStruct = OpFunction %float None %24

         %37 = OpLabel
         %38 =   OpAccessChain %_ptr_Uniform_float %16 %int_0 %int_0 %int_0
         %39 =   OpLoad %float %38
                 OpReturnValue %39
               OpFunctionEnd


               ; Function unsizedInParameterE_ff_testArr
%unsizedInParameterE_ff_testArr = OpFunction %float None %24

         %40 = OpLabel
                 OpReturnValue %float_0
               OpFunctionEnd


               ; Function unsizedInParameterF_fS_testArrStruct
%unsizedInParameterF_fS_testArrStruct = OpFunction %float None %24

         %42 = OpLabel
                 OpReturnValue %float_0
               OpFunctionEnd


               ; Function getColor_h4f_testArr
%getColor_h4f_testArr = OpFunction %v4float None %43    ; RelaxedPrecision

         %44 = OpLabel
         %45 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_0
         %46 =   OpLoad %float %45
         %48 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_1
         %49 =   OpLoad %float %48
         %51 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_2
         %52 =   OpLoad %float %51
         %54 =   OpAccessChain %_ptr_Uniform_float %11 %int_0 %int_3
         %55 =   OpLoad %float %54
         %56 =   OpCompositeConstruct %v4float %46 %49 %52 %55  ; RelaxedPrecision
                 OpReturnValue %56
               OpFunctionEnd


               ; Function getColor_helper_h4f_testArr
%getColor_helper_h4f_testArr = OpFunction %v4float None %43     ; RelaxedPrecision

         %57 = OpLabel
         %58 =   OpFunctionCall %v4float %getColor_h4f_testArr
                 OpReturnValue %58
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %60

         %61 = OpLabel
         %62 =   OpFunctionCall %v4float %getColor_helper_h4f_testArr
                 OpStore %sk_FragColor %62
         %63 =   OpFunctionCall %float %unsizedInParameterA_ff_testArr
         %64 =   OpFunctionCall %float %unsizedInParameterB_fS_testArrStruct
         %65 =   OpFunctionCall %float %unsizedInParameterC_ff_testArr
         %66 =   OpFunctionCall %float %unsizedInParameterD_fS_testArrStruct
         %67 =   OpFunctionCall %float %unsizedInParameterE_ff_testArr
         %68 =   OpFunctionCall %float %unsizedInParameterF_fS_testArrStruct
                 OpReturn
               OpFunctionEnd
