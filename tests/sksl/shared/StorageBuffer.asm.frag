               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %bufferIndex
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %SomeData "SomeData"          ; id %8
               OpMemberName %SomeData 0 "a"
               OpMemberName %SomeData 1 "b"
               OpName %storageBuffer "storageBuffer"    ; id %10
               OpMemberName %storageBuffer 0 "offset"
               OpMemberName %storageBuffer 1 "inputData"
               OpName %outputBuffer "outputBuffer"  ; id %13
               OpMemberName %outputBuffer 0 "outputData"
               OpName %sk_FragColor "sk_FragColor"  ; id %15
               OpName %bufferIndex "bufferIndex"    ; id %17
               OpName %_entrypoint_v "_entrypoint_v"    ; id %20
               OpName %main "main"                      ; id %2

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpMemberDecorate %SomeData 0 Offset 0
               OpMemberDecorate %SomeData 1 Offset 16
               OpDecorate %_runtimearr_SomeData ArrayStride 32
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpMemberDecorate %storageBuffer 1 Offset 16
               OpMemberDecorate %storageBuffer 1 RelaxedPrecision
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %3 Binding 0
               OpDecorate %3 DescriptorSet 0
               OpMemberDecorate %outputBuffer 0 Offset 0
               OpMemberDecorate %outputBuffer 0 RelaxedPrecision
               OpDecorate %outputBuffer BufferBlock
               OpDecorate %12 Binding 1
               OpDecorate %12 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %bufferIndex Location 2
               OpDecorate %bufferIndex Flat
               OpDecorate %39 RelaxedPrecision

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
   %SomeData = OpTypeStruct %v4float %v2float
%_runtimearr_SomeData = OpTypeRuntimeArray %SomeData    ; ArrayStride 32
%storageBuffer = OpTypeStruct %uint %_runtimearr_SomeData   ; BufferBlock
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %3 = OpVariable %_ptr_Uniform_storageBuffer Uniform   ; Binding 0, DescriptorSet 0
%outputBuffer = OpTypeStruct %_runtimearr_SomeData              ; BufferBlock
%_ptr_Uniform_outputBuffer = OpTypePointer Uniform %outputBuffer
         %12 = OpVariable %_ptr_Uniform_outputBuffer Uniform    ; Binding 1, DescriptorSet 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%bufferIndex = OpVariable %_ptr_Input_int Input     ; Location 2, Flat
       %void = OpTypeVoid
         %22 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %29 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Uniform_SomeData = OpTypePointer Uniform %SomeData
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %22

         %23 = OpLabel
         %26 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %26 %25
         %28 =   OpFunctionCall %v4float %main %26
                 OpStore %sk_FragColor %28
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %29         ; RelaxedPrecision
         %30 = OpFunctionParameter %_ptr_Function_v2float

         %31 = OpLabel
         %34 =   OpAccessChain %_ptr_Uniform_uint %3 %int_0
         %36 =   OpLoad %uint %34
         %37 =   OpAccessChain %_ptr_Uniform_SomeData %3 %int_1 %36
         %39 =   OpLoad %SomeData %37               ; RelaxedPrecision
         %40 =   OpAccessChain %_ptr_Uniform_uint %3 %int_0
         %41 =   OpLoad %uint %40
         %42 =   OpAccessChain %_ptr_Uniform_SomeData %12 %int_0 %41
                 OpStore %42 %39
         %43 =   OpLoad %int %bufferIndex
         %44 =   OpAccessChain %_ptr_Uniform_v4float %3 %int_1 %43 %int_0
         %46 =   OpLoad %v4float %44
         %47 =   OpLoad %int %bufferIndex
         %48 =   OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %47 %int_1
         %50 =   OpLoad %v2float %48
         %51 =   OpCompositeExtract %float %50 0
         %52 =   OpVectorTimesScalar %v4float %46 %51
                 OpReturnValue %52
               OpFunctionEnd
