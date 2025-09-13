               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testStorageBuffer "testStorageBuffer"    ; id %18
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %S "S"                        ; id %21
               OpMemberName %S 0 "y"
               OpName %testStorageBufferStruct "testStorageBufferStruct"    ; id %23
               OpMemberName %testStorageBufferStruct 0 "testArrStruct"
               OpName %sk_FragColor "sk_FragColor"  ; id %25
               OpName %unsizedInParameterA_ff_testArr "unsizedInParameterA_ff_testArr"  ; id %6
               OpName %unsizedInParameterB_fS_testArrStruct "unsizedInParameterB_fS_testArrStruct"  ; id %7
               OpName %unsizedInParameterC_ff_testArr "unsizedInParameterC_ff_testArr"              ; id %8
               OpName %unsizedInParameterD_fS_testArrStruct "unsizedInParameterD_fS_testArrStruct"  ; id %9
               OpName %unsizedInParameterE_ff_testArr "unsizedInParameterE_ff_testArr"              ; id %10
               OpName %unsizedInParameterF_fS_testArrStruct "unsizedInParameterF_fS_testArrStruct"  ; id %11
               OpName %getColor_h4f_testArr "getColor_h4f_testArr"                                  ; id %12
               OpName %getColor_helper_h4f_testArr "getColor_helper_h4f_testArr"                    ; id %13
               OpName %main "main"                                                                  ; id %14

               ; Annotations
               OpDecorate %getColor_h4f_testArr RelaxedPrecision
               OpDecorate %getColor_helper_h4f_testArr RelaxedPrecision
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %testStorageBufferStruct 0 Offset 0
               OpMemberDecorate %testStorageBufferStruct 0 RelaxedPrecision
               OpDecorate %testStorageBufferStruct BufferBlock
               OpDecorate %20 Binding 1
               OpDecorate %20 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %59 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float      ; ArrayStride 4
%testStorageBuffer = OpTypeStruct %_runtimearr_float    ; BufferBlock
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %15 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform   ; Binding 0, DescriptorSet 0
          %S = OpTypeStruct %float
%_runtimearr_S = OpTypeRuntimeArray %S              ; ArrayStride 4
%testStorageBufferStruct = OpTypeStruct %_runtimearr_S  ; BufferBlock
%_ptr_Uniform_testStorageBufferStruct = OpTypePointer Uniform %testStorageBufferStruct
         %20 = OpVariable %_ptr_Uniform_testStorageBufferStruct Uniform     ; Binding 1, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %28 = OpTypeFunction %float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_0 = OpConstant %float 0
         %46 = OpTypeFunction %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %63 = OpTypeFunction %void


               ; Function unsizedInParameterA_ff_testArr
%unsizedInParameterA_ff_testArr = OpFunction %float None %28

         %29 = OpLabel
         %31 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_0
         %33 =   OpLoad %float %31
                 OpReturnValue %33
               OpFunctionEnd


               ; Function unsizedInParameterB_fS_testArrStruct
%unsizedInParameterB_fS_testArrStruct = OpFunction %float None %28

         %34 = OpLabel
         %35 =   OpAccessChain %_ptr_Uniform_float %20 %int_0 %int_0 %int_0
         %36 =   OpLoad %float %35
                 OpReturnValue %36
               OpFunctionEnd


               ; Function unsizedInParameterC_ff_testArr
%unsizedInParameterC_ff_testArr = OpFunction %float None %28

         %37 = OpLabel
         %38 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_0
         %39 =   OpLoad %float %38
                 OpReturnValue %39
               OpFunctionEnd


               ; Function unsizedInParameterD_fS_testArrStruct
%unsizedInParameterD_fS_testArrStruct = OpFunction %float None %28

         %40 = OpLabel
         %41 =   OpAccessChain %_ptr_Uniform_float %20 %int_0 %int_0 %int_0
         %42 =   OpLoad %float %41
                 OpReturnValue %42
               OpFunctionEnd


               ; Function unsizedInParameterE_ff_testArr
%unsizedInParameterE_ff_testArr = OpFunction %float None %28

         %43 = OpLabel
                 OpReturnValue %float_0
               OpFunctionEnd


               ; Function unsizedInParameterF_fS_testArrStruct
%unsizedInParameterF_fS_testArrStruct = OpFunction %float None %28

         %45 = OpLabel
                 OpReturnValue %float_0
               OpFunctionEnd


               ; Function getColor_h4f_testArr
%getColor_h4f_testArr = OpFunction %v4float None %46    ; RelaxedPrecision

         %47 = OpLabel
         %48 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_0
         %49 =   OpLoad %float %48
         %51 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_1
         %52 =   OpLoad %float %51
         %54 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_2
         %55 =   OpLoad %float %54
         %57 =   OpAccessChain %_ptr_Uniform_float %15 %int_0 %int_3
         %58 =   OpLoad %float %57
         %59 =   OpCompositeConstruct %v4float %49 %52 %55 %58  ; RelaxedPrecision
                 OpReturnValue %59
               OpFunctionEnd


               ; Function getColor_helper_h4f_testArr
%getColor_helper_h4f_testArr = OpFunction %v4float None %46     ; RelaxedPrecision

         %60 = OpLabel
         %61 =   OpFunctionCall %v4float %getColor_h4f_testArr
                 OpReturnValue %61
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %63

         %64 = OpLabel
         %65 =   OpFunctionCall %v4float %getColor_helper_h4f_testArr
                 OpStore %sk_FragColor %65
         %66 =   OpFunctionCall %float %unsizedInParameterA_ff_testArr
         %67 =   OpFunctionCall %float %unsizedInParameterB_fS_testArrStruct
         %68 =   OpFunctionCall %float %unsizedInParameterC_ff_testArr
         %69 =   OpFunctionCall %float %unsizedInParameterD_fS_testArrStruct
         %70 =   OpFunctionCall %float %unsizedInParameterE_ff_testArr
         %71 =   OpFunctionCall %float %unsizedInParameterF_fS_testArrStruct
                 OpReturn
               OpFunctionEnd
