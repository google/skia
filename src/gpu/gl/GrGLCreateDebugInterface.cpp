
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#include "SkTArray.h"

// the OpenGLES 2.0 spec says this must be >= 2
static const GrGLint kDefaultMaxTextureUnits = 8;

////////////////////////////////////////////////////////////////////////////////
// This object is used to track the OpenGL objects. We don't use real
// reference counting (i.e., we don't free the objects when their ref count 
// goes to 0) so that we can detect invalid memory accesses. The refs we
// are tracking in this class are actually OpenGL's references to the objects
// not "ours"
// Each object also gets a unique globally identifying ID
class GrFakeRefObj
{
public:
    GrFakeRefObj(GrGLuint ID) 
        : fRef(0)
        , fID(ID)
        , fMarkedForDeletion(false)
        , fDeleted(false) {
    }
    virtual ~GrFakeRefObj() {};

    void ref()   { fRef++; }
    void unref() { 
        fRef--; 
        // often in OpenGL a given object may still be in use when the 
        // delete call is made. In these cases the object is marked
        // for deletion and then freed when it is no longer in use
        if (0 == fRef && fMarkedForDeletion) {
            this->deleteAction(); 
        }
    }
    int getRefCount() const { return fRef; }

    GrGLuint getID() const { return fID; }

    void setMarkedForDeletion() { fMarkedForDeletion = true; }
    bool getMarkedForDeletion() const { return fMarkedForDeletion; }

    // setDeleted should only ever appear in deleteAction methods!
    void setDeleted()           { fDeleted = true; }
    bool getDeleted() const     { return fDeleted; }

    // The deleteAction fires if the object has been marked for deletion but
    // couldn't be deleted earlier due to refs
    virtual void deleteAction() = 0;

protected:
private:
    int         fRef;
    GrGLuint    fID;
    bool        fMarkedForDeletion;
    // The deleted flag is only set when OpenGL thinks the object is deleted
    // It is obviously still allocated w/in this framework
    bool        fDeleted;
};

////////////////////////////////////////////////////////////////////////////////
class GrBufferObj : public GrFakeRefObj
{
public:
    GrBufferObj(GrGLuint ID) 
        : GrFakeRefObj(ID)
        , fDataPtr(NULL)
        , fMapped(false)
        , fBound(false)
        , fSize(0)
        , fUsage(GR_GL_STATIC_DRAW) {
    }
    virtual ~GrBufferObj() {
        delete[] fDataPtr;
    }

    void access() {
        // cannot access the buffer if it is currently mapped
        GrAlwaysAssert(!fMapped);
    }

    void setMapped()        { fMapped = true; }
    void resetMapped()      { fMapped = false; }
    bool getMapped() const  { return fMapped; }

    void setBound()         { fBound = true; }
    void resetBound()       { fBound = false; }
    bool getBound() const   { return fBound; }

    void allocate(GrGLint size, const GrGLvoid *dataPtr) {
        GrAlwaysAssert(size >= 0);

        // delete pre-existing data
        delete fDataPtr;

        fSize = size;
        fDataPtr = new char[size];
        if (dataPtr) {
            memcpy(fDataPtr, dataPtr, fSize);
        }
        // TODO: w/ no dataPtr the data is unitialized - this could be tracked
    }
    GrGLint getSize() const { return fSize; }
    GrGLvoid *getDataPtr()  { return fDataPtr; }

    GrGLint getUsage() const { return fUsage; }
    void setUsage(GrGLint usage) { fUsage = usage; }

    virtual void deleteAction() SK_OVERRIDE {

        // buffers are automatically unmapped when deleted
        this->resetMapped();
        this->setDeleted();
    }


protected:
private:

    GrGLvoid*   fDataPtr;
    bool        fMapped;       // is the buffer object mapped via "glMapBuffer"?
    bool        fBound;        // is the buffer object bound via "glBindBuffer"?
    GrGLint     fSize;         // size in bytes
    GrGLint     fUsage;        // one of: GL_STREAM_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW

    typedef GrFakeRefObj INHERITED;
};


////////////////////////////////////////////////////////////////////////////////
class GrShaderObj : public GrFakeRefObj
{
public:
    GrShaderObj(GrGLuint ID, GrGLenum type) 
        : GrFakeRefObj(ID)
        , fType(type)   {}

