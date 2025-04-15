(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

    function Int32ArrayToBidiRegions(int32Array) {
      if (!int32Array || !int32Array.length) {
        return [];
      }
      let ret = [];
      for (let i = 0; i < int32Array.length; i+=3) {
        let start = int32Array[i];
        let end = int32Array[i+1];
        let level = int32Array[i+2];
        ret.push({'start': start, 'end': end, 'level': level});
      }
      return ret;
    }

    function Int32ArrayToBidiIndexes(int32Array) {
      if (!int32Array || !int32Array.length) {
        return [];
      }
      let ret = [];
      for (let i = 0; i < int32Array.length; i+=1) {
        let index = int32Array[i];
        ret.push({'index': index});
      }
      return ret;
    }

    function Int16ArrayToCodeUnitsFlags(int16Array) {
      if (!int16Array || !int16Array.length) {
        return [];
      }
      let ret = [];
      for (let i = 0; i < int16Array.length; i+=1) {
        let index = int16Array[i];
        ret.push({'flags': index});
      }
      return ret;
    }

    CanvasKit.Bidi.getBidiRegions = function(text, textDirection) {
      let dir = textDirection === CanvasKit.TextDirection.LTR ? 1 : 0;
      /**
      * @type {Int32Array}
      */
      let int32Array = CanvasKit.Bidi._getBidiRegions(text, dir);
      return Int32ArrayToBidiRegions(int32Array);
    }

    CanvasKit.Bidi.reorderVisual = function(visualRuns) {
      /**
      * @type {Uint8Array}
      */
      let vPtr = copy1dArray(visualRuns, 'HEAPU8');
      /**
       * @type {Int32Array}
       */
      let int32Array = CanvasKit.Bidi._reorderVisual(vPtr, visualRuns && visualRuns.length || 0);
      freeArraysThatAreNotMallocedByUsers(vPtr, visualRuns);
      return Int32ArrayToBidiIndexes(int32Array);
    }

    CanvasKit.CodeUnits.compute = function(text) {
      /**
       * @type {Uint16Array}
       */
      let uint16Array = CanvasKit.CodeUnits._compute(text);
      return Int16ArrayToCodeUnitsFlags(uint16Array);
    }

    if (!CanvasKit['TextDirection']) {
      CanvasKit['TextDirection'] = {
        'LTR': 1,
        'RTL': 0,
      }
    }
});
}(Module)); // When this file is loaded in, the high level object is "Module";
