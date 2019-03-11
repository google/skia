var dumpErrors = false;
var container;

function getViewBox(path) {
    let bounds = path.getBounds();
    return `${(bounds.fLeft-2)*.95} ${(bounds.fTop-2)*.95} ${(bounds.fRight+2)*1.05} ${(bounds.fBottom+2)*1.05}`;
}

function addSVG(testName, expectedPath, actualPath, message) {
    if (!dumpErrors) {
        return;
    }
    if (!container) {
        let styleEl = document.createElement('style');
        document.head.appendChild(styleEl);
        let sheet = styleEl.sheet;
        sheet.insertRule(`svg {
            border: 1px solid #DDD;
            max-width: 45%;
            vertical-align: top;
        }`, 0);

        container = document.createElement('div');
        document.body.appendChild(container);

    }

    let thisTest = document.createElement('div');
    thisTest.innerHTML = `
    <h2>Failed test ${testName}</h2>

    <div>${message}</div>

    <svg class='expected' viewBox='${getViewBox(expectedPath)}'>
        <path stroke=black fill=white stroke-width=0.01 d="${expectedPath.toSVGString()}"></path>
    </svg>

    <svg class='actual' viewBox='${getViewBox(actualPath)}'>
        <path stroke=black fill=white stroke-width=0.01 d="${actualPath.toSVGString()}"></path>
    </svg>
`;
    container.appendChild(thisTest);

}

const TOLERANCE = 0.0001;

function diffPaths(expected, actual) {
    // Look through commands and see if they are within tolerance.
    let eCmds = expected.toCmds(), aCmds = actual.toCmds();
    if (eCmds.length !== aCmds.length) {
        //console.log(`Expected: ${JSON.stringify(eCmds)} and Actual: ${JSON.stringify(aCmds)}`);
        return `Different amount of verbs.  Expected had ${eCmds.length}, Actual had ${aCmds.length}`;
    }
    for(let idx = 0; idx < eCmds.length; idx++){
        let eCmd = eCmds[idx], aCmd = aCmds[idx];
        if (eCmd.length !== aCmd.length) {
            // Should never happen, means WASM code is returning bad ops.
            return `Command index ${idx} differs in num arguments. Expected had ${eCmd.length}, Actual had ${aCmd.length}`;
        }
        let eVerb = eCmd[0], aVerb = aCmd[0];
        if (eVerb !== aVerb) {
            return `Command index ${idx} differs. Expected had ${eVerb}, Actual had ${aVerb}`;
        }
        for (let arg = 1; arg < eCmd.length; arg++) {
            if (Math.abs(eCmd[arg] - aCmd[arg]) > TOLERANCE) {
                return `Command index ${idx} has different argument for verb ${eVerb} at position ${arg}. Expected had ${eCmd[arg]}, Actual had ${aCmd[arg]}`
            }
        }
    }
    return null;
}

