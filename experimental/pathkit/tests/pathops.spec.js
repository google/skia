
var dumpErrors = true;
var container;

function addSVG(testName, expectedStr, actualStr, message) {
    if (!dumpErrors) {
        return;
    }
    if (!container) {
        let styleEl = document.createElement('style');
        document.head.appendChild(styleEl);
        let sheet = styleEl.sheet;
        sheet.insertRule('svg { border: 1px solid #DDD}', 0)
        container = document.createElement('div');
        document.body.appendChild(container);

    }

    let thisTest = document.createElement('div');
    thisTest.innerHTML = `
    <h2>Failed test ${testName}</h2>

    <div>${message}</div>

    <svg class='expected'>
        <path stroke=black d="${expectedStr}"></path>
    </svg>

    <svg class='actual'>
        <path stroke=black d="${actualStr}"></path>
    </svg>
`;

    container.appendChild(thisTest);

}

const TOLERANCE = 0.001;

function diffPaths(expected, actual) {
    let eCmds = expected.toCmds();
    let aCmds = actual.toCmds();
    if (eCmds.length !== aCmds.length) {
        return `Different amount of verbs.  Expected had ${eCmds.length}, Actual had ${aCmds.length}`;
    }
    for(let idx = 0; idx < eCmds.length; idx++){
        let eCmd = eCmds[idx];
        let aCmd = aCmds[idx];
        if (eCmd.length !== aCmd.length) {
            // Should never happen, means Wasm code is returning bad ops.
            return `Command index ${idx} differs in num arguments. Expected had ${eCmd.length}, Actual had ${aCmd.length}`;
        }
        let eVerb = eCmd[0]; let aVerb = aCmd[0];
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
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    var PATHOP_MAP = {};
    const LoadPathKit = new Promise(function(resolve, reject){
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/base/npm-wasm/bin/test/'+file,
            }).then((_PathKit) => {
                PathKit = _PathKit;
                PATHOP_MAP = {
                    'kIntersect_SkPathOp':PathKit.PathOp.INTERSECT,
                    'kDifference_SkPathOp':PathKit.PathOp.DIFFERENCE,
                    'kUnion_SkPathOp': PathKit.PathOp.UNION,
                    'kXOR_SkPathOp': PathKit.PathOp.XOR,
                    'kXOR_PathOp': PathKit.PathOp.XOR,
                    'kReverseDifference_SkPathOp': PathKit.PathOp.REVERSE_DIFFERENCE,
                };
                resolve();
            });
        }
    });

    it('matches the behavior we see from PathKit in C++ (-m PathOpsOp$)', function(done){
        LoadPathKit.then(() => {
            fetch('/base/tests/PathOpsOp.json').then((r) => {
                r.json().then((json)=>{
                    expect(json).toBeTruthy();
                    let testNames = Object.keys(json);
                    expect(testNames.length).toBe(358);
                    testNames.sort();
                    let success = 0;
                    for (testName of testNames) {
                        let test = json[testName];
                        let path1 = PathKit.FromSVGString(test.p1);
                        expect(path1).not.toBeNull(`path1 error when loading string'${test.p1}'`);
                        let path2 = PathKit.FromSVGString(test.p2);
                        expect(path2).not.toBeNull(`path2 error when loading string'${test.p2}'`);
                        let opEnum = PATHOP_MAP[test.op];
                        expect(opEnum).toBeTruthy(`Could not find Enum for ${test.op}`)
                        let combined = path1.op(path2, opEnum);

                        if (!test.succeeded) {
                            expect(combined).toBeNull(`Test ${testName} should have not created output, but did`);
                        } else {
                            expect(combined).not.toBeNull();
                            let expected = PathKit.FromSVGString(test.out);
                            // Do a tolerant match.
                            let diff = diffPaths(expected, combined);
                            if (diff) {
                                expect(`[${testName}] ${diff}`).toBe('');
                                addSVG(testName, combined.toSVGString(), test.out, diff);
                            } else {
                                success++;
                            }
                            expected.delete();
                            combined.delete();
                        }

                        path1.delete();
                        path2.delete();
                    }
                    expect(success).toBe(testNames.length, 'Num successes compared to num tests');
                    done();
                });
            });
        });
    });

});