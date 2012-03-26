
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#include "SkTArray.h"
#include "SkTDArray.h"

// the OpenGLES 2.0 spec says this must be >= 128
static const GrGLint kDefaultMaxVertexUniformVectors = 128;

// the OpenGLES 2.0 spec says this must be >=16
static const GrGLint kDefaultMaxFragmentUniformVectors = 16;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVertexAttribs = 8;

// the OpenGLES 2.0 spec says this must be >= 8
static const GrGLint kDefaultMaxVaryingVectors = 8;

////////////////////////////////////////////////////////////////////////////////
// Each class derived from GrFakeRefObj should use this macro to add a
// factory creation entry point. This entry point is used by the GrGLDebug
// object to instantiate the various objects
// all globally unique IDs
#define GR_DEFINE_CREATOR(className)                        \
    public:                                                 \
    static GrFakeRefObj *create ## className() {            \
        return SkNEW(className);                            \
    }

////////////////////////////////////////////////////////////////////////////////
// Helper macro to make creating an object (where you need to get back a derived
// type) easier
#define GR_CREATE(className, classEnum)                     \
    reinterpret_cast<className *>(GrDebugGL::getInstance()->createObj(classEnum))

////////////////////////////////////////////////////////////////////////////////
// Helper macro to make finding objects less painful
#define GR_FIND(id, className, classEnum)                   \
    reinterpret_cast<className *>(GrDebugGL::getInstance()->findObject(id, classEnum))



////////////////////////////////////////////////////////////////////////////////
// This object is used to track the OpenGL objects. We don't use real
// reference counting (i.e., we don't free the objects when their ref count 
// goes to 0) so that we can detect invalid memory accesses. The refs we
// are tracking in this class are actually OpenGL's references to the objects
// not "ours"
// Each object also gets a unique globally identifying ID
class GrFakeRefObj {
public:
    GrFakeRefObj() 
        : fRef(0)
        , fHighRefCount(0)
        , fMarkedForDeletion(false)
        , fDeleted(false) {

        static int fNextID = 0;  // source for globally unique IDs - 0 is reserved!

        fID = ++fNextID;
    }
    virtual ~GrFakeRefObj() {};

    void ref() { 
        fRef++; 
        if (fHighRefCount < fRef) {
            fHighRefCount = fRef;
        }
    }
    void unref() { 
        fRef--; 
        GrAlwaysAssert(fRef >= 0);

        // often in OpenGL a given object may still be in use when the 
        // delete call is made. In these cases the object is marked
        // for deletion and then freed when it is no longer in use
        if (0 == fRef && fMarkedForDeletion) {
            this->deleteAction(); 
        }
    }
    int getRefCount() const { return fRef; }
    int getHighRefCount() const { return fHighRefCount; }

    GrGLuint getID() const { return fID; }

    void setMarkedForDeletion() { fMarkedForDeletion = true; }
    bool getMarkedForDeletion() const { return fMarkedForDeletion; }

    bool getDeleted() const     { return fDeleted; }

    // The deleteAction fires if the object has been marked for deletion but
    // couldn't be deleted earlier due to refs
    virtual void deleteAction() {
        this->setDeleted();
    }

protected:
private:
    int         fRef;
    int         fHighRefCount;      // high water mark of the ref count
    GrGLuint    fID;
    bool        fMarkedForDeletion;
    // The deleted flag is only set when OpenGL thinks the object is deleted
    // It is obviously still allocated w/in this framework
    bool        fDeleted;

    // setDeleted should only ever appear in the deleteAction method!
    void setDeleted()           { fDeleted = true; }
};

////////////////////////////////////////////////////////////////////////////////
class GrBufferObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrBufferObj);

public:
    GrBufferObj() 
        : GrFakeRefObj()
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

    void allocate(GrGLint size, const GrGLchar *dataPtr) {
        GrAlwaysAssert(size >= 0);

        // delete pre-existing data
        delete[] fDataPtr;

        fSize = size;
        fDataPtr = new GrGLchar[size];
        if (dataPtr) {
            memcpy(fDataPtr, dataPtr, fSize);
        }
        // TODO: w/ no dataPtr the data is unitialized - this could be tracked
    }
    GrGLint getSize() const { return fSize; }
    GrGLchar *getDataPtr()  { return fDataPtr; }

    GrGLint getUsage() const { return fUsage; }
    void setUsage(GrGLint usage) { fUsage = usage; }

    virtual void deleteAction() SK_OVERRIDE {

        // buffers are automatically unmapped when deleted
        this->resetMapped();

        this->INHERITED::deleteAction();
    }


protected:
private:

    GrGLchar*   fDataPtr;
    bool        fMapped;       // is the buffer object mapped via "glMapBuffer"?
    bool        fBound;        // is the buffer object bound via "glBindBuffer"?
    GrGLint     fSize;         // size in bytes
    GrGLint     fUsage;        // one of: GL_STREAM_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW

    typedef GrFakeRefObj INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
// A common base class for render buffers and textures
class GrFBBindable : public GrFakeRefObj {

public:
    GrFBBindable() 
        : GrFakeRefObj() {
    }

