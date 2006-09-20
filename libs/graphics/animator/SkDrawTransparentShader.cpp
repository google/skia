#include "SkDrawTransparentShader.h"
#include "SkTransparentShader.h"

SkShader* SkDrawTransparentShader::getShader() {
	return new SkTransparentShader();
}

