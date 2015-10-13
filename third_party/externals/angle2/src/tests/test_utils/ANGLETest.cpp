#include "ANGLETest.h"
#include "EGLWindow.h"
#include "OSWindow.h"

ANGLETest::ANGLETest()
    : mEGLWindow(nullptr),
      mWidth(16),
      mHeight(16)
{
    mEGLWindow =
        new EGLWindow(GetParam().majorVersion, GetParam().minorVersion, GetParam().eglParameters);
}

ANGLETest::~ANGLETest()
{
    SafeDelete(mEGLWindow);
}

void ANGLETest::SetUp()
{
    // Resize the window before creating the context so that the first make current
    // sets the viewport and scissor box to the right size.
    bool needSwap = false;
    if (mOSWindow->getWidth() != mWidth || mOSWindow->getHeight() != mHeight)
    {
        if (!mOSWindow->resize(mWidth, mHeight))
        {
            FAIL() << "Failed to resize ANGLE test window.";
        }
        needSwap = true;
    }

    if (!createEGLContext())
    {
        FAIL() << "egl context creation failed.";
    }

    if (needSwap)
    {
        // Swap the buffers so that the default framebuffer picks up the resize
        // which will allow follow-up test code to assume the framebuffer covers
        // the whole window.
        swapBuffers();
    }

    // This Viewport command is not strictly necessary but we add it so that programs
    // taking OpenGL traces can guess the size of the default framebuffer and show it
    // in their UIs
    glViewport(0, 0, mWidth, mHeight);
}

void ANGLETest::TearDown()
{
    swapBuffers();
    mOSWindow->messageLoop();

    if (!destroyEGLContext())
    {
        FAIL() << "egl context destruction failed.";
    }

    // Check for quit message
    Event myEvent;
    while (mOSWindow->popEvent(&myEvent))
    {
        if (myEvent.Type == Event::EVENT_CLOSED)
        {
            exit(0);
        }
    }
}

void ANGLETest::swapBuffers()
{
    if (mEGLWindow->isGLInitialized())
    {
        mEGLWindow->swap();
    }
}

void ANGLETest::drawQuad(GLuint program, const std::string& positionAttribName, GLfloat quadDepth, GLfloat quadScale)
{
    GLint positionLocation = glGetAttribLocation(program, positionAttribName.c_str());

    glUseProgram(program);

    const GLfloat vertices[] =
    {
        -1.0f * quadScale,  1.0f * quadScale, quadDepth,
        -1.0f * quadScale, -1.0f * quadScale, quadDepth,
         1.0f * quadScale, -1.0f * quadScale, quadDepth,

        -1.0f * quadScale,  1.0f * quadScale, quadDepth,
         1.0f * quadScale, -1.0f * quadScale, quadDepth,
         1.0f * quadScale,  1.0f * quadScale, quadDepth,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glUseProgram(0);
}

GLuint ANGLETest::compileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), NULL, &infoLog[0]);

        std::cerr << "shader compilation failed: " << &infoLog[0];

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

static bool checkExtensionExists(const char *allExtensions, const std::string &extName)
{
    return strstr(allExtensions, extName.c_str()) != nullptr;
}

bool ANGLETest::extensionEnabled(const std::string &extName)
{
    return checkExtensionExists(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)),
                                extName);
}

bool ANGLETest::eglDisplayExtensionEnabled(EGLDisplay display, const std::string &extName)
{
    return checkExtensionExists(eglQueryString(display, EGL_EXTENSIONS), extName);
}

bool ANGLETest::eglClientExtensionEnabled(const std::string &extName)
{
    return checkExtensionExists(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS), extName);
}

void ANGLETest::setWindowWidth(int width)
{
    mWidth = width;
}

void ANGLETest::setWindowHeight(int height)
{
    mHeight = height;
}

void ANGLETest::setConfigRedBits(int bits)
{
    mEGLWindow->setConfigRedBits(bits);
}

void ANGLETest::setConfigGreenBits(int bits)
{
    mEGLWindow->setConfigGreenBits(bits);
}

void ANGLETest::setConfigBlueBits(int bits)
{
    mEGLWindow->setConfigBlueBits(bits);
}

void ANGLETest::setConfigAlphaBits(int bits)
{
    mEGLWindow->setConfigAlphaBits(bits);
}

void ANGLETest::setConfigDepthBits(int bits)
{
    mEGLWindow->setConfigDepthBits(bits);
}

void ANGLETest::setConfigStencilBits(int bits)
{
    mEGLWindow->setConfigStencilBits(bits);
}

void ANGLETest::setMultisampleEnabled(bool enabled)
{
    mEGLWindow->setMultisample(enabled);
}

int ANGLETest::getClientVersion() const
{
    return mEGLWindow->getClientMajorVersion();
}

EGLWindow *ANGLETest::getEGLWindow() const
{
    return mEGLWindow;
}

int ANGLETest::getWindowWidth() const
{
    return mWidth;
}

int ANGLETest::getWindowHeight() const
{
    return mHeight;
}

bool ANGLETest::isMultisampleEnabled() const
{
    return mEGLWindow->isMultisample();
}

bool ANGLETest::createEGLContext()
{
    return mEGLWindow->initializeGL(mOSWindow);
}

bool ANGLETest::destroyEGLContext()
{
    mEGLWindow->destroyGL();
    return true;
}

bool ANGLETest::InitTestWindow()
{
    mOSWindow = CreateOSWindow();
    if (!mOSWindow->initialize("ANGLE_TEST", 128, 128))
    {
        return false;
    }

    mOSWindow->setVisible(true);

    return true;
}

bool ANGLETest::DestroyTestWindow()
{
    if (mOSWindow)
    {
        mOSWindow->destroy();
        delete mOSWindow;
        mOSWindow = NULL;
    }

    return true;
}

void ANGLETest::SetWindowVisible(bool isVisible)
{
    mOSWindow->setVisible(isVisible);
}

bool ANGLETest::isIntel() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("Intel") != std::string::npos);
}

bool ANGLETest::isAMD() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("AMD") != std::string::npos) ||
           (rendererString.find("ATI") != std::string::npos);
}

bool ANGLETest::isNVidia() const
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("NVIDIA") != std::string::npos);
}

EGLint ANGLETest::getPlatformRenderer() const
{
    assert(mEGLWindow);
    return mEGLWindow->getPlatform().renderer;
}

OSWindow *ANGLETest::mOSWindow = NULL;

void ANGLETestEnvironment::SetUp()
{
    if (!ANGLETest::InitTestWindow())
    {
        FAIL() << "Failed to create ANGLE test window.";
    }
}

void ANGLETestEnvironment::TearDown()
{
    ANGLETest::DestroyTestWindow();
}