    virtual ~GrFBBindable() {
        GrAlwaysAssert(0 == fColorReferees.count());
        GrAlwaysAssert(0 == fDepthReferees.count());
        GrAlwaysAssert(0 == fStencilReferees.count());
    }

    void setColorBound(GrFakeRefObj *referee) { 
        fColorReferees.append(1, &referee);
    }
    void resetColorBound(GrFakeRefObj *referee) { 
        int index = fColorReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fColorReferees.removeShuffle(index);
    }
    bool getColorBound(GrFakeRefObj *referee) const { 
        int index = fColorReferees.find(referee);
        return 0 <= index;
    }
    bool getColorBound() const { 
        return 0 != fColorReferees.count();
    }

    void setDepthBound(GrFakeRefObj *referee) { 
        fDepthReferees.append(1, &referee);
    }
    void resetDepthBound(GrFakeRefObj *referee) { 
        int index = fDepthReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fDepthReferees.removeShuffle(index);
    }
    bool getDepthBound(GrFakeRefObj *referee) const { 
        int index = fDepthReferees.find(referee);
        return 0 <= index;
    }
    bool getDepthBound() const { 
        return 0 != fDepthReferees.count();
    }

    void setStencilBound(GrFakeRefObj *referee) { 
        fStencilReferees.append(1, &referee);
    }
    void resetStencilBound(GrFakeRefObj *referee) { 
        int index = fStencilReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fStencilReferees.removeShuffle(index);
    }
    bool getStencilBound(GrFakeRefObj *referee) const { 
        int index = fStencilReferees.find(referee);
        return 0 <= index;
    }
    bool getStencilBound() const { 
        return 0 != fStencilReferees.count();
    }


protected:
private:
    SkTDArray<GrFakeRefObj *> fColorReferees;   // frame buffers that use this as a color buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")
    SkTDArray<GrFakeRefObj *> fDepthReferees;   // frame buffers that use this as a depth buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")
    SkTDArray<GrFakeRefObj *> fStencilReferees; // frame buffers that use this as a stencil buffer (via "glFramebufferRenderbuffer" or "glFramebufferTexture2D")

    typedef GrFakeRefObj INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class GrRenderBufferObj : public GrFBBindable {
    GR_DEFINE_CREATOR(GrRenderBufferObj);

public:
    GrRenderBufferObj()
        : GrFBBindable()
        , fBound(false) {
    }

    void setBound()         { fBound = true; }
    void resetBound()       { fBound = false; }
    bool getBound() const   { return fBound; }

    virtual void deleteAction() SK_OVERRIDE {

        this->INHERITED::deleteAction();
    }

protected:
private:
    bool fBound;           // is this render buffer currently bound via "glBindRenderbuffer"?

    typedef GrFBBindable INHERITED;
};

class GrTextureUnitObj;

////////////////////////////////////////////////////////////////////////////////
class GrTextureObj : public GrFBBindable {
    GR_DEFINE_CREATOR(GrTextureObj);

public:
    GrTextureObj() 
        : GrFBBindable() {
    }

    virtual ~GrTextureObj() {
        GrAlwaysAssert(0 == fTextureUnitReferees.count());
    }

    void setBound(GrTextureUnitObj *referee) { 
        fTextureUnitReferees.append(1, &referee);
    }

    void resetBound(GrTextureUnitObj *referee) { 
        int index = fTextureUnitReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fTextureUnitReferees.removeShuffle(index);
    }
    bool getBound(GrTextureUnitObj *referee) const { 
        int index = fTextureUnitReferees.find(referee);
        return 0 <= index;
    }
    bool getBound() const { 
        return 0 != fTextureUnitReferees.count();
    }

    virtual void deleteAction() SK_OVERRIDE {

        this->INHERITED::deleteAction();
    }

protected:

private:
    SkTDArray<GrTextureUnitObj *> fTextureUnitReferees;   // texture units that bind this texture (via "glBindTexture")

    typedef GrFBBindable INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
// Although texture unit objects are allocated & deallocated like the other
// GL emulation objects they are derived from GrFakeRefObj to provide some
// uniformity in how the GrDebugGL class manages resources
class GrTextureUnitObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrTextureUnitObj);

public:
    GrTextureUnitObj()
        : GrFakeRefObj()
        , fNumber(0)
        , fTexture(NULL) {
    }

    void setNumber(GrGLenum number) {
        fNumber = number;
    }
    GrGLenum getNumber() const { return fNumber; }

    void setTexture(GrTextureObj *texture)  { 

        if (fTexture) {
            GrAlwaysAssert(fTexture->getBound(this));
            fTexture->resetBound(this);

            GrAlwaysAssert(!fTexture->getDeleted());
            fTexture->unref();
        }

        fTexture = texture; 

        if (fTexture) {
            GrAlwaysAssert(!fTexture->getDeleted());
            fTexture->ref();

            GrAlwaysAssert(!fTexture->getBound(this));
            fTexture->setBound(this);
        }
    }

    GrTextureObj *getTexture()                  { return fTexture; }

protected:
private:
    GrGLenum fNumber;
    GrTextureObj *fTexture;

    typedef GrFakeRefObj INHERITED;
};


