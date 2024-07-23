### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-OpTypeRuntimeArray-04680] OpVariable, <id> '74[%74]', is attempting to create memory for an illegal type, OpTypeRuntimeArray.
For Vulkan OpTypeRuntimeArray can only appear as the final member of an OpTypeStruct, thus cannot be instantiated via OpVariable
  %74 = OpVariable %_ptr_Function__runtimearr_float_0 Function

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
               OpName %main "main"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %testStorageBufferStruct 0 Offset 0
               OpMemberDecorate %testStorageBufferStruct 0 RelaxedPrecision
               OpDecorate %testStorageBufferStruct BufferBlock
               OpDecorate %15 Binding 1
               OpDecorate %15 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_runtimearr_float_0 ArrayStride 16
               OpDecorate %_runtimearr_S_0 ArrayStride 16
               OpDecorate %67 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %10 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
          %S = OpTypeStruct %float
%_runtimearr_S = OpTypeRuntimeArray %S
%testStorageBufferStruct = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_testStorageBufferStruct = OpTypePointer Uniform %testStorageBufferStruct
         %15 = OpVariable %_ptr_Uniform_testStorageBufferStruct Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_runtimearr_float_0 = OpTypeRuntimeArray %float
%_ptr_Function__runtimearr_float_0 = OpTypePointer Function %_runtimearr_float_0
         %25 = OpTypeFunction %float %_ptr_Function__runtimearr_float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%_runtimearr_S_0 = OpTypeRuntimeArray %S
%_ptr_Function__runtimearr_S_0 = OpTypePointer Function %_runtimearr_S_0
         %35 = OpTypeFunction %float %_ptr_Function__runtimearr_S_0
    %float_0 = OpConstant %float 0
         %53 = OpTypeFunction %v4float %_ptr_Function__runtimearr_float_0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %void = OpTypeVoid
         %69 = OpTypeFunction %void
%_ptr_Uniform__runtimearr_float = OpTypePointer Uniform %_runtimearr_float
%_ptr_Uniform__runtimearr_S = OpTypePointer Uniform %_runtimearr_S
%unsizedInParameterA_ff = OpFunction %float None %25
         %26 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %27 = OpLabel
         %30 = OpAccessChain %_ptr_Function_float %26 %int_0
         %32 = OpLoad %float %30
               OpReturnValue %32
               OpFunctionEnd
%unsizedInParameterB_fS = OpFunction %float None %35
         %36 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %37 = OpLabel
         %38 = OpAccessChain %_ptr_Function_float %36 %int_0 %int_0
         %39 = OpLoad %float %38
               OpReturnValue %39
               OpFunctionEnd
%unsizedInParameterC_ff = OpFunction %float None %25
         %40 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %41 = OpLabel
         %42 = OpAccessChain %_ptr_Function_float %40 %int_0
         %43 = OpLoad %float %42
               OpReturnValue %43
               OpFunctionEnd
%unsizedInParameterD_fS = OpFunction %float None %35
         %44 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %45 = OpLabel
         %46 = OpAccessChain %_ptr_Function_float %44 %int_0 %int_0
         %47 = OpLoad %float %46
               OpReturnValue %47
               OpFunctionEnd
%unsizedInParameterE_ff = OpFunction %float None %25
         %48 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %49 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd
%unsizedInParameterF_fS = OpFunction %float None %35
         %51 = OpFunctionParameter %_ptr_Function__runtimearr_S_0
         %52 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd
%getColor_h4f = OpFunction %v4float None %53
         %54 = OpFunctionParameter %_ptr_Function__runtimearr_float_0
         %55 = OpLabel
         %56 = OpAccessChain %_ptr_Function_float %54 %int_0
         %57 = OpLoad %float %56
         %59 = OpAccessChain %_ptr_Function_float %54 %int_1
         %60 = OpLoad %float %59
         %62 = OpAccessChain %_ptr_Function_float %54 %int_2
         %63 = OpLoad %float %62
         %65 = OpAccessChain %_ptr_Function_float %54 %int_3
         %66 = OpLoad %float %65
         %67 = OpCompositeConstruct %v4float %57 %60 %63 %66
               OpReturnValue %67
               OpFunctionEnd
       %main = OpFunction %void None %69
         %70 = OpLabel
         %74 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %78 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %83 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %87 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %91 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %95 = OpVariable %_ptr_Function__runtimearr_float_0 Function
         %99 = OpVariable %_ptr_Function__runtimearr_S_0 Function
         %71 = OpAccessChain %_ptr_Uniform__runtimearr_float %10 %int_0
         %73 = OpLoad %_runtimearr_float %71
               OpStore %74 %73
         %75 = OpFunctionCall %v4float %getColor_h4f %74
               OpStore %sk_FragColor %75
         %76 = OpAccessChain %_ptr_Uniform__runtimearr_float %10 %int_0
         %77 = OpLoad %_runtimearr_float %76
               OpStore %78 %77
         %79 = OpFunctionCall %float %unsizedInParameterA_ff %78
         %80 = OpAccessChain %_ptr_Uniform__runtimearr_S %15 %int_0
         %82 = OpLoad %_runtimearr_S %80
               OpStore %83 %82
         %84 = OpFunctionCall %float %unsizedInParameterB_fS %83
         %85 = OpAccessChain %_ptr_Uniform__runtimearr_float %10 %int_0
         %86 = OpLoad %_runtimearr_float %85
               OpStore %87 %86
         %88 = OpFunctionCall %float %unsizedInParameterC_ff %87
         %89 = OpAccessChain %_ptr_Uniform__runtimearr_S %15 %int_0
         %90 = OpLoad %_runtimearr_S %89
               OpStore %91 %90
         %92 = OpFunctionCall %float %unsizedInParameterD_fS %91
         %93 = OpAccessChain %_ptr_Uniform__runtimearr_float %10 %int_0
         %94 = OpLoad %_runtimearr_float %93
               OpStore %95 %94
         %96 = OpFunctionCall %float %unsizedInParameterE_ff %95
         %97 = OpAccessChain %_ptr_Uniform__runtimearr_S %15 %int_0
         %98 = OpLoad %_runtimearr_S %97
               OpStore %99 %98
        %100 = OpFunctionCall %float %unsizedInParameterF_fS %99
               OpReturn
               OpFunctionEnd

1 error
