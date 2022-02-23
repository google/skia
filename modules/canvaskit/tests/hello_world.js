describe('The test harness', () => {
    it('runs the first test', () => {
        expect(2+3).toBe(5);
    });
    it('runs the second test', () => {
        expect(null).toBeFalsy();
    });

    describe('the CanvasKit loading promise', () => {
        beforeEach(async () => {
            await LoadCanvasKit;
        });

        it('has access to CanvasKit', () => {
           const r = CanvasKit.LTRBRect(1, 2, 3, 4);
           expect(r.constructor.name).toEqual('Float32Array');
        });
    });
})