////////////////////////////////////////////////////////////////////////////////
// TODO: when a framebuffer obj is bound the GL_SAMPLES query must return 0
// TODO: GL_STENCIL_BITS must also be redirected to the framebuffer
class GrFrameBufferObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrFrameBufferObj);

public:
    GrFrameBufferObj()
        : GrFakeRefObj()
        , fBound(false)
        , fColorBuffer(NULL)
        , fDepthBuffer(NULL)
        , fStencilBuffer(NULL) {
    }

    virtual ~GrFrameBufferObj() {
        fColorBuffer = NULL;
        fDepthBuffer = NULL;
        fStencilBuffer = NULL;
    }

    void setBound()         { fBound = true; }
    void resetBound()       { fBound = false; }
    bool getBound() const   { return fBound; }

    void setColor(GrFBBindable *buffer) {
        if (fColorBuffer) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fColorBuffer->getColorBound(this));
            fColorBuffer->resetColorBound(this);

            GrAlwaysAssert(!fColorBuffer->getDeleted());
            fColorBuffer->unref();
        }
        fColorBuffer = buffer;
        if (fColorBuffer) {
            GrAlwaysAssert(!fColorBuffer->getDeleted());
            fColorBuffer->ref();

            GrAlwaysAssert(!fColorBuffer->getColorBound(this));
            fColorBuffer->setColorBound(this);
        }
    }
    GrFBBindable *getColor()       { return fColorBuffer; }

    void setDepth(GrFBBindable *buffer) {
        if (fDepthBuffer) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fDepthBuffer->getDepthBound(this));
            fDepthBuffer->resetDepthBound(this);

            GrAlwaysAssert(!fDepthBuffer->getDeleted());
            fDepthBuffer->unref();
        }
        fDepthBuffer = buffer;
        if (fDepthBuffer) {
            GrAlwaysAssert(!fDepthBuffer->getDeleted());
            fDepthBuffer->ref();

            GrAlwaysAssert(!fDepthBuffer->getDepthBound(this));
            fDepthBuffer->setDepthBound(this);
        }
    }
    GrFBBindable *getDepth()       { return fDepthBuffer; }

    void setStencil(GrFBBindable *buffer) {
        if (fStencilBuffer) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fStencilBuffer->getStencilBound(this));
            fStencilBuffer->resetStencilBound(this);

            GrAlwaysAssert(!fStencilBuffer->getDeleted());
            fStencilBuffer->unref();
        }
        fStencilBuffer = buffer;
        if (fStencilBuffer) {
            GrAlwaysAssert(!fStencilBuffer->getDeleted());
            fStencilBuffer->ref();

            GrAlwaysAssert(!fStencilBuffer->getStencilBound(this));
            fStencilBuffer->setStencilBound(this);
        }
    }
    GrFBBindable *getStencil()     { return fStencilBuffer; }

    virtual void deleteAction() SK_OVERRIDE {

        setColor(NULL);
        setDepth(NULL);
        setStencil(NULL);

        this->INHERITED::deleteAction();
    }

protected:
private:
    bool fBound;        // is this frame buffer currently bound via "glBindFramebuffer"?
    GrFBBindable * fColorBuffer;
    GrFBBindable * fDepthBuffer;
    GrFBBindable * fStencilBuffer;

    typedef GrFakeRefObj INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
class GrShaderObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrShaderObj);

public:
    GrShaderObj() 
        : GrFakeRefObj()
        , fType(GR_GL_VERTEX_SHADER)   {}

    void setType(GrGLenum type) { fType = type; }
    GrGLenum getType()  { return fType; }

    virtual void deleteAction() SK_OVERRIDE {

        this->INHERITED::deleteAction();
    }

protected:
private:
    GrGLenum fType;  // either GR_GL_VERTEX_SHADER or GR_GL_FRAGMENT_SHADER

    typedef GrFakeRefObj INHERITED;
};


////////////////////////////////////////////////////////////////////////////////
class GrProgramObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrProgramObj);

public:
    GrProgramObj() 
        : GrFakeRefObj()
        , fInUse(false) {}

    void AttachShader(GrShaderObj *shader) {
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

        this->INHERITED::deleteAction();
    }

    // TODO: this flag system won't work w/ multiple contexts!
    void setInUse()         { fInUse = true; }
    void resetInUse()       { fInUse = false; }
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
class GrDebugGL {
public:
    typedef enum GrObjTypes {
        kTexture_ObjTypes = 0,
        kBuffer_ObjTypes,
        kRenderBuffer_ObjTypes,
        kFrameBuffer_ObjTypes,
        kShader_ObjTypes,
        kProgram_ObjTypes,
        kTextureUnit_ObjTypes,
        kObjTypeCount
    };

    GrFakeRefObj *createObj(GrObjTypes type) {
        GrFakeRefObj *temp = (*gFactoryFunc[type])();

        fObjects.push_back(temp);

        return temp;
    }

