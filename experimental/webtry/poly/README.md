polyfill.js
===========

To rebuild or update res/js/polyfill.js you will need to have
[node.js](http://nodejs.org/) installed. Once you have it installed run the
following:

    cd polyfill
    npm install
    grunt

If you want to create a verion of the polyfill.js that hasn't been minified,
say for debugging purposes, then run:


    cd polyfill
    npm install
    grunt notmin
