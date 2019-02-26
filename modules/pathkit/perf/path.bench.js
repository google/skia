

describe('PathKit\'s Path Behavior', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject) {
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/pathkit/'+file,
            }).ready().then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    function drawPath() {
        let path = PathKit.NewPath();
        path.moveTo(20, 5);
        path.lineTo(30, 20);
        path.lineTo(40, 10);
        path.lineTo(50, 20);
        path.lineTo(60, 0);
        path.lineTo(20, 5);

        path.moveTo(20, 80);
        path.bezierCurveTo(90, 10, 160, 150, 190, 10);

        path.moveTo(36, 148);
        path.quadraticCurveTo(66, 188, 120, 136);
        path.lineTo(36, 148);

        path.rect(5, 170, 20, 20);

        path.moveTo(150, 180);
        path.arcTo(150, 100, 50, 200, 20);
        path.lineTo(160, 160);

        path.moveTo(20, 120);
        path.arc(20, 120, 18, 0, 1.75 * Math.PI);
        path.lineTo(20, 120);

        let secondPath = PathKit.NewPath();
        secondPath.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI, false);

        path.addPath(secondPath);

        let m = document.createElementNS('http://www.w3.org/2000/svg', 'svg').createSVGMatrix();
        m.a = 1; m.b = 0;
        m.c = 0; m.d = 1;
        m.e = 0; m.f = 20.5;

        path.addPath(secondPath, m);
        secondPath.delete();
        return path;
    }

    it('path_path2dapi', function(done) {
        function setup(ctx) { }

        function test(ctx) {
            path = drawPath();
            path.delete();
        }

        function teardown(ctx) { }

        LoadPathKit.then(() => {
            benchmarkAndReport('path_path2dapi', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    describe('import options', function() {
        it('path_copy', function(done) {
            function setup(ctx) {
                ctx.path = PathKit.FromSVGString('M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');
            }

            function test(ctx) {
                let p = ctx.path.copy();
                p.delete();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_copy', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_from_api_calls', function(done) {
            function setup(ctx) { }

            function test(ctx) {
                let p = PathKit.NewPath()
                               .moveTo(205, 5)
                               .lineTo(795, 5)
                               .lineTo(595, 295)
                               .lineTo(5, 295)
                               .lineTo(205, 5)
                               .close();
                p.delete();
            }

            function teardown(ctx) { }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_from_api_calls', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_fromCmds', function(done) {
            function setup(ctx) { }

            function test(ctx) {
                let p = PathKit.FromCmds(
                    [[PathKit.MOVE_VERB, 205, 5],
                    [PathKit.LINE_VERB, 795, 5],
                    [PathKit.LINE_VERB, 595, 295],
                    [PathKit.LINE_VERB, 5, 295],
                    [PathKit.LINE_VERB, 205, 5],
                    [PathKit.CLOSE_VERB]]);
                p.delete();
            }

            function teardown(ctx) { }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_fromCmds', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_fromSVGString', function(done) {
            function setup(ctx) {}

            function test(ctx) {
                // https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_parallelogram.svg
                let p = PathKit.FromSVGString('M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');
                p.delete();
            }

            function teardown(ctx) { }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_fromSVGString', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });
    });

    describe('export options', function() {
        it('path_toCmds', function(done) {
            function setup(ctx) {
                ctx.path = drawPath();
            }

            function test(ctx) {
                ctx.path.toCmds();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_toCmds', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_toPath2D', function(done) {
            function setup(ctx) {
                ctx.path = drawPath();
            }

            function test(ctx) {
                ctx.path.toPath2D();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_toPath2D', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_toSVGString', function(done) {
            function setup(ctx) {
                ctx.path = drawPath();
            }

            function test(ctx) {
                ctx.path.toSVGString();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_toSVGString', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });
    });

    describe('matrix options', function() {
        function drawTriangle() {
            let path = PathKit.NewPath();
            path.moveTo(0, 0);
            path.lineTo(10, 0);
            path.lineTo(10, 10);
            path.close();
            return path;
        }

        it('path_add_path_svgmatrix', function(done) {
            function setup(ctx) {
                ctx.path = drawTriangle();
            }

            function test(ctx) {
                let path = PathKit.NewPath();
                let m = document.createElementNS('http://www.w3.org/2000/svg', 'svg').createSVGMatrix();
                m.a = 1; m.b = 0;
                m.c = 0; m.d = 1;
                m.e = 0; m.f = 20.5;
                path.addPath(ctx.path, m);
                path.delete();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_add_path_svgmatrix', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_add_path_svgmatrix_reuse', function(done) {
            function setup(ctx) {
                ctx.path = drawTriangle();
                let m = document.createElementNS('http://www.w3.org/2000/svg', 'svg').createSVGMatrix();
                ctx.matrix = m;
            }

            function test(ctx) {
                let path = PathKit.NewPath();
                let m = ctx.matrix
                m.a = 1; m.b = 0;
                m.c = 0; m.d = 1;
                m.e = 0; m.f = 20.5;
                path.addPath(ctx.path, m);
                path.delete();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_add_path_svgmatrix_reuse', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it('path_add_path_svgmatrix_bare', function(done) {
            function setup(ctx) {
                ctx.path = drawTriangle();
            }

            function test(ctx) {
                let path = PathKit.NewPath();
                path.addPath(ctx.path, 1, 0, 0, 1, 0, 20.5);
                path.delete();
            }

            function teardown(ctx) {
                ctx.path.delete();
            }

            LoadPathKit.then(() => {
                benchmarkAndReport('path_add_path_svgmatrix_bare', setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });
    });

});