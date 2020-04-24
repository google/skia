jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

let PathKit = null;
const LoadPathKit = new Promise(function(resolve, reject) {
    console.log('pathkit loading', new Date());
    PathKitInit({
        locateFile: (file) => '/pathkit/'+file,
    }).ready().then((_PathKit) => {
        console.log('pathkit loaded', new Date());
        PathKit = _PathKit;
        resolve();
    }).catch((e) => {
        console.error('pathkit failed to load', new Date(), e);
        reject();
    });
});