    GrFakeRefObj *findObject(GrGLuint ID, GrObjTypes type) {
        for (int i = 0; i < fObjects.count(); ++i) {
            if (fObjects[i]->getID() == ID) { // && fObjects[i]->getType() == type) {
                // The application shouldn't be accessing objects
                // that (as far as OpenGL knows) were already deleted 
                GrAlwaysAssert(!fObjects[i]->getDeleted());
                GrAlwaysAssert(!fObjects[i]->getMarkedForDeletion());
                return fObjects[i];
            }
        }

        return NULL;
    }

    GrGLuint getMaxTextureUnits() const { return kDefaultMaxTextureUnits; }

    void setCurTextureUnit(GrGLuint curTextureUnit) { fCurTextureUnit = curTextureUnit; }
    GrGLuint getCurTextureUnit() const { return fCurTextureUnit; }

    GrTextureUnitObj *getTextureUnit(int iUnit) {
        GrAlwaysAssert(0 <= iUnit && kDefaultMaxTextureUnits > iUnit);

        return fTextureUnits[iUnit];
    }

    void setArrayBuffer(GrBufferObj *arrayBuffer) { 
        if (fArrayBuffer) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fArrayBuffer->getBound());
            fArrayBuffer->resetBound();

            GrAlwaysAssert(!fArrayBuffer->getDeleted());
            fArrayBuffer->unref();
        }

        fArrayBuffer = arrayBuffer; 

        if (fArrayBuffer) {
            GrAlwaysAssert(!fArrayBuffer->getDeleted());
            fArrayBuffer->ref();

            GrAlwaysAssert(!fArrayBuffer->getBound());
            fArrayBuffer->setBound();
        }
    }
    GrBufferObj *getArrayBuffer()                   { return fArrayBuffer; }

    void setElementArrayBuffer(GrBufferObj *elementArrayBuffer) { 
        if (fElementArrayBuffer) {
            // automatically break the binding of the old buffer
            GrAlwaysAssert(fElementArrayBuffer->getBound());
            fElementArrayBuffer->resetBound();

            GrAlwaysAssert(!fElementArrayBuffer->getDeleted());
            fElementArrayBuffer->unref();
        }

        fElementArrayBuffer = elementArrayBuffer; 

        if (fElementArrayBuffer) {
            GrAlwaysAssert(!fElementArrayBuffer->getDeleted());
            fElementArrayBuffer->ref();

            GrAlwaysAssert(!fElementArrayBuffer->getBound());
            fElementArrayBuffer->setBound();
        }
    }

    GrBufferObj *getElementArrayBuffer()                            { return fElementArrayBuffer; }

    void setTexture(GrTextureObj *texture)  { 
        fTextureUnits[fCurTextureUnit]->setTexture(texture);
    }

    void setFrameBuffer(GrFrameBufferObj *frameBuffer)  { 
        if (fFrameBuffer) {
            GrAlwaysAssert(fFrameBuffer->getBound());
            fFrameBuffer->resetBound();

            GrAlwaysAssert(!fFrameBuffer->getDeleted());
            fFrameBuffer->unref();
        }

        fFrameBuffer = frameBuffer; 

        if (fFrameBuffer) {
            GrAlwaysAssert(!fFrameBuffer->getDeleted());
            fFrameBuffer->ref();

            GrAlwaysAssert(!fFrameBuffer->getBound());
            fFrameBuffer->setBound();
        }
    }

    GrFrameBufferObj *getFrameBuffer()                  { return fFrameBuffer; }

    void setRenderBuffer(GrRenderBufferObj *renderBuffer)  { 
        if (fRenderBuffer) {
            GrAlwaysAssert(fRenderBuffer->getBound());
            fRenderBuffer->resetBound();

            GrAlwaysAssert(!fRenderBuffer->getDeleted());
            fRenderBuffer->unref();
        }

        fRenderBuffer = renderBuffer; 

        if (fRenderBuffer) {
            GrAlwaysAssert(!fRenderBuffer->getDeleted());
            fRenderBuffer->ref();

            GrAlwaysAssert(!fRenderBuffer->getBound());
            fRenderBuffer->setBound();
        }
    }

    GrRenderBufferObj *getRenderBuffer()                  { return fRenderBuffer; }

    void useProgram(GrProgramObj *program) {
        if (fProgram) {
            GrAlwaysAssert(fProgram->getInUse());
            fProgram->resetInUse();

            GrAlwaysAssert(!fProgram->getDeleted());
            fProgram->unref();
        }

        fProgram = program;

        if (fProgram) {
            GrAlwaysAssert(!fProgram->getDeleted());
            fProgram->ref();

            GrAlwaysAssert(!fProgram->getInUse());
            fProgram->setInUse();
        }
    }

    static GrDebugGL *getInstance() {
//        static GrDebugGL Obj;

        return &Obj;
    }

    void report() const {
        for (int i = 0; i < fObjects.count(); ++i) {
            GrAlwaysAssert(0 == fObjects[i]->getRefCount());
            GrAlwaysAssert(0 < fObjects[i]->getHighRefCount());
            GrAlwaysAssert(fObjects[i]->getDeleted());
        }
    }

protected:

