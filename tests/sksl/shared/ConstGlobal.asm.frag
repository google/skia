               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %MATRIXFIVE "MATRIXFIVE"      ; id %12
               OpName %_UniformBuffer "_UniformBuffer"  ; id %23
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %25
               OpName %verify_const_globals_biih44 "verify_const_globals_biih44"    ; id %6
               OpName %main "main"                                                  ; id %7

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %MATRIXFIVE RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %22 Binding 0
               OpDecorate %22 DescriptorSet 0
               OpDecorate %40 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
 %MATRIXFIVE = OpVariable %_ptr_Private_mat4v4float Private     ; RelaxedPrecision
    %float_5 = OpConstant %float 5
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %18 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %19 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_5
         %21 = OpConstantComposite %mat4v4float %17 %18 %19 %20
%_UniformBuffer = OpTypeStruct %v4float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %22 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
         %30 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %37 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_int %_ptr_Function_mat4v4float
      %false = OpConstantFalse %bool
      %int_7 = OpConstant %int 7
     %int_10 = OpConstant %int 10
     %v4bool = OpTypeVector %bool 4
         %72 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %27

         %28 = OpLabel
         %31 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %31 %30
         %33 =   OpFunctionCall %v4float %main %31
                 OpStore %sk_FragColor %33
                 OpReturn
               OpFunctionEnd


               ; Function verify_const_globals_biih44
%verify_const_globals_biih44 = OpFunction %bool None %37
         %38 = OpFunctionParameter %_ptr_Function_int
         %39 = OpFunctionParameter %_ptr_Function_int
         %40 = OpFunctionParameter %_ptr_Function_mat4v4float   ; RelaxedPrecision

         %41 = OpLabel
         %43 =   OpLoad %int %38
         %45 =   OpIEqual %bool %43 %int_7
                 OpSelectionMerge %47 None
                 OpBranchConditional %45 %46 %47

         %46 =     OpLabel
         %48 =       OpLoad %int %39
         %50 =       OpIEqual %bool %48 %int_10
                     OpBranch %47

         %47 = OpLabel
         %51 =   OpPhi %bool %false %41 %50 %46
                 OpSelectionMerge %53 None
                 OpBranchConditional %51 %52 %53

         %52 =     OpLabel
         %54 =       OpLoad %mat4v4float %40        ; RelaxedPrecision
         %56 =       OpCompositeExtract %v4float %54 0  ; RelaxedPrecision
         %57 =       OpFOrdEqual %v4bool %56 %17        ; RelaxedPrecision
         %58 =       OpAll %bool %57
         %59 =       OpCompositeExtract %v4float %54 1  ; RelaxedPrecision
         %60 =       OpFOrdEqual %v4bool %59 %18        ; RelaxedPrecision
         %61 =       OpAll %bool %60
         %62 =       OpLogicalAnd %bool %58 %61
         %63 =       OpCompositeExtract %v4float %54 2  ; RelaxedPrecision
         %64 =       OpFOrdEqual %v4bool %63 %19        ; RelaxedPrecision
         %65 =       OpAll %bool %64
         %66 =       OpLogicalAnd %bool %62 %65
         %67 =       OpCompositeExtract %v4float %54 3  ; RelaxedPrecision
         %68 =       OpFOrdEqual %v4bool %67 %20        ; RelaxedPrecision
         %69 =       OpAll %bool %68
         %70 =       OpLogicalAnd %bool %66 %69
                     OpBranch %53

         %53 = OpLabel
         %71 =   OpPhi %bool %false %47 %70 %52
                 OpReturnValue %71
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %72         ; RelaxedPrecision
         %73 = OpFunctionParameter %_ptr_Function_v2float

         %74 = OpLabel
         %75 =   OpVariable %_ptr_Function_int Function
         %76 =   OpVariable %_ptr_Function_int Function
         %77 =   OpVariable %_ptr_Function_mat4v4float Function
         %79 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %MATRIXFIVE %21
                 OpStore %75 %int_7
                 OpStore %76 %int_10
                 OpStore %77 %21
         %78 =   OpFunctionCall %bool %verify_const_globals_biih44 %75 %76 %77
                 OpSelectionMerge %83 None
                 OpBranchConditional %78 %81 %82

         %81 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %22 %int_0
         %87 =       OpLoad %v4float %84            ; RelaxedPrecision
                     OpStore %79 %87
                     OpBranch %83

         %82 =     OpLabel
         %88 =       OpAccessChain %_ptr_Uniform_v4float %22 %int_1
         %90 =       OpLoad %v4float %88            ; RelaxedPrecision
                     OpStore %79 %90
                     OpBranch %83

         %83 = OpLabel
         %91 =   OpLoad %v4float %79                ; RelaxedPrecision
                 OpReturnValue %91
               OpFunctionEnd
