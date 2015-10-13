#ifndef LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_
#define LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_

#include "libANGLE/Error.h"

#include "common/angleutils.h"
#include "common/platform.h"

#include <vector>
#include <string>

namespace gl
{
class InfoLog;
}

namespace rx
{

struct CompileConfig
{
    UINT flags;
    std::string name;

    CompileConfig();
    CompileConfig(UINT flags, const std::string &name);
};

class HLSLCompiler : angle::NonCopyable
{
  public:
    HLSLCompiler();
    ~HLSLCompiler();

    void release();

    // Attempt to compile a HLSL shader using the supplied configurations, may output a NULL compiled blob
    // even if no GL errors are returned.
    gl::Error compileToBinary(gl::InfoLog &infoLog, const std::string &hlsl, const std::string &profile,
                              const std::vector<CompileConfig> &configs, const D3D_SHADER_MACRO *overrideMacros,
                              ID3DBlob **outCompiledBlob, std::string *outDebugInfo);

    gl::Error disassembleBinary(ID3DBlob *shaderBinary, std::string *disassemblyOut);

  private:
    gl::Error initialize();

    bool mInitialized;
    HMODULE mD3DCompilerModule;
    pD3DCompile mD3DCompileFunc;
    pD3DDisassemble mD3DDisassembleFunc;
};

}

#endif // LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_