describe('PathKit\'s PathOps Behavior', function() {
    var PATHOP_MAP = {};
    var FILLTYPE_MAP = {};

    function init() {
        if (PathKit && !PATHOP_MAP['kIntersect_SkPathOp']) {
            PATHOP_MAP = {
                'kIntersect_SkPathOp':         PathKit.PathOp.INTERSECT,
                'kDifference_SkPathOp':        PathKit.PathOp.DIFFERENCE,
                'kUnion_SkPathOp':             PathKit.PathOp.UNION,
                'kXOR_SkPathOp':               PathKit.PathOp.XOR,
                'kXOR_PathOp':                 PathKit.PathOp.XOR,
                'kReverseDifference_SkPathOp': PathKit.PathOp.REVERSE_DIFFERENCE,
            };
            FILLTYPE_MAP = {
                'kWinding_FillType':        PathKit.FillType.WINDING,
                'kEvenOdd_FillType':        PathKit.FillType.EVENODD,
                'kInverseWinding_FillType': PathKit.FillType.INVERSE_WINDING,
                'kInverseEvenOdd_FillType': PathKit.FillType.INVERSE_EVENODD,
            };
        }
    }

    function getFillType(str) {
        let e = FILLTYPE_MAP[str];
        expect(e).toBeTruthy(`Could not find FillType Enum for ${str}`);
        return e;
    }

    function getPathOp(str) {
        let e = PATHOP_MAP[str];
        expect(e).toBeTruthy(`Could not find PathOp Enum for ${str}`);
        return e;
    }

    it('combines two paths with .op() and matches what we see from C++', function(done) {
        LoadPathKit.then(catchException(done, () => {
            init();
            // Test JSON created with:
            // ./out/Clang/pathops_unittest -J ./modules/pathkit/tests/PathOpsOp.json -m PathOpsOp$
            fetch('/base/tests/PathOpsOp.json').then((r) => {
                r.json().then((json) => {
                    expect(json).toBeTruthy();
                    let testNames = Object.keys(json);
                    // Assert we loaded a non-zero amount of tests, i.e. the JSON is valid.
                    expect(testNames.length > 0).toBeTruthy();
                    testNames.sort();
                    for (testName of testNames) {
                        let test = json[testName];

                        let path1 = PathKit.FromCmds(test.p1);
                        expect(path1).not.toBeNull(`path1 error when loading cmds '${test.p1}'`);
                        path1.setFillType(getFillType(test.fillType1));

                        let path2 = PathKit.FromCmds(test.p2);
                        expect(path2).not.toBeNull(`path2 error when loading cmds '${test.p2}'`);
                        path2.setFillType(getFillType(test.fillType2));

                        let combined = path1.op(path2, getPathOp(test.op));

                        if (test.expectSuccess === 'no') {
                            expect(combined).toBeNull(`Test ${testName} should have not created output, but did`);
                        } else {
                            expect(combined).not.toBeNull();
                            let expected = PathKit.FromCmds(test.out);
                            // Do a tolerant match.
                            let diff = diffPaths(expected, combined);
                            if (test.expectMatch === 'yes'){
                                // Check fill type
                                expect(combined.getFillType().value).toEqual(getFillType(test.fillTypeOut).value);
                                // diff should be null if the paths are identical (modulo rounding)
                                if (diff) {
                                    expect(`[${testName}] ${diff}`).toBe('');
                                    addSVG('[PathOps] ' + testName, expected, combined, diff);
                                }
                            } else if (test.expectMatch === 'flaky') {
                                // Don't worry about it, at least it didn't crash.
                            } else {
                                if (!diff) {
                                    expect(`[${testName}] was expected to have paths that differed`).not.toBe('');
                                }
                            }
                            expected.delete();
                        }
                        // combined === path1, so we only have to delete one.
                        path1.delete();
                        path2.delete();
                    }
                    done();
                });
            });
        }));
    });

    it('simplifies a path with .simplify() and matches what we see from C++', function(done) {
        LoadPathKit.then(catchException(done, () => {
            init();
            // Test JSON created with:
            // ./out/Clang/pathops_unittest -J ./modules/pathkit/tests/PathOpsSimplify.json -m PathOpsSimplify$
            fetch('/base/tests/PathOpsSimplify.json').then((r) => {
                r.json().then((json) => {
                    expect(json).toBeTruthy();
                    let testNames = Object.keys(json);
                    // Assert we loaded a non-zero amount of tests, i.e. the JSON is valid.
                    expect(testNames.length > 0).toBeTruthy();
                    testNames.sort();
                    for (testName of testNames) {
                        let test = json[testName];

                        let path = PathKit.FromCmds(test.path);
                        expect(path).not.toBeNull(`path1 error when loading cmds '${test.path}'`);
                        path.setFillType(getFillType(test.fillType));

                        let simplified = path.simplify();

                        if (test.expectSuccess === 'no') {
                            expect(simplified).toBeNull(`Test ${testName} should have not created output, but did`);
                        } else {
                            expect(simplified).not.toBeNull();
                            let expected = PathKit.FromCmds(test.out);
                            // Do a tolerant match.
                            let diff = diffPaths(expected, simplified);
                            if (test.expectMatch === 'yes'){
                                // Check fill type
                                expect(simplified.getFillType().value).toEqual(getFillType(test.fillTypeOut).value);
                                // diff should be null if the paths are identical (modulo rounding)
                                if (diff) {
                                    expect(`[${testName}] ${diff}`).toBe('');
                                    addSVG('[Simplify] ' + testName, expected, simplified, diff);
                                }
                            } else if (test.expectMatch === 'flaky') {
                                // Don't worry about it, at least it didn't crash.
                            } else {
                                if (!diff) {
                                    expect(`[${testName}] was expected to not match output`).not.toBe('');
                                }
                            }
                            expected.delete();
                        }
                        // simplified === path, so we only have to delete one.
                        path.delete();
                    }
                    done();
                });
            });
        }));
    });
});