    GrGLenum getType()  { return fType; }

    virtual void deleteAction() SK_OVERRIDE {

        this->setDeleted();
    }

protected:
private:
    GrGLenum fType;  // either GR_GL_VERTEX_SHADER or GR_GL_FRAGMENT_SHADER

    typedef GrFakeRefObj INHERITED;
};


////////////////////////////////////////////////////////////////////////////////
class GrProgramObj : public GrFakeRefObj
{
public:
    GrProgramObj(GrGLuint ID) 
        : GrFakeRefObj(ID)
        , fInUse(false) {}

    void AttachShader(GrShaderObj *shader)
    {
        shader->ref();
        fShaders.push_back(shader);
    }

    virtual void deleteAction() SK_OVERRIDE {

        // shaders are automatically detached from a deleted program. They will only be
        // deleted if they were marked for deletion by a prior call to glDeleteShader
        for (int i = 0; i < fShaders.count(); ++i) {
            fShaders[i]->unref();
        }
        fShaders.reset();

        this->setDeleted();
    }

    void setInUse()         { fInUse = true; }
    bool getInUse() const   { return fInUse; }

protected:

private:
    SkTArray<GrShaderObj *> fShaders;
    bool fInUse;            // has this program been activated by a glUseProgram call?

    typedef GrFakeRefObj INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
// This is the main debugging object. It is a singleton and keeps track of
// all the other debug objects.
class GrDebugGL
{
public:
    // TODO: merge findBuffer, findShader & findProgram??
    GrBufferObj *findBuffer(GrGLuint ID)
    {
        GrFakeRefObj *obj = this->findObject(ID);
        if (NULL == obj) {
            return NULL;
        }

        return reinterpret_cast<GrBufferObj *>(obj);
    }

    GrShaderObj *findShader(GrGLuint ID)
    {
        GrFakeRefObj *obj = this->findObject(ID);
        if (NULL == obj) {
            return NULL;
        }

        return reinterpret_cast<GrShaderObj *>(obj);
    }

    GrProgramObj *findProgram(GrGLuint ID)
    {
        GrFakeRefObj *obj = this->findObject(ID);
        if (NULL == obj) {
            return NULL;
        }

        return reinterpret_cast<GrProgramObj *>(obj);
    }

    // TODO: merge createBuffer, createShader, createProgram??
    GrBufferObj *createBuffer()
    {
        GrBufferObj *buffer = new GrBufferObj(++fNextID);

        fObjects.push_back(buffer);

        return buffer;
    }

    GrShaderObj *createShader(GrGLenum type)
    {
        GrShaderObj *shader = new GrShaderObj(++fNextID, type);

        fObjects.push_back(shader);

        return shader;
    }

    GrProgramObj *createProgram()
    {
        GrProgramObj *program = new GrProgramObj(++fNextID);

        fObjects.push_back(program);

        return program;
    }

    GrFakeRefObj *findObject(GrGLuint ID)
    {
        for (int i = 0; i < fObjects.count(); ++i) {
            if (fObjects[i]->getID() == ID) {
                // The application shouldn't be accessing objects
                // that (as far as OpenGL knows) were already deleted 
                GrAlwaysAssert(!fObjects[i]->getDeleted());
                GrAlwaysAssert(!fObjects[i]->getMarkedForDeletion());
                return fObjects[i];
            }
        }

        return NULL;
    }

    
    void setMaxTextureUnits(GrGLuint maxTextureUnits) { fMaxTextureUnits = maxTextureUnits; }
    GrGLuint getMaxTextureUnits() const { return fMaxTextureUnits; }

    void setCurTextureUnit(GrGLuint curTextureUnit) { fCurTextureUnit = curTextureUnit; }
    GrGLuint getCurTextureUnit() const { return fCurTextureUnit; }

    void setArrayBuffer(GrBufferObj *arrayBuffer)    { fArrayBuffer = arrayBuffer; }
    GrBufferObj *getArrayBuffer()                   { return fArrayBuffer; }

    void setElementArrayBuffer(GrBufferObj *elementArrayBuffer)    { fElementArrayBuffer = elementArrayBuffer; }
    GrBufferObj *getElementArrayBuffer()                            { return fElementArrayBuffer; }

    static GrDebugGL *getInstance()
    {
//        static GrDebugGL Obj;

        return &Obj;
    }

