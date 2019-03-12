describe('PathKit\'s SVG Behavior', function() {
    it('can create a path from an SVG string', function(done) {
        LoadPathKit.then(catchException(done, () => {
            //.This is a parallelagram from
            // https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_parallelogram.svg
            let path = PathKit.FromSVGString('M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');

            let cmds = path.toCmds();
            expect(cmds).toBeTruthy();
            // 1 move, 4 lines, 1 close
            // each element in cmds is an array, with index 0 being the verb, and the rest being args
            expect(cmds.length).toBe(6);
            expect(cmds).toEqual([[PathKit.MOVE_VERB, 205, 5],
                                  [PathKit.LINE_VERB, 795, 5],
                                  [PathKit.LINE_VERB, 595, 295],
                                  [PathKit.LINE_VERB, 5, 295],
                                  [PathKit.LINE_VERB, 205, 5],
                                  [PathKit.CLOSE_VERB]]);
            path.delete();
            done();
        }));
    });

    it('can create an SVG string from a path', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let cmds = [[PathKit.MOVE_VERB, 205, 5],
                       [PathKit.LINE_VERB, 795, 5],
                       [PathKit.LINE_VERB, 595, 295],
                       [PathKit.LINE_VERB, 5, 295],
                       [PathKit.LINE_VERB, 205, 5],
                       [PathKit.CLOSE_VERB]];
            let path = PathKit.FromCmds(cmds);

            let svgStr = path.toSVGString();
            // We output it in terse form, which is different than Wikipedia's version
            expect(svgStr).toEqual('M205 5L795 5L595 295L5 295L205 5Z');
            path.delete();
            done();
        }));
    });

    it('can create an SVG string from hex values', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let cmds = [[PathKit.MOVE_VERB, "0x15e80300", "0x400004dc"], // 9.37088e-26f, 2.0003f
                       [PathKit.LINE_VERB, 795, 5],
                       [PathKit.LINE_VERB, 595, 295],
                       [PathKit.LINE_VERB, 5, 295],
                       [PathKit.LINE_VERB, "0x15e80300", "0x400004dc"], // 9.37088e-26f, 2.0003f
                       [PathKit.CLOSE_VERB]];
            let path = PathKit.FromCmds(cmds);

            let svgStr = path.toSVGString();
            expect(svgStr).toEqual('M9.37088e-26 2.0003L795 5L595 295L5 295L9.37088e-26 2.0003Z');
            path.delete();
            done();
        }));
    });

    it('should have input and the output be the same', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let testCases = [
                'M0 0L1075 0L1075 242L0 242L0 0Z'
            ];

            for(let svg of testCases) {
                let path = PathKit.FromSVGString(svg);
                let output = path.toSVGString();

                expect(svg).toEqual(output);

                path.delete();
            }
            done();
        }));
    });

    it('approximates arcs (conics) with quads', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let path = PathKit.NewPath();
            path.moveTo(50, 120);
            path.arc(50, 120, 45, 0, 1.75 * Math.PI);
            path.lineTo(50, 120);
            let svgStr = path.toSVGString();
            // Q stands for quad.  No need to check the whole path, as that's more
            // what the gold correctness tests are for (can account for changes we make
            // to the approximation algorithms).
            expect(svgStr).toContain('Q');
            path.delete();

             reportSVGString(svgStr, 'conics_quads_approx').then(() => {
                done();
            }).catch(reportError(done));
        }));
    });

});
