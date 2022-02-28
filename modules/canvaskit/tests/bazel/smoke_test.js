describe('The test harness', () => {
    beforeEach(async () => {
        await EverythingLoaded;
    });

    it('can do assertions', () => {
        expect(2+3).toBe(5);
    });

    it('has access to CanvasKit', () => {
       const r = CanvasKit.LTRBRect(1, 2, 3, 4);
       expect(r.constructor.name).toEqual('Float32Array');
    });

    it('can talk to the Gold server', async () => {
        const resp = await fetch('/gold_rpc/healthz');
        expect(resp.status).toEqual(200);
    });
})