    void report() const {
        for (int i = 0; i < fObjects.count(); ++i) {
            GrAlwaysAssert(fObjects[i]->getDeleted());
        }
    }

protected:

private:
    GrGLuint        fMaxTextureUnits;
    GrGLuint        fCurTextureUnit;
    GrBufferObj *   fArrayBuffer;
    GrBufferObj *   fElementArrayBuffer;

    static int fNextID;                     // source for globally unique IDs

    static GrDebugGL Obj;

    // global store of all objects
    SkTArray<GrFakeRefObj *> fObjects;

    GrDebugGL() 
        : fMaxTextureUnits(kDefaultMaxTextureUnits)
        ,  fCurTextureUnit(0)
        ,  fArrayBuffer(NULL)
        ,  fElementArrayBuffer(NULL)
    {
    }

    ~GrDebugGL() {
        this->report();

        for (int i = 0; i < fObjects.count(); ++i) {
            delete fObjects[i];
        }
        fObjects.reset();
        fArrayBuffer = NULL;
        fElementArrayBuffer = NULL;
    }
};

int GrDebugGL::fNextID = 0;
GrDebugGL GrDebugGL::Obj;

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLActiveTexture(GrGLenum texture) 
{
    
    GrAlwaysAssert(0 <= texture);
//    GrAlwaysAssert(texture < GrDebugGL::getInstance()->getMaxTextureUnits());

    GrDebugGL::getInstance()->setCurTextureUnit(texture);
}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLAttachShader(GrGLuint programID, GrGLuint shaderID) 
{
    GrProgramObj *program = GrDebugGL::getInstance()->findProgram(programID);
    GrAlwaysAssert(program);

    GrShaderObj *shader = GrDebugGL::getInstance()->findShader(shaderID);
    GrAlwaysAssert(shader);

    program->AttachShader(shader);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBeginQuery(GrGLenum target, GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindTexture(GrGLenum target, GrGLuint texture) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocation(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendFunc(GrGLenum sfactor, GrGLenum dfactor) {}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage) 
{
    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);
    GrAlwaysAssert(size >= 0);
    GrAlwaysAssert(GR_GL_STREAM_DRAW == usage || GR_GL_STATIC_DRAW == usage || GR_GL_DYNAMIC_DRAW == usage);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glBufferData");
            break;
    }

    GrAlwaysAssert(buffer);
    GrAlwaysAssert(buffer->getBound());

    buffer->allocate(size, data);
    buffer->setUsage(usage);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBufferSubData(GrGLenum target, GrGLintptr offset, GrGLsizeiptr size, const GrGLvoid* data) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLClear(GrGLbitfield mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLClearColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLClearStencil(GrGLint s) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLColorMask(GrGLboolean red, GrGLboolean green, GrGLboolean blue, GrGLboolean alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLCompileShader(GrGLuint shader) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLCompressedTexImage2D(GrGLenum target, GrGLint level, GrGLenum internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLsizei imageSize, const GrGLvoid* data) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLCullFace(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDepthMask(GrGLboolean flag) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDisable(GrGLenum cap) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDisableVertexAttribArray(GrGLuint index) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawArrays(GrGLenum mode, GrGLint first, GrGLsizei count) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawBuffer(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawBuffers(GrGLsizei n, const GrGLenum* bufs) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDrawElements(GrGLenum mode, GrGLsizei count, GrGLenum type, const GrGLvoid* indices) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLEnable(GrGLenum cap) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLEnableVertexAttribArray(GrGLuint index) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLEndQuery(GrGLenum target) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFinish() {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFlush() {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFrontFace(GrGLenum mode) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLLineWidth(GrGLfloat width) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLLinkProgram(GrGLuint program) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLPixelStorei(GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLQueryCounter(GrGLuint id, GrGLenum target) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLReadBuffer(GrGLenum src) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLReadPixels(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLScissor(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLShaderSource(GrGLuint shader, GrGLsizei count, const char** str, const GrGLint* length) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilFuncSeparate(GrGLenum face, GrGLenum func, GrGLint ref, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilMask(GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilMaskSeparate(GrGLenum face, GrGLuint mask) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilOp(GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLStencilOpSeparate(GrGLenum face, GrGLenum fail, GrGLenum zfail, GrGLenum zpass) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexImage2D(GrGLenum target, GrGLint level, GrGLint internalformat, GrGLsizei width, GrGLsizei height, GrGLint border, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexParameteri(GrGLenum target, GrGLenum pname, GrGLint param) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexStorage2D(GrGLenum target, GrGLsizei levels, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLTexSubImage2D(GrGLenum target, GrGLint level, GrGLint xoffset, GrGLint yoffset, GrGLsizei width, GrGLsizei height, GrGLenum format, GrGLenum type, const GrGLvoid* pixels) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1f(GrGLint location, GrGLfloat v0) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1i(GrGLint location, GrGLint v0) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform1iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2f(GrGLint location, GrGLfloat v0, GrGLfloat v1) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2i(GrGLint location, GrGLint v0, GrGLint v1) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform2iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform3iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4f(GrGLint location, GrGLfloat v0, GrGLfloat v1, GrGLfloat v2, GrGLfloat v3) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4i(GrGLint location, GrGLint v0, GrGLint v1, GrGLint v2, GrGLint v3) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4fv(GrGLint location, GrGLsizei count, const GrGLfloat* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniform4iv(GrGLint location, GrGLsizei count, const GrGLint* v) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix2fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix3fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLUniformMatrix4fv(GrGLint location, GrGLsizei count, GrGLboolean transpose, const GrGLfloat* value) {}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLUseProgram(GrGLuint programID) {

    GrProgramObj *program = GrDebugGL::getInstance()->findProgram(programID);
    GrAlwaysAssert(program);

    program->setInUse();
}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttrib4fv(GrGLuint indx, const GrGLfloat* values) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttribPointer(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLViewport(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFramebuffer(GrGLenum target, GrGLuint framebuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindRenderbuffer(GrGLenum target, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteFramebuffers(GrGLsizei n, const GrGLuint *framebuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteRenderbuffers(GrGLsizei n, const GrGLuint *renderbuffers) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferRenderbuffer(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderbuffer) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint texture, GrGLint level) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetFramebufferAttachmentParameteriv(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetRenderbufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorage(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorageMultisample(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlitFramebuffer(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLResolveMultisampleFramebuffer() {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocationIndexed(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name) {}

GrGLenum GR_GL_FUNCTION_TYPE debugGLCheckFramebufferStatus(GrGLenum target) {
    return GR_GL_FRAMEBUFFER_COMPLETE;
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateProgram() {

    GrProgramObj *program = GrDebugGL::getInstance()->createProgram();

    return program->getID();
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateShader(GrGLenum type) {
    
    GrAlwaysAssert(GR_GL_VERTEX_SHADER == type || GR_GL_FRAGMENT_SHADER == type);

    GrShaderObj *shader = GrDebugGL::getInstance()->createShader(type);

    return shader->getID();
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteProgram(GrGLuint programID) {

    GrProgramObj *program = GrDebugGL::getInstance()->findProgram(programID);
    GrAlwaysAssert(program);

    program->unref();

    if (program->getRefCount()) {
        // someone is still using this program so we can't delete it here
        program->setMarkedForDeletion();
    } else {
        program->deleteAction();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteShader(GrGLuint shaderID) {

    GrShaderObj *shader = GrDebugGL::getInstance()->findShader(shaderID);
    GrAlwaysAssert(shader);

    shader->unref();

    if (shader->getRefCount()) {
        // someone is still using this shader so we can't delete it here
        shader->setMarkedForDeletion();
    } else {
        shader->deleteAction();
    }
}

// same function used for all glGen*(GLsize i, GLuint*) functions
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenIds(GrGLsizei n, GrGLuint* ids) {
    static int gCurrID = 1;
    for (int i = 0; i < n; ++i) {
        ids[i] = ++gCurrID;
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenBuffers(GrGLsizei n, GrGLuint* ids) {

    for (int i = 0; i < n; ++i) {
        GrBufferObj *buffer = GrDebugGL::getInstance()->createBuffer();
        GrAlwaysAssert(buffer);
        ids[i] = buffer->getID();
    }
}

// same delete function for all glDelete*(GLsize i, const GLuint*) except buffers
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteIds(GrGLsizei n, const GrGLuint* ids) {}


GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindBuffer(GrGLenum target, GrGLuint bufferID) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = GrDebugGL::getInstance()->findBuffer(bufferID);
    // 0 is a permissable bufferID - it unbinds the current buffer

    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            if (GrDebugGL::getInstance()->getArrayBuffer()) {
                // automatically break the binding of the old buffer
                GrDebugGL::getInstance()->getArrayBuffer()->resetBound();
            }
            if (buffer) {
                GrAlwaysAssert(!buffer->getBound());
            }
            GrDebugGL::getInstance()->setArrayBuffer(buffer);
            if (buffer) {
                buffer->setBound();
            }
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            if (GrDebugGL::getInstance()->getElementArrayBuffer()) {
                // automatically break the binding of the old buffer
                GrDebugGL::getInstance()->getElementArrayBuffer()->resetBound();
            }
            if (buffer) {
                GrAlwaysAssert(!buffer->getBound());
            }
            GrDebugGL::getInstance()->setElementArrayBuffer(buffer);
            if (buffer) {
                buffer->setBound();
            }
            break;
        default:
            GrCrash("Unexpected target to glBindBuffer");
            break;
    }
}

// deleting a bound buffer has the side effect of binding 0
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteBuffers(GrGLsizei n, const GrGLuint* ids) {
    // first potentially unbind the buffers
    for (int i = 0; i < n; ++i) {

        if (GrDebugGL::getInstance()->getArrayBuffer() && 
            ids[i] == GrDebugGL::getInstance()->getArrayBuffer()->getID()) {
            // this ID is the current array buffer
            GrAlwaysAssert(GrDebugGL::getInstance()->getArrayBuffer()->getBound());
            GrDebugGL::getInstance()->getArrayBuffer()->resetBound();
            GrDebugGL::getInstance()->setArrayBuffer(NULL);
        }
        if (GrDebugGL::getInstance()->getElementArrayBuffer() && 
            ids[i] == GrDebugGL::getInstance()->getElementArrayBuffer()->getID()) {
            // this ID is the current element array buffer
            GrAlwaysAssert(GrDebugGL::getInstance()->getElementArrayBuffer()->getBound());
            GrDebugGL::getInstance()->getElementArrayBuffer()->resetBound();
            GrDebugGL::getInstance()->setElementArrayBuffer(NULL);
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrBufferObj *buffer = GrDebugGL::getInstance()->findBuffer(ids[i]);
        GrAlwaysAssert(buffer);

        GrAlwaysAssert(!buffer->getDeleted());
        buffer->deleteAction();
    }
}

// map a buffer to the caller's address space
GrGLvoid* GR_GL_FUNCTION_TYPE debugGLMapBuffer(GrGLenum target, GrGLenum access) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);
    GrAlwaysAssert(GR_GL_WRITE_ONLY == access); // GR_GL_READ_ONLY == access ||  || GR_GL_READ_WRIT == access);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glMapBuffer");
            break;
    }

    if (buffer) {
        GrAlwaysAssert(!buffer->getMapped());
        buffer->setMapped();
        return buffer->getDataPtr();
    }

    GrAlwaysAssert(false);
    return NULL;        // no buffer bound to the target
}

// remove a buffer from the caller's address space
// TODO: check if the "access" method from "glMapBuffer" was honored 
GrGLboolean GR_GL_FUNCTION_TYPE debugGLUnmapBuffer(GrGLenum target) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;
        default:
            GrCrash("Unexpected target to glUnmapBuffer");
            break;
    }

    if (buffer) {
        GrAlwaysAssert(buffer->getMapped());
        buffer->resetMapped();
        return GR_GL_TRUE;
    }

    GrAlwaysAssert(false);
    return GR_GL_FALSE; // GR_GL_INVALID_OPERATION;
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetBufferParameteriv(GrGLenum target, GrGLenum value, GrGLint* params) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);
    GrAlwaysAssert(GR_GL_BUFFER_SIZE == value || GR_GL_BUFFER_USAGE == value);

    GrBufferObj *buffer = NULL;
    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getArrayBuffer();
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            buffer = GrDebugGL::getInstance()->getElementArrayBuffer();
            break;  
    }

    GrAlwaysAssert(buffer);

    switch (value) {
        case GR_GL_BUFFER_MAPPED:
            *params = GR_GL_FALSE;
            if (buffer)
                *params = buffer->getMapped() ? GR_GL_TRUE : GR_GL_FALSE;
            break;
        case GR_GL_BUFFER_SIZE:
            *params = 0;
            if (buffer)
                *params = buffer->getSize();
            break;
        case GR_GL_BUFFER_USAGE:
            *params = GR_GL_STATIC_DRAW;
            if (buffer)
                *params = buffer->getUsage();
            break;
        default:
            GrCrash("Unexpected value to glGetBufferParamateriv");
            break;
    }
};

GrGLenum GR_GL_FUNCTION_TYPE debugGLGetError() {
    return GR_GL_NO_ERROR;
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetIntegerv(GrGLenum pname, GrGLint* params) {
    // TODO: remove from Ganesh the #defines for gets we don't use.
    // We would like to minimize gets overall due to performance issues
    switch (pname) {
        case GR_GL_STENCIL_BITS:
            *params = 8;
            break;
        case GR_GL_SAMPLES:
            *params = 1;
            break;
        case GR_GL_FRAMEBUFFER_BINDING:
            *params = 0;
            break;
        case GR_GL_VIEWPORT:
            params[0] = 0;
            params[1] = 0;
            params[2] = 800;
            params[3] = 600;
            break;
        case GR_GL_MAX_TEXTURE_IMAGE_UNITS:
            *params = 8;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = 16;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
            *params = 16 * 4;
            break;
        case GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS:
            *params = 0;
            break;
        case GR_GL_COMPRESSED_TEXTURE_FORMATS:
            break;
        case GR_GL_MAX_TEXTURE_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_RENDERBUFFER_SIZE:
            *params = 8192;
            break;
        case GR_GL_MAX_SAMPLES:
            *params = 32;
            break;
        case GR_GL_MAX_VERTEX_ATTRIBS:
            *params = 16;
            break;
        case GR_GL_MAX_TEXTURE_UNITS:
            *params = GrDebugGL::getInstance()->getMaxTextureUnits();
            break;
        default:
            GrCrash("Unexpected pname to GetIntegerv");
    }
}
// used for both the program and shader info logs
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetInfoLog(GrGLuint program, GrGLsizei bufsize, GrGLsizei* length, char* infolog) {
    if (length) {
        *length = 0;
    }
    if (bufsize > 0) {
        *infolog = 0;
    }
}

// used for both the program and shader params
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetShaderOrProgramiv(GrGLuint program, GrGLenum pname, GrGLint* params) {
    switch (pname) {
        case GR_GL_LINK_STATUS:  // fallthru
        case GR_GL_COMPILE_STATUS:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_INFO_LOG_LENGTH:
            *params = 0;
            break;
        // we don't expect any other pnames
        default:
            GrCrash("Unexpected pname to GetProgramiv");
            break;
    }
}

namespace {
template <typename T>
void query_result(GrGLenum GLtarget, GrGLenum pname, T *params) {
    switch (pname) {
        case GR_GL_QUERY_RESULT_AVAILABLE:
            *params = GR_GL_TRUE;
            break;
        case GR_GL_QUERY_RESULT:
            *params = 0;
            break;
        default:
            GrCrash("Unexpected pname passed to GetQueryObject.");
            break;
    }
}
}

// Queries on the null GL just don't do anything at all. We could potentially make
// the timers work.
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryiv(GrGLenum GLtarget, GrGLenum pname, GrGLint *params) {
    switch (pname) {
        case GR_GL_CURRENT_QUERY:
            *params = 0;
            break;
        case GR_GL_QUERY_COUNTER_BITS:
            *params = 32;
            break;
        default:
            GrCrash("Unexpected pname passed GetQueryiv.");
    }
}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjecti64v(GrGLuint id, GrGLenum pname, GrGLint64 *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectiv(GrGLuint id, GrGLenum pname, GrGLint *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectui64v(GrGLuint id, GrGLenum pname, GrGLuint64 *params) {
    query_result(id, pname, params);
}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetQueryObjectuiv(GrGLuint id, GrGLenum pname, GrGLuint *params) {
    query_result(id, pname, params);
}

const GrGLubyte* GR_GL_FUNCTION_TYPE debugGLGetString(GrGLenum name) {
    switch (name) {
        case GR_GL_EXTENSIONS:
            return (const GrGLubyte*)"GL_ARB_framebuffer_object GL_ARB_blend_func_extended GL_ARB_timer_query GL_ARB_draw_buffers GL_ARB_occlusion_query GL_EXT_blend_color GL_EXT_stencil_wrap";
        case GR_GL_VERSION:
            return (const GrGLubyte*)"4.0 Null GL";
        case GR_GL_SHADING_LANGUAGE_VERSION:
            return (const GrGLubyte*)"4.20.8 Null GLSL";
        default:
            GrCrash("Unexpected name to GetString");
            return NULL;
    }
}

// we used to use this to query stuff about externally created textures, now we just
// require clients to tell us everything about the texture.
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetTexLevelParameteriv(GrGLenum target, GrGLint level, GrGLenum pname, GrGLint* params) {
    GrCrash("Should never query texture parameters.");
}

GrGLint GR_GL_FUNCTION_TYPE debugGLGetUniformLocation(GrGLuint program, const char* name) {
    static int gUniLocation = 0;
    return ++gUniLocation;
}

const GrGLInterface* GrGLCreateDebugInterface() {
    // The gl functions are not context-specific so we create one global 
    // interface
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        interface->fBindingsExported = kDesktop_GrGLBinding;
        interface->fActiveTexture = debugGLActiveTexture;
        interface->fAttachShader = debugGLAttachShader;
        interface->fBeginQuery = debugGLBeginQuery;
        interface->fBindAttribLocation = debugGLBindAttribLocation;
        interface->fBindBuffer = debugGLBindBuffer;
        interface->fBindFragDataLocation = debugGLBindFragDataLocation;
        interface->fBindTexture = debugGLBindTexture;
        interface->fBlendColor = debugGLBlendColor;
        interface->fBlendFunc = debugGLBlendFunc;
        interface->fBufferData = debugGLBufferData;
        interface->fBufferSubData = debugGLBufferSubData;
        interface->fClear = debugGLClear;
        interface->fClearColor = debugGLClearColor;
        interface->fClearStencil = debugGLClearStencil;
        interface->fColorMask = debugGLColorMask;
        interface->fCompileShader = debugGLCompileShader;
        interface->fCompressedTexImage2D = debugGLCompressedTexImage2D;
        interface->fCreateProgram = debugGLCreateProgram;
        interface->fCreateShader = debugGLCreateShader;
        interface->fCullFace = debugGLCullFace;
        interface->fDeleteBuffers = debugGLDeleteBuffers;
        interface->fDeleteProgram = debugGLDeleteProgram;
        interface->fDeleteQueries = debugGLDeleteIds;
        interface->fDeleteShader = debugGLDeleteShader;
        interface->fDeleteTextures = debugGLDeleteIds;
        interface->fDepthMask = debugGLDepthMask;
        interface->fDisable = debugGLDisable;
        interface->fDisableVertexAttribArray = debugGLDisableVertexAttribArray;
        interface->fDrawArrays = debugGLDrawArrays;
        interface->fDrawBuffer = debugGLDrawBuffer;
        interface->fDrawBuffers = debugGLDrawBuffers;
        interface->fDrawElements = debugGLDrawElements;
        interface->fEnable = debugGLEnable;
        interface->fEnableVertexAttribArray = debugGLEnableVertexAttribArray;
        interface->fEndQuery = debugGLEndQuery;
        interface->fFinish = debugGLFinish;
        interface->fFlush = debugGLFlush;
        interface->fFrontFace = debugGLFrontFace;
        interface->fGenBuffers = debugGLGenBuffers;
        interface->fGenQueries = debugGLGenIds;
        interface->fGenTextures = debugGLGenIds;
        interface->fGetBufferParameteriv = debugGLGetBufferParameteriv;
        interface->fGetError = debugGLGetError;
        interface->fGetIntegerv = debugGLGetIntegerv;
        interface->fGetQueryObjecti64v = debugGLGetQueryObjecti64v;
        interface->fGetQueryObjectiv = debugGLGetQueryObjectiv;
        interface->fGetQueryObjectui64v = debugGLGetQueryObjectui64v;
        interface->fGetQueryObjectuiv = debugGLGetQueryObjectuiv;
        interface->fGetQueryiv = debugGLGetQueryiv;
        interface->fGetProgramInfoLog = debugGLGetInfoLog;
        interface->fGetProgramiv = debugGLGetShaderOrProgramiv;
        interface->fGetShaderInfoLog = debugGLGetInfoLog;
        interface->fGetShaderiv = debugGLGetShaderOrProgramiv;
        interface->fGetString = debugGLGetString;
        interface->fGetTexLevelParameteriv = debugGLGetTexLevelParameteriv;
        interface->fGetUniformLocation = debugGLGetUniformLocation;
        interface->fLineWidth = debugGLLineWidth;
        interface->fLinkProgram = debugGLLinkProgram;
        interface->fPixelStorei = debugGLPixelStorei;
        interface->fQueryCounter = debugGLQueryCounter;
        interface->fReadBuffer = debugGLReadBuffer;
        interface->fReadPixels = debugGLReadPixels;
        interface->fScissor = debugGLScissor;
        interface->fShaderSource = debugGLShaderSource;
        interface->fStencilFunc = debugGLStencilFunc;
        interface->fStencilFuncSeparate = debugGLStencilFuncSeparate;
        interface->fStencilMask = debugGLStencilMask;
        interface->fStencilMaskSeparate = debugGLStencilMaskSeparate;
        interface->fStencilOp = debugGLStencilOp;
        interface->fStencilOpSeparate = debugGLStencilOpSeparate;
        interface->fTexImage2D = debugGLTexImage2D;
        interface->fTexParameteri = debugGLTexParameteri;
        interface->fTexSubImage2D = debugGLTexSubImage2D;
        interface->fTexStorage2D = debugGLTexStorage2D;
        interface->fUniform1f = debugGLUniform1f;
        interface->fUniform1i = debugGLUniform1i;
        interface->fUniform1fv = debugGLUniform1fv;
        interface->fUniform1iv = debugGLUniform1iv;
        interface->fUniform2f = debugGLUniform2f;
        interface->fUniform2i = debugGLUniform2i;
        interface->fUniform2fv = debugGLUniform2fv;
        interface->fUniform2iv = debugGLUniform2iv;
        interface->fUniform3f = debugGLUniform3f;
        interface->fUniform3i = debugGLUniform3i;
        interface->fUniform3fv = debugGLUniform3fv;
        interface->fUniform3iv = debugGLUniform3iv;
        interface->fUniform4f = debugGLUniform4f;
        interface->fUniform4i = debugGLUniform4i;
        interface->fUniform4fv = debugGLUniform4fv;
        interface->fUniform4iv = debugGLUniform4iv;
        interface->fUniformMatrix2fv = debugGLUniformMatrix2fv;
        interface->fUniformMatrix3fv = debugGLUniformMatrix3fv;
        interface->fUniformMatrix4fv = debugGLUniformMatrix4fv;
        interface->fUseProgram = debugGLUseProgram;
        interface->fVertexAttrib4fv = debugGLVertexAttrib4fv;
        interface->fVertexAttribPointer = debugGLVertexAttribPointer;
        interface->fViewport = debugGLViewport;
        interface->fBindFramebuffer = debugGLBindFramebuffer;
        interface->fBindRenderbuffer = debugGLBindRenderbuffer;
        interface->fCheckFramebufferStatus = debugGLCheckFramebufferStatus;
        interface->fDeleteFramebuffers = debugGLDeleteFramebuffers;
        interface->fDeleteRenderbuffers = debugGLDeleteRenderbuffers;
        interface->fFramebufferRenderbuffer = debugGLFramebufferRenderbuffer;
        interface->fFramebufferTexture2D = debugGLFramebufferTexture2D;
        interface->fGenFramebuffers = debugGLGenIds;
        interface->fGenRenderbuffers = debugGLGenIds;
        interface->fGetFramebufferAttachmentParameteriv = debugGLGetFramebufferAttachmentParameteriv;
        interface->fGetRenderbufferParameteriv = debugGLGetRenderbufferParameteriv;
        interface->fRenderbufferStorage = debugGLRenderbufferStorage;
        interface->fRenderbufferStorageMultisample = debugGLRenderbufferStorageMultisample;
        interface->fBlitFramebuffer = debugGLBlitFramebuffer;
        interface->fResolveMultisampleFramebuffer = debugGLResolveMultisampleFramebuffer;
        interface->fMapBuffer = debugGLMapBuffer;
        interface->fUnmapBuffer = debugGLUnmapBuffer;
        interface->fBindFragDataLocationIndexed = debugGLBindFragDataLocationIndexed;
    }
    glInterface.get()->ref();
    return glInterface.get();
}