private:
    // the OpenGLES 2.0 spec says this must be >= 2
    static const GrGLint kDefaultMaxTextureUnits = 8;

    GrGLuint        fMaxTextureUnits;
    GrGLuint        fCurTextureUnit;
    GrBufferObj *   fArrayBuffer;
    GrBufferObj *   fElementArrayBuffer;
    GrFrameBufferObj *fFrameBuffer;
    GrRenderBufferObj *fRenderBuffer;
    GrProgramObj *  fProgram;
    GrTextureObj *  fTexture;
    GrTextureUnitObj *fTextureUnits[kDefaultMaxTextureUnits];

    typedef GrFakeRefObj *(*Create)();

    static Create gFactoryFunc[kObjTypeCount];

    static GrDebugGL Obj;

    // global store of all objects
    SkTArray<GrFakeRefObj *> fObjects;

    GrDebugGL() 
        : fCurTextureUnit(0)
        , fArrayBuffer(NULL)
        , fElementArrayBuffer(NULL)
        , fFrameBuffer(NULL)
        , fRenderBuffer(NULL)
        , fProgram(NULL)
        , fTexture(NULL) {

        for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {
            fTextureUnits[i] = GR_CREATE(GrTextureUnitObj, GrDebugGL::kTextureUnit_ObjTypes);
            fTextureUnits[i]->ref();

            fTextureUnits[i]->setNumber(i);
        }
    }

    ~GrDebugGL() {
        // unref & delete the texture units first so they don't show up on the leak report
        for (int i = 0; i < kDefaultMaxTextureUnits; ++i) {
            fTextureUnits[i]->unref();
            fTextureUnits[i]->deleteAction();
        }

        this->report();

        for (int i = 0; i < fObjects.count(); ++i) {
            delete fObjects[i];
        }
        fObjects.reset();

        fArrayBuffer = NULL;
        fElementArrayBuffer = NULL;
        fFrameBuffer = NULL;
        fRenderBuffer = NULL;
        fProgram = NULL;
        fTexture = NULL;
    }
};

GrDebugGL GrDebugGL::Obj;
GrDebugGL::Create GrDebugGL::gFactoryFunc[kObjTypeCount] = {
    GrTextureObj::createGrTextureObj,
    GrBufferObj::createGrBufferObj,
    GrRenderBufferObj::createGrRenderBufferObj,
    GrFrameBufferObj::createGrFrameBufferObj,
    GrShaderObj::createGrShaderObj,
    GrProgramObj::createGrProgramObj,
    GrTextureUnitObj::createGrTextureUnitObj,
};


