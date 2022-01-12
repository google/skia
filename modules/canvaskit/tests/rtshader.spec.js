describe('Runtime shader effects', () => {
    let container;

    beforeEach(async () => {
        await LoadCanvasKit;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    const spiralSkSL = `
uniform float rad_scale;
uniform int2   in_center;
uniform float4 in_colors0;
uniform float4 in_colors1;

half4 main(float2 p) {
    float2 pp = p - float2(in_center);
    float radius = sqrt(dot(pp, pp));
    radius = sqrt(radius);
    float angle = atan(pp.y / pp.x);
    float t = (angle + 3.1415926/2) / (3.1415926);
    t += radius * rad_scale;
    t = fract(t);
    return half4(mix(in_colors0, in_colors1, t));
}`;

    // TODO(kjlubick) rewrite testRTShader and callers to use gm.
    const testRTShader = (name, done, localMatrix) => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');
        if (!surface) {
            return;
        }
        const spiral = CanvasKit.RuntimeEffect.Make(spiralSkSL);
        expect(spiral).toBeTruthy('could not compile program');

        expect(spiral.getUniformCount()     ).toEqual(4);
        expect(spiral.getUniformFloatCount()).toEqual(11);
        const center = spiral.getUniform(1);
        expect(center).toBeTruthy('could not fetch numbered uniform');
        expect(center.slot     ).toEqual(1);
        expect(center.columns  ).toEqual(2);
        expect(center.rows     ).toEqual(1);
        expect(center.isInteger).toEqual(true);
        const color_0 = spiral.getUniform(2);
        expect(color_0).toBeTruthy('could not fetch numbered uniform');
        expect(color_0.slot     ).toEqual(3);
        expect(color_0.columns  ).toEqual(4);
        expect(color_0.rows     ).toEqual(1);
        expect(color_0.isInteger).toEqual(false);
        expect(spiral.getUniformName(2)).toEqual('in_colors0');

        const canvas = surface.getCanvas();
        const paint = new CanvasKit.Paint();
        canvas.clear(CanvasKit.BLACK); // black should not be visible
        const shader = spiral.makeShader([
            0.3,
            CANVAS_WIDTH/2, CANVAS_HEIGHT/2,
            1, 0, 0, 1, // solid red
            0, 1, 0, 1], // solid green
            true, /*=opaque*/
            localMatrix);
        paint.setShader(shader);
        canvas.drawRect(CanvasKit.LTRBRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT), paint);

        paint.delete();
        shader.delete();
        spiral.delete();

        reportSurface(surface, name, done);
    };

    it('can compile custom shader code', (done) => {
        testRTShader('rtshader_spiral', done);
    });

    it('can apply a matrix to the shader', (done) => {
        testRTShader('rtshader_spiral_translated', done, CanvasKit.Matrix.translated(-200, 100));
    });

    it('can provide a error handler for compilation errors', () => {
        let error = '';
        const spiral = CanvasKit.RuntimeEffect.Make(`invalid sksl code, I hope`, (e) => {
            error = e;
        });
        expect(spiral).toBeFalsy();
        expect(error).toContain('error');
    });

    it('can generate a debug trace', () => {
        // We don't support debug tracing on GPU, so we always request a software canvas here.
        const surface = CanvasKit.MakeSWCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');
        if (!surface) {
            return;
        }
        const spiral = CanvasKit.RuntimeEffect.Make(spiralSkSL);
        expect(spiral).toBeTruthy('could not compile program');

        const canvas = surface.getCanvas();
        const paint = new CanvasKit.Paint();
        const shader = spiral.makeShader([
            0.3,
            CANVAS_WIDTH/2, CANVAS_HEIGHT/2,
            1, 0, 0, 1, // solid red
            0, 1, 0, 1], // solid green
            true /*=opaque*/);

        const traced = CanvasKit.RuntimeEffect.MakeTraced(shader, CANVAS_WIDTH/2, CANVAS_HEIGHT/2);
        paint.setShader(traced.shader);
        canvas.drawRect(CanvasKit.LTRBRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT), paint);

        const traceData = traced.debugTrace.writeTrace();
        paint.delete();
        shader.delete();
        spiral.delete();
        traced.shader.delete();
        traced.debugTrace.delete();
        surface.delete();

        const parsedTrace = JSON.parse(traceData);
        expect(parsedTrace).toBeTruthy('could not parse trace JSON');
        expect(parsedTrace.functions).toBeTruthy('debug trace does not include function list');
        expect(parsedTrace.slots).toBeTruthy('debug trace does not include slot list');
        expect(parsedTrace.trace).toBeTruthy('debug trace does not include trace data');
        expect(parsedTrace.nonsense).toBeFalsy('debug trace includes a nonsense key');
        expect(parsedTrace.mystery).toBeFalsy('debug trace includes a mystery key');
        expect(parsedTrace.source).toEqual([
            "",
            "uniform float rad_scale;",
            "uniform int2   in_center;",
            "uniform float4 in_colors0;",
            "uniform float4 in_colors1;",
            "",
            "half4 main(float2 p) {",
            "    float2 pp = p - float2(in_center);",
            "    float radius = sqrt(dot(pp, pp));",
            "    radius = sqrt(radius);",
            "    float angle = atan(pp.y / pp.x);",
            "    float t = (angle + 3.1415926/2) / (3.1415926);",
            "    t += radius * rad_scale;",
            "    t = fract(t);",
            "    return half4(mix(in_colors0, in_colors1, t));",
            "}"
        ]);
    });

    const loadBrick = fetch(
        '/assets/brickwork-texture.jpg')
        .then((response) => response.arrayBuffer());
    const loadMandrill = fetch(
        '/assets/mandrill_512.png')
        .then((response) => response.arrayBuffer());

    const thresholdSkSL = `
uniform shader before_map;
uniform shader after_map;
uniform shader threshold_map;

uniform float cutoff;
uniform float slope;

float smooth_cutoff(float x) {
    x = x * slope + (0.5 - slope * cutoff);
    return clamp(x, 0, 1);
}

half4 main(float2 xy) {
    half4 before = before_map.eval(xy);
    half4 after = after_map.eval(xy);

    float m = smooth_cutoff(threshold_map.eval(xy).r);
    return mix(before, after, half(m));
}`;

    // TODO(kjlubick) rewrite testChildrenShader and callers to use gm.
    const testChildrenShader = (name, done, localMatrix) => {
        Promise.all([loadBrick, loadMandrill]).then((values) => {
            catchException(done, () => {
                const [brickData, mandrillData] = values;
                const brickImg = CanvasKit.MakeImageFromEncoded(brickData);
                expect(brickImg).toBeTruthy('brick image could not be loaded');
                const mandrillImg = CanvasKit.MakeImageFromEncoded(mandrillData);
                expect(mandrillImg).toBeTruthy('mandrill image could not be loaded');

                const thresholdEffect = CanvasKit.RuntimeEffect.Make(thresholdSkSL);
                expect(thresholdEffect).toBeTruthy('threshold did not compile');
                const spiralEffect = CanvasKit.RuntimeEffect.Make(spiralSkSL);
                expect(spiralEffect).toBeTruthy('spiral did not compile');

                const brickShader = brickImg.makeShaderCubic(
                    CanvasKit.TileMode.Decal, CanvasKit.TileMode.Decal,
                    1/3 /*B*/, 1/3 /*C*/,
                    CanvasKit.Matrix.scaled(CANVAS_WIDTH/brickImg.width(),
                                            CANVAS_HEIGHT/brickImg.height()));
                const mandrillShader = mandrillImg.makeShaderCubic(
                    CanvasKit.TileMode.Decal, CanvasKit.TileMode.Decal,
                    1/3 /*B*/, 1/3 /*C*/,
                    CanvasKit.Matrix.scaled(CANVAS_WIDTH/mandrillImg.width(),
                                            CANVAS_HEIGHT/mandrillImg.height()));
                const spiralShader = spiralEffect.makeShader([
                    0.8,
                    CANVAS_WIDTH/2, CANVAS_HEIGHT/2,
                    1, 1, 1, 1,
                    0, 0, 0, 1], true);

                const blendShader = thresholdEffect.makeShaderWithChildren(
                    [0.5, 5],
                    true, [brickShader, mandrillShader, spiralShader], localMatrix);

                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface');
                const canvas = surface.getCanvas();
                const paint = new CanvasKit.Paint();
                canvas.clear(CanvasKit.WHITE);

                paint.setShader(blendShader);
                canvas.drawRect(CanvasKit.LTRBRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT), paint);

                brickImg.delete();
                mandrillImg.delete();
                thresholdEffect.delete();
                spiralEffect.delete();
                brickShader.delete();
                mandrillShader.delete();
                spiralShader.delete();
                blendShader.delete();
                paint.delete();

                reportSurface(surface, name, done);
            })();
        });
    }

    it('take other shaders as fragment processors', (done) => {
        testChildrenShader('rtshader_children', done);
    });

    it('apply a local matrix to the children-based shader', (done) => {
        testChildrenShader('rtshader_children_rotated', done, CanvasKit.Matrix.rotated(Math.PI/12));
    });
});
