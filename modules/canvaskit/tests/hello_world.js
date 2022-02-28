describe('The test harness', () => {
    it('runs the first test', () => {
        expect(2+3).toBe(5);
    });
    it('runs the second test', () => {
        expect(null).toBeFalsy();
    });

    describe('the EverythingLoaded promise', () => {
        beforeEach(async () => {
            await EverythingLoaded;
        });

        it('has access to CanvasKit', () => {
           const r = CanvasKit.LTRBRect(1, 2, 3, 4);
           expect(r.constructor.name).toEqual('Float32Array');
        });

        it('can talk to Gold once', async () => {
            const payload = {
                name: 'test_001',
                b64_data: btoa('This could have been a PNG_' + (new Date().toLocaleString()))
            }

            const resp = await fetch('/gold_rpc/report', {
                body: JSON.stringify(payload),
                method: 'POST',
            });
            expect(resp.status).toEqual(201); // StatusCreated
        });

        it('can talk to Gold twice', async () => {
            const payload = {
                name: 'test_002',
                b64_data: btoa('This is some other data ' + (new Date().toLocaleString()))
            }

            const resp = await fetch('/gold_rpc/report', {
                body: JSON.stringify(payload),
                method: 'POST',
            });
            expect(resp.status).toEqual(201); // StatusCreated
        });
    });
})
