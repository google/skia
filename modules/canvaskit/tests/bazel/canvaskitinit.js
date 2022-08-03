// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 60000;

let CanvasKit = null;
const _LoadCanvasKit = new Promise((resolve, reject) => {
    console.log('canvaskit loading', new Date());
    CanvasKitInit({
        locateFile: (file) => '/static/skia/modules/canvaskit/canvaskit/'+file,
    }).then((loaded) => {
        console.log('canvaskit loaded', new Date());
        CanvasKit = loaded;
        resolve();
    }).catch((e) => {
        console.error('canvaskit failed to load', new Date(), e);
        reject();
    });
});

const _TestReportServer = new Promise((resolve, reject) => {
    fetch('/gold_rpc/healthz').then((resp) => {
        if (resp.ok) {
            resolve();
            return;
        }
        console.log('/healthz returned non 200 code')
        reject();
    }).catch((e) => {
        console.log('Server for reporting results was not up', e)
        reject();
    });
});

const EverythingLoaded = Promise.all([_LoadCanvasKit, _TestReportServer]);
