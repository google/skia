#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkCullPoints.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"

#include <AGL/agl.h>
#include <OpenGL/gl.h>

static void test_draw_gl(SkCanvas* canvas) {
    const float verts[] = {
        10, 10, 250, 250, 490, 10
    };
    const float texs[] = {
        0, 0, 0.5f, 0, 1, 1
    };
    const uint8_t colors[] = {
        0, 0, 0, 1,
        128, 0, 0, 1,
        255, 255, 0, 1
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//    glTexCoordPointer(2, GL_FLOAT, 0, texs);

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

static const char* gVertShaderText =
    "varying vec2 uv;"
    "void main(void) {"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    "    uv = vec2(gl_Color);"
    "}";

static const char* gFragShaderText =
    "varying vec2 uv;"
    "void main(void) {"
    "    float u = uv.x;"
    "    float v = uv.y;"
    "    if (u*u > v) {"
"    gl_FragColor = vec4(1.0, 0, 0, 1.0);"
    "    } else {"
    "    gl_FragColor = vec4(0, 1.0, 0, 1.0);"
    "}"
    "}";

static bool compile_shader(GLuint shader) {
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar buffer[256];
        glGetShaderInfoLog(shader, sizeof(buffer), NULL, buffer);
        SkDebugf("---- glCompileShader failed: %s\n", buffer);
        return false;
    }
    return true;
}

static bool link_program(GLuint program) {
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar buffer[256];
        glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);
        SkDebugf("---- glLinkProgram failed: %s\n", buffer);
        return false;
    }
    return true;
}

static void test_glshader(SkCanvas* canvas) {
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertShader, 1, &gVertShaderText, NULL);
    glShaderSource(fragShader, 1, &gFragShaderText, NULL);

    if (!compile_shader(vertShader)) { return; }
    if (!compile_shader(fragShader)) { return; }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    if (link_program(program)) {
        glUseProgram(program);
        test_draw_gl(canvas);
        glUseProgram(0);
    }
    glDeleteProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

static void setmatrix6(SkMatrix* matrix, const float val[]) {
    matrix->reset();
    for (int i = 0; i < 6; i++) {
        matrix->set(i, val[i]);
    }
}

static void testinvert() {
    SkMatrix matrix;

    const float vals[] = { 0,9,.000001,10000,0,0 };
    setmatrix6(&matrix, vals);

    const float vals2[] = { 0,100,71,9,0,7 };
    SkMatrix tmp;
    setmatrix6(&tmp, vals2);
    
    matrix.preConcat(tmp);
    matrix.dump();

    SkMatrix inverse;
    matrix.invert(&inverse);
    inverse.dump();
    
    matrix.preConcat(inverse);
    matrix.dump();

//    o2dContext.setTransform(0,9,.000001,10000,0,0);
//    o2dContext.transform(0,100,71,9,0,7);
//    o2dContext.setTransform(0,6,95,4,1,0);
}

/*
 [0]	9.9999997e-005	float
 [1]	9.0000003e-006	float
 [2]	7.0000001e-006	float
 [3]	1000000.0	float
 [4]	90639.000	float
 [5]	70000.000	float
 [6]	0.00000000	float
 [7]	0.00000000	float
 [8]	1.0000000	float
 */
static void testinvert2() {
    const float val[] = {
        9.9999997e-005, 9.0000003e-006, 7.0000001e-006,
        1000000.0, 90639.000, 70000.000
    };
    SkMatrix matrix;
    setmatrix6(&matrix, val);
    matrix.dump();

    SkMatrix inverse;
    matrix.invert(&inverse);
    inverse.dump();

    matrix.preConcat(inverse);
    matrix.dump();
    // result is that matrix[3] is 49550 instead of 0 :(
}

static void show_ramp(SkCanvas* canvas, const SkRect& r) {
    SkPoint pts[] = { r.fLeft, 0, r.fRight, 0 };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                 SkShader::kRepeat_TileMode);
    SkPaint p;
    p.setShader(s)->unref();
    canvas->drawRect(r, p);
    canvas->translate(r.width() + SkIntToScalar(8), 0);
    p.setDither(true);
    canvas->drawRect(r, p);
}

class TestGLView : public SkView {
public:
	TestGLView() {
        testinvert2();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "TestGL");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);
        
        test_glshader(canvas);
        return;
        
        SkRect r;
        r.set(0, 0, 100, 100);
        
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        
        SkPaint paint;
        paint.setAntiAlias(false);
        paint.setColor(SK_ColorRED);
        
        canvas->drawRect(r, paint);

        canvas->translate(r.width() + SkIntToScalar(20), 0);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, paint);

        canvas->translate(r.width() + SkIntToScalar(20), 0);
        paint.setStrokeWidth(SkIntToScalar(5));
        canvas->drawRect(r, paint);
        
        canvas->translate(r.width() * 10/9, 0);
        show_ramp(canvas, r);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TestGLView; }
static SkViewRegister reg(MyFactory);

