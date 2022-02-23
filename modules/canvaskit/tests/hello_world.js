describe('The test harness', () => {
    it('runs the first test', () => {
        expect(2+3).toBe(5);
    });
    it('runs the second test', () => {
        expect(null).toBeFalsy();
    });
})