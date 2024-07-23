### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '72[%72]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %72 = OpVariable %_ptr_Function__runtimearr_float_0 Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testArr"
               OpName %S "S"
               OpMemberName %S 0 "y"
               OpName %testStorageBufferStruct "testStorageBufferStruct"
               OpMemberName %testStorageBufferStruct 0 "testArrStruct"
               OpName %sk_FragColor "sk_FragColor"
               OpName %unsizedInParameterA_ff "unsizedInParameterA_ff"
               OpName %unsizedInParameterB_fS "unsizedInParameterB_fS"
               OpName %unsizedInParameterC_ff "unsizedInParameterC_ff"
               OpName %unsizedInParameterD_fS "unsizedInParameterD_fS"
               OpName %unsizedInParameterE_ff "unsizedInParameterE_ff"
               OpName %unsizedInParameterF_fS "unsizedInParameterF_fS"
               OpName %getColor_h4f "getColor_h4f"
               OpName %getColor_helper_h4f "getColor_helper_h4f"
               OpName %main "main"
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
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %_runtimearr_S_0 ArrayStride 16
               OpDecorate %68 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %11 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
          %S = OpTypeStruct %float
%_runtimearr_S = OpTypeRuntimeArray %S
%testStorageBufferStruct = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_testStorageBufferStruct = OpTypePointer Uniform %testStorageBufferStruct
         %16 = OpVariable %_ptr_Uniform_testStorageBufferStruct Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %26 = OpTypeFunction %float %_ptr_Function__runtimearr_float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%_runtimearr_S_0 = OpTypeRuntimeArray %S
%_ptr_Function__runtimearr_S_0 = OpTypePointer Function %_runtimearr_S_0
         %36 = OpTypeFunction %float %_ptr_Function__runtimearr_S_0
    %float_0 = OpConstant %float 0
         %54 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %75 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%_ptr_Uniform__runtimearr_S = OpTypePointer Uniform %_runtimearr_S
%unsizedInParameterA_ff = OpFunction %float None %26
         %27 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %28 = OpLabel
         %31 = OpAccessChain %_ptr_Function_float %27 %int_0
         %33 = OpLoad %float %31
               OpReturnValue %33
               OpFunctionEnd
%unsizedInParameterB_fS = OpFunction %float None %36
         %37 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %38 = OpLabel
         %39 = OpAccessChain %_ptr_Function_float %37 %int_0 %int_0
         %40 = OpLoad %float %39
               OpReturnValue %40
               OpFunctionEnd
%unsizedInParameterC_ff = OpFunction %float None %26
         %41 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %42 = OpLabel
         %43 = OpAccessChain %_ptr_Function_float %41 %int_0
         %44 = OpLoad %float %43
               OpReturnValue %44
               OpFunctionEnd
%unsizedInParameterD_fS = OpFunction %float None %36
         %45 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %46 = OpLabel
         %47 = OpAccessChain %_ptr_Function_float %45 %int_0 %int_0
         %48 = OpLoad %float %47
               OpReturnValue %48
               OpFunctionEnd
%unsizedInParameterE_ff = OpFunction %float None %26
         %49 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %50 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd
%unsizedInParameterF_fS = OpFunction %float None %36
         %52 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %53 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd
%getColor_h4f = OpFunction %v4float None %54
         %55 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %56 = OpLabel
         %57 = OpAccessChain %_ptr_Function_float %55 %int_0
         %58 = OpLoad %float %57
         %60 = OpAccessChain %_ptr_Function_float %55 %int_1
         %61 = OpLoad %float %60
         %63 = OpAccessChain %_ptr_Function_float %55 %int_2
         %64 = OpLoad %float %63
         %66 = OpAccessChain %_ptr_Function_float %55 %int_3
         %67 = OpLoad %float %66
         %68 = OpCompositeConstruct %v4float %58 %61 %64 %67
               OpReturnValue %68
               OpFunctionEnd
%getColor_helper_h4f = OpFunction %v4float None %54
         %69 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %70 = OpLabel
         %72 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %71 = OpLoad %_runtimearr_float_0 %69
               OpStore %72 %71
         %73 = OpFunctionCall %v4float %getColor_h4f %72
               OpReturnValue %73
               OpFunctionEnd
       %main = OpFunction %void None %75
         %76 = OpLabel
         %80 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %84 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %89 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %93 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %97 = OpVariable %_ptr_Function__runtimearr_S_0 Function
        %101 = OpVariable %_ptr_Function__runtimearr_float_0 Function
        %105 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %77 = OpAccessChain %_ptr_Uniform__runtimearr_float %11 %int_0
         %79 = OpLoad %_runtimearr_float %77
               OpStore %80 %79
         %81 = OpFunctionCall %v4float %getColor_helper_h4f %80
               OpStore %sk_FragColor %81
         %82 = OpAccessChain %_ptr_Uniform__runtimearr_float %11 %int_0
         %83 = OpLoad %_runtimearr_float %82
               OpStore %84 %83
         %85 = OpFunctionCall %float %unsizedInParameterA_ff %84
         %86 = OpAccessChain %_ptr_Uniform__runtimearr_S %16 %int_0
         %88 = OpLoad %_runtimearr_S %86
               OpStore %89 %88
         %90 = OpFunctionCall %float %unsizedInParameterB_fS %89
         %91 = OpAccessChain %_ptr_Uniform__runtimearr_float %11 %int_0
         %92 = OpLoad %_runtimearr_float %91
               OpStore %93 %92
         %94 = OpFunctionCall %float %unsizedInParameterC_ff %93
         %95 = OpAccessChain %_ptr_Uniform__runtimearr_S %16 %int_0
         %96 = OpLoad %_runtimearr_S %95
               OpStore %97 %96
         %98 = OpFunctionCall %float %unsizedInParameterD_fS %97
         %99 = OpAccessChain %_ptr_Uniform__runtimearr_float %11 %int_0
        %100 = OpLoad %_runtimearr_float %99
               OpStore %101 %100
        %102 = OpFunctionCall %float %unsizedInParameterE_ff %101
        %103 = OpAccessChain %_ptr_Uniform__runtimearr_S %16 %int_0
        %104 = OpLoad %_runtimearr_S %103
               OpStore %105 %104
        %106 = OpFunctionCall %float %unsizedInParameterF_fS %105
               OpReturn
               OpFunctionEnd

1 error
