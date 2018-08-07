

describe('PathKit\'s Path2D API', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject){
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/base/npm-wasm/bin/test/'+file,
            }).then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    it('should load PathKit', function(done){
        LoadPathKit.then(() => {
            expect(PathKit.thisProgram).toBeTruthy();
            expect(PathKit.notDefined).toBeUndefined();
            done();
        });
    });

    it('should really load PathKit', function(done){
        LoadPathKit.then(() => {
            expect(PathKit.thisProgram).toBeTruthy();
            expect(PathKit.notDefined).toBeUndefined();
            done();
        });
    });
});