////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLActiveTexture(GrGLenum texture) {
    
    // Ganesh offsets the texture unit indices
    texture -= GR_GL_TEXTURE0;
    GrAlwaysAssert(texture < GrDebugGL::getInstance()->getMaxTextureUnits());

    GrDebugGL::getInstance()->setCurTextureUnit(texture);
}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLAttachShader(GrGLuint programID, GrGLuint shaderID) {

    GrProgramObj *program = GR_FIND(programID, GrProgramObj, GrDebugGL::kProgram_ObjTypes);
    GrAlwaysAssert(program);

    GrShaderObj *shader = GR_FIND(shaderID, GrShaderObj, GrDebugGL::kShader_ObjTypes);
    GrAlwaysAssert(shader);

    program->AttachShader(shader);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBeginQuery(GrGLenum target, GrGLuint id) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindAttribLocation(GrGLuint program, GrGLuint index, const char* name) {}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindTexture(GrGLenum target, GrGLuint textureID) {

    // we don't use cube maps
    GrAlwaysAssert(target == GR_GL_TEXTURE_2D); // || target == GR_GL_TEXTURE_CUBE_MAP);

    // a textureID of 0 is acceptable - it binds to the default texture target
    GrTextureObj *texture = GR_FIND(textureID, GrTextureObj, GrDebugGL::kTexture_ObjTypes);

    GrDebugGL::getInstance()->setTexture(texture);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendColor(GrGLclampf red, GrGLclampf green, GrGLclampf blue, GrGLclampf alpha) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocation(GrGLuint program, GrGLuint colorNumber, const GrGLchar* name) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlendFunc(GrGLenum sfactor, GrGLenum dfactor) {}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBufferData(GrGLenum target, GrGLsizeiptr size, const GrGLvoid* data, GrGLenum usage) {
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

    buffer->allocate(size, reinterpret_cast<const GrGLchar *>(data));
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

    // A programID of 0 is legal
    GrProgramObj *program = GR_FIND(programID, GrProgramObj, GrDebugGL::kProgram_ObjTypes);

    GrDebugGL::getInstance()->useProgram(program);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttrib4fv(GrGLuint indx, const GrGLfloat* values) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLVertexAttribPointer(GrGLuint indx, GrGLint size, GrGLenum type, GrGLboolean normalized, GrGLsizei stride, const GrGLvoid* ptr) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLViewport(GrGLint x, GrGLint y, GrGLsizei width, GrGLsizei height) {}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFramebuffer(GrGLenum target, GrGLuint frameBufferID) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);

    // a frameBufferID of 0 is acceptable - it binds to the default frame buffer
    GrFrameBufferObj *frameBuffer = GR_FIND(frameBufferID, GrFrameBufferObj, GrDebugGL::kFrameBuffer_ObjTypes);

    GrDebugGL::getInstance()->setFrameBuffer(frameBuffer);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindRenderbuffer(GrGLenum target, GrGLuint renderBufferID) {

    GrAlwaysAssert(GR_GL_RENDERBUFFER == target);

    // a renderBufferID of 0 is acceptable - it unbinds the bound render buffer
    GrRenderBufferObj *renderBuffer = GR_FIND(renderBufferID, GrRenderBufferObj, GrDebugGL::kRenderBuffer_ObjTypes);

    GrDebugGL::getInstance()->setRenderBuffer(renderBuffer);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteTextures(GrGLsizei n, const GrGLuint* textures) {

    // first potentially unbind the texture
    // TODO: move this into GrDebugGL as unBindTexture?
    for (unsigned int i = 0; i < GrDebugGL::getInstance()->getMaxTextureUnits(); ++i)
    {
        GrTextureUnitObj *pTU = GrDebugGL::getInstance()->getTextureUnit(i);

        if (pTU->getTexture()) {
            for (int j = 0; j < n; ++j) {

                if (textures[j] == pTU->getTexture()->getID()) {
                    // this ID is the current texture - revert the binding to 0
                    pTU->setTexture(NULL);
                }
            }
        }
    }

    // TODO: fuse the following block with DeleteRenderBuffers?
    // Open GL will remove a deleted render buffer from the active frame buffer but not
    // from any other frame buffer
    if (GrDebugGL::getInstance()->getFrameBuffer()) {

        GrFrameBufferObj *frameBuffer = GrDebugGL::getInstance()->getFrameBuffer();

        for (int i = 0; i < n; ++i) {

            if (NULL != frameBuffer->getColor() && textures[i] == frameBuffer->getColor()->getID()) {
                frameBuffer->setColor(NULL);
            } 
            if (NULL != frameBuffer->getDepth() && textures[i] == frameBuffer->getDepth()->getID()) {
                frameBuffer->setDepth(NULL);
            }
            if (NULL != frameBuffer->getStencil() && textures[i] == frameBuffer->getStencil()->getID()) {
                frameBuffer->setStencil(NULL);
            }
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrTextureObj *buffer = GR_FIND(textures[i], GrTextureObj, GrDebugGL::kTexture_ObjTypes);
        GrAlwaysAssert(buffer);

        // OpenGL gives no guarantees if a texture is deleted while attached to
        // something other than the currently bound frame buffer
        GrAlwaysAssert(!buffer->getBound());

        GrAlwaysAssert(!buffer->getDeleted());
        buffer->deleteAction();
    }

}


GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteFramebuffers(GrGLsizei n, const GrGLuint *frameBuffers) {

    // first potentially unbind the buffers
    if (GrDebugGL::getInstance()->getFrameBuffer()) {
        for (int i = 0; i < n; ++i) {

            if (frameBuffers[i] == GrDebugGL::getInstance()->getFrameBuffer()->getID()) {
                // this ID is the current frame buffer - rebind to the default
                GrDebugGL::getInstance()->setFrameBuffer(NULL);
            }
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrFrameBufferObj *buffer = GR_FIND(frameBuffers[i], GrFrameBufferObj, GrDebugGL::kFrameBuffer_ObjTypes);
        GrAlwaysAssert(buffer);

        GrAlwaysAssert(!buffer->getDeleted());
        buffer->deleteAction();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteRenderbuffers(GrGLsizei n, const GrGLuint *renderBuffers) {

    // first potentially unbind the buffers
    if (GrDebugGL::getInstance()->getRenderBuffer()) {
        for (int i = 0; i < n; ++i) {

            if (renderBuffers[i] == GrDebugGL::getInstance()->getRenderBuffer()->getID()) {
                // this ID is the current render buffer - make no render buffer be bound
                GrDebugGL::getInstance()->setRenderBuffer(NULL);
            }
        }
    }

    // TODO: fuse the following block with DeleteTextures?
    // Open GL will remove a deleted render buffer from the active frame buffer but not
    // from any other frame buffer
    if (GrDebugGL::getInstance()->getFrameBuffer()) {

        GrFrameBufferObj *frameBuffer = GrDebugGL::getInstance()->getFrameBuffer();

        for (int i = 0; i < n; ++i) {

            if (NULL != frameBuffer->getColor() && renderBuffers[i] == frameBuffer->getColor()->getID()) {
                frameBuffer->setColor(NULL);
            } 
            if (NULL != frameBuffer->getDepth() && renderBuffers[i] == frameBuffer->getDepth()->getID()) {
                frameBuffer->setDepth(NULL);
            }
            if (NULL != frameBuffer->getStencil() && renderBuffers[i] == frameBuffer->getStencil()->getID()) {
                frameBuffer->setStencil(NULL);
            }
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrRenderBufferObj *buffer = GR_FIND(renderBuffers[i], GrRenderBufferObj, GrDebugGL::kRenderBuffer_ObjTypes);
        GrAlwaysAssert(buffer);

        // OpenGL gives no guarantees if a render buffer is deleted while attached to
        // something other than the currently bound frame buffer
        GrAlwaysAssert(!buffer->getColorBound());
        GrAlwaysAssert(!buffer->getDepthBound());
        GrAlwaysAssert(!buffer->getStencilBound());

        GrAlwaysAssert(!buffer->getDeleted());
        buffer->deleteAction();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferRenderbuffer(GrGLenum target, GrGLenum attachment, GrGLenum renderbuffertarget, GrGLuint renderBufferID) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
    GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment || 
                   GR_GL_DEPTH_ATTACHMENT == attachment || 
                   GR_GL_STENCIL_ATTACHMENT == attachment);
    GrAlwaysAssert(GR_GL_RENDERBUFFER == renderbuffertarget);

    GrFrameBufferObj *framebuffer = GrDebugGL::getInstance()->getFrameBuffer();
    // A render buffer cannot be attached to the default framebuffer
    GrAlwaysAssert(NULL != framebuffer);

    // a renderBufferID of 0 is acceptable - it unbinds the current render buffer
    GrRenderBufferObj *renderbuffer = GR_FIND(renderBufferID, GrRenderBufferObj, GrDebugGL::kRenderBuffer_ObjTypes);

    switch (attachment) {
    case GR_GL_COLOR_ATTACHMENT0:
        framebuffer->setColor(renderbuffer);
        break;
    case GR_GL_DEPTH_ATTACHMENT:
        framebuffer->setDepth(renderbuffer);
        break;
    case GR_GL_STENCIL_ATTACHMENT:
        framebuffer->setStencil(renderbuffer);
        break;
    default:
        GrAlwaysAssert(false);
        break;
    };

}

////////////////////////////////////////////////////////////////////////////////
GrGLvoid GR_GL_FUNCTION_TYPE debugGLFramebufferTexture2D(GrGLenum target, GrGLenum attachment, GrGLenum textarget, GrGLuint textureID, GrGLint level) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);
    GrAlwaysAssert(GR_GL_COLOR_ATTACHMENT0 == attachment || 
                   GR_GL_DEPTH_ATTACHMENT == attachment || 
                   GR_GL_STENCIL_ATTACHMENT == attachment);
    GrAlwaysAssert(GR_GL_TEXTURE_2D == textarget);

    GrFrameBufferObj *framebuffer = GrDebugGL::getInstance()->getFrameBuffer();
    // A texture cannot be attached to the default framebuffer
    GrAlwaysAssert(NULL != framebuffer);

    // A textureID of 0 is allowed - it unbinds the currently bound texture
    GrTextureObj *texture = GR_FIND(textureID, GrTextureObj, GrDebugGL::kTexture_ObjTypes);
    if (texture) {
        // The texture shouldn't be bound to a texture unit - this could lead to a feedback loop
        GrAlwaysAssert(!texture->getBound());
    }

    GrAlwaysAssert(0 == level);

    switch (attachment) {
    case GR_GL_COLOR_ATTACHMENT0:
        framebuffer->setColor(texture);
        break;
    case GR_GL_DEPTH_ATTACHMENT:
        framebuffer->setDepth(texture);
        break;
    case GR_GL_STENCIL_ATTACHMENT:
        framebuffer->setStencil(texture);
        break;
    default:
        GrAlwaysAssert(false);
        break;
    };
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetFramebufferAttachmentParameteriv(GrGLenum target, GrGLenum attachment, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLGetRenderbufferParameteriv(GrGLenum target, GrGLenum pname, GrGLint* params) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorage(GrGLenum target, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLRenderbufferStorageMultisample(GrGLenum target, GrGLsizei samples, GrGLenum internalformat, GrGLsizei width, GrGLsizei height) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBlitFramebuffer(GrGLint srcX0, GrGLint srcY0, GrGLint srcX1, GrGLint srcY1, GrGLint dstX0, GrGLint dstY0, GrGLint dstX1, GrGLint dstY1, GrGLbitfield mask, GrGLenum filter) {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLResolveMultisampleFramebuffer() {}
GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindFragDataLocationIndexed(GrGLuint program, GrGLuint colorNumber, GrGLuint index, const GrGLchar * name) {}

GrGLenum GR_GL_FUNCTION_TYPE debugGLCheckFramebufferStatus(GrGLenum target) {

    GrAlwaysAssert(GR_GL_FRAMEBUFFER == target);

    return GR_GL_FRAMEBUFFER_COMPLETE;
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateProgram() {

    GrProgramObj *program = GR_CREATE(GrProgramObj, GrDebugGL::kProgram_ObjTypes);

    return program->getID();
}

GrGLuint GR_GL_FUNCTION_TYPE debugGLCreateShader(GrGLenum type) {
    
    GrAlwaysAssert(GR_GL_VERTEX_SHADER == type || GR_GL_FRAGMENT_SHADER == type);

    GrShaderObj *shader = GR_CREATE(GrShaderObj, GrDebugGL::kShader_ObjTypes);
    shader->setType(type);

    return shader->getID();
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteProgram(GrGLuint programID) {

    GrProgramObj *program = GR_FIND(programID, GrProgramObj, GrDebugGL::kProgram_ObjTypes);
    GrAlwaysAssert(program);

    if (program->getRefCount()) {
        // someone is still using this program so we can't delete it here
        program->setMarkedForDeletion();
    } else {
        program->deleteAction();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteShader(GrGLuint shaderID) {

    GrShaderObj *shader = GR_FIND(shaderID, GrShaderObj, GrDebugGL::kShader_ObjTypes);
    GrAlwaysAssert(shader);

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

GrGLvoid debugGenObjs(GrDebugGL::GrObjTypes type, GrGLsizei n, GrGLuint* ids) {

   for (int i = 0; i < n; ++i) {
        GrFakeRefObj *obj = GrDebugGL::getInstance()->createObj(type);
        GrAlwaysAssert(obj);
        ids[i] = obj->getID();
    }
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenBuffers(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenFramebuffers(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kFrameBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenRenderbuffers(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kRenderBuffer_ObjTypes, n, ids);
}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLGenTextures(GrGLsizei n, GrGLuint* ids) {

    debugGenObjs(GrDebugGL::kTexture_ObjTypes, n, ids);
}

// same delete function for all glDelete*(GLsize i, const GLuint*) except buffers
GrGLvoid GR_GL_FUNCTION_TYPE debugGLDeleteIds(GrGLsizei n, const GrGLuint* ids) {}

GrGLvoid GR_GL_FUNCTION_TYPE debugGLBindBuffer(GrGLenum target, GrGLuint bufferID) {

    GrAlwaysAssert(GR_GL_ARRAY_BUFFER == target || GR_GL_ELEMENT_ARRAY_BUFFER == target);

    GrBufferObj *buffer = GR_FIND(bufferID, GrBufferObj, GrDebugGL::kBuffer_ObjTypes);
    // 0 is a permissable bufferID - it unbinds the current buffer

    switch (target) {
        case GR_GL_ARRAY_BUFFER:
            GrDebugGL::getInstance()->setArrayBuffer(buffer);
            break;
        case GR_GL_ELEMENT_ARRAY_BUFFER:
            GrDebugGL::getInstance()->setElementArrayBuffer(buffer);
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
            GrDebugGL::getInstance()->setArrayBuffer(NULL);
        }
        if (GrDebugGL::getInstance()->getElementArrayBuffer() && 
            ids[i] == GrDebugGL::getInstance()->getElementArrayBuffer()->getID()) {
            // this ID is the current element array buffer
            GrDebugGL::getInstance()->setElementArrayBuffer(NULL);
        }
    }

    // then actually "delete" the buffers
    for (int i = 0; i < n; ++i) {
        GrBufferObj *buffer = GR_FIND(ids[i], GrBufferObj, GrDebugGL::kBuffer_ObjTypes);
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
        case GR_GL_MAX_VERTEX_UNIFORM_VECTORS:
            *params = kDefaultMaxVertexUniformVectors;
            break;
        case GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = kDefaultMaxFragmentUniformVectors;
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
            *params = kDefaultMaxVertexAttribs;
            break;
        case GR_GL_MAX_VARYING_VECTORS:
            *params = kDefaultMaxVaryingVectors;
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

////////////////////////////////////////////////////////////////////////////////
struct GrDebugGLInterface : public GrGLInterface {

public:
    GrDebugGLInterface()
        : fWrapped(NULL) {
    }

    void setWrapped(GrGLInterface *interface) {
        fWrapped.reset(interface);
    }

    // TODO: there are some issues w/ wrapping another GL interface inside the
    // debug interface:
    //      Since none of the "gl" methods are member functions they don't get
    //      a "this" pointer through which to access "fWrapped"
    //      This could be worked around by having all of them access the 
    //      "glInterface" pointer - i.e., treating the debug interface as a 
    //      true singleton
    //
    //      The problem with this is that we also want to handle OpenGL 
    //      contexts. The natural way to do this is to have multiple debug 
    //      interfaces. Each of which represents a separate context. The 
    //      static ID count would still uniquify IDs across all of them. 
    //      The problem then is that we couldn't treat the debug GL 
    //      interface as a singleton (since there would be one for each 
    //      context).
    //
    //      The solution to this is probably to alter SkDebugGlContext's 
    //      "makeCurrent" method to make a call like "makeCurrent(this)" to 
    //      the debug GL interface (assuming that the application will create 
    //      multiple SkGLContext's) to let it switch between the active 
    //      context. Everything in the GrDebugGL object would then need to be 
    //      moved to a GrContextObj and the GrDebugGL object would just switch 
    //      between them. Note that this approach would also require that 
    //      SkDebugGLContext wrap an arbitrary other context
    //      and then pass the wrapped interface to the debug GL interface.

protected:
private:

    SkAutoTUnref<GrGLInterface> fWrapped;

    typedef GrGLInterface INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
const GrGLInterface* GrGLCreateDebugInterface() {
    // The gl functions are not context-specific so we create one global 
    // interface
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrDebugGLInterface;
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
        interface->fDeleteTextures = debugGLDeleteTextures;
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
        interface->fGenTextures = debugGLGenTextures;
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
        interface->fGenFramebuffers = debugGLGenFramebuffers;
        interface->fGenRenderbuffers = debugGLGenRenderbuffers;
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
