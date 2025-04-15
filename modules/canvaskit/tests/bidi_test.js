describe('Bidi Behavior', function () {
    let container;

    const assetLoadingPromises = [];

    let robotoFontBuffer = null;
    assetLoadingPromises.push(fetch('/assets/Roboto-Regular.otf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            robotoFontBuffer = buffer;
        }));

    beforeEach(async () => {
        await EverythingLoaded;
        container = document.createElement('div');
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    it('should provide correct level data from getBidiRegions', () => {
        if (!CanvasKit.Bidi) {
            console.warn('Skipping test because not compiled with bidi');
            return;
        }
        const sampleText = 'left1 يَهْدِيْكُمُ left2 اللَّه left3 ُ وَيُصْلِحُ left4 بَالَكُم left5 يَهْدِيْكُم';
        const regions = CanvasKit.Bidi.getBidiRegions(sampleText, CanvasKit.TextDirection.LTR);

        // Making sure that the returned regions are UTF16-based:
        // the last region should end at the end of the text
        // (and we know that the text is UTF16 in javascript)
        expect(regions[regions.length-1].end).toEqual(sampleText.length);

        expect(regions.length).toEqual(10);
        for (var i = 0; i < 5; i++) {
            expect(regions[i * 2].level).toEqual(0);
            expect(regions[i * 2 + 1].level).toEqual(1);
        }
    });

    function reorder(input, expected) {
        const logicals = CanvasKit.Bidi.reorderVisual(input);
        var result = '[';
        for (var i = 0; i < logicals.length; ++i) {
            const logical = logicals[i].index;
            result += (i === 0 ? '' : ', ') + logical;
        }
        result += ']';
        expect(result).toEqual(expected);
    }

    it('should provide correct order of bidi regions from reorderVisuals', () => {
        if (!CanvasKit.Bidi) {
            console.warn('Skipping test because not compiled with bidi');
            return;
        }
        reorder([], '[]');
        reorder([0], '[0]');
        reorder([1], '[0]');
        reorder([0, 1, 0, 1], '[0, 1, 2, 3]');
    });

    function checkFlags(flags, start, end, expected) {
        for (var i = start; i < end; ++i) {
            const flag = flags[i];
            var result = '';
            result += (flag & CanvasKit.CodeUnitFlags.Ideographic.value) !== 0 ? 'I' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Whitespace.value) !== 0 ? 'W' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Space.value) !== 0 ? 'S' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Control.value) !== 0 ? 'C' : '';
            expect(result).toEqual(expected);
        }
    }

    it('should provide all available unicode info for code points from getCodePointsInfo', () => {
        if (!CanvasKit.Bidi) {
            console.warn('Skipping test because not compiled with bidi');
            return;
        }
        const flagsText = '   |\u{a0}\u{a0}\u{a0}|\u{0a}\u{0a}\u{0a}|満毎行';
        const flags = CanvasKit.CodeUnits.compute(flagsText);
        checkFlags(flags.flags, 0, 3, 'WS');      // Whitespaces
        checkFlags(flags.flags, 4, 7, 'S');       // Spaces (including some that are not whitespaces)
        checkFlags(flags.flags, 8, 11, 'WSC');    // Controls
        checkFlags(flags.flags, 12, 15, 'I');     // Ideographic
    });
});
