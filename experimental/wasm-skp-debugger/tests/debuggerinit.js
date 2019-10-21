jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

let Debugger = null;
const LoadDebugger = new Promise(function(resolve, reject) {
    if (Debugger) {
        resolve();
    } else {
        DebuggerInit({
            locateFile: (file) => '/debugger/bin/'+file,
        }).ready().then((_Debugger) => {
            Debugger = _Debugger;
            resolve();
        });
    }
});