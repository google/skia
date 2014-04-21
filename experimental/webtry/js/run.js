/**
 * Common JS that talks XHR back to the server and runs the code and receives
 * the results.
 */

/**
 * A polyfill for HTML Templates.
 *
 * This just adds in the content attribute, it doesn't stop scripts
 * from running nor does it stop other side-effects.
 */
(function polyfillTemplates() {
  if('content' in document.createElement('template')) {
    return false;
  }

  var templates = document.getElementsByTagName('template');
  for (var i=0; i<templates.length; i++) {
    var content = document.createDocumentFragment();
    while (templates[i].firstChild) {
     content.appendChild(templates[i].firstChild);
    }
    templates[i].content = content;
  }
})();

/**
 * All the functionality is wrapped up in this anonymous closure, but we need
 * to be told if we are on the workspace page or a normal try page, so the
 * workspaceName is passed into the closure, it must be set in the global
 * namespace. If workspaceName is the empty string then we know we aren't
 * running on a workspace page.
 *
 * If we are on a workspace page we also look for a 'history'
 * variable in the global namespace which contains the list of tries
 * that are included in this workspace. That variable is used to
 * populate the history list.
 */
(function(workspaceName) {
    var run = document.getElementById('run');
    var permalink = document.getElementById('permalink');
    var embed = document.getElementById('embed');
    var embedButton = document.getElementById('embedButton');
    var code = document.getElementById('code');
    var output = document.getElementById('output');
    var img = document.getElementById('img');
    var tryHistory = document.getElementById('tryHistory');
    var parser = new DOMParser();
    var tryTemplate = document.getElementById('tryTemplate');


    function addToHistory(hash, imgUrl) {
      var clone = tryTemplate.content.cloneNode(true);
      clone.querySelector('a').href = '/c/' + hash;
      clone.querySelector('img').src = imgUrl;
      tryHistory.insertBefore(clone, tryHistory.firstChild);
    }


    function beginWait() {
      document.body.classList.add('waiting');
      run.disabled = true;
    }


    function endWait() {
      document.body.classList.remove('waiting');
      run.disabled = false;
    }


    /**
     * Callback for when the XHR returns after attempting to run the code.
     * @param e The callback event.
     */
    function codeComplete(e) {
      // The response is JSON of the form:
      // {
      //   "message": "you had an error...",
      //   "img": "<base64 encoded image but only on success>"
      // }
      //
      // The img is optional and only appears if there is a valid
      // image to display.
      endWait();
      console.log(e.target.response);
      body = JSON.parse(e.target.response);
      output.innerText = body.message;
      if (body.hasOwnProperty('img')) {
        img.src = 'data:image/png;base64,' + body.img;
      } else {
        img.src = '';
      }
      // Add the image to the history if we are on a workspace page.
      if (tryHistory) {
        addToHistory(body.hash, 'data:image/png;base64,' + body.img);
      } else {
        window.history.pushState(null, null, "./" + body.hash);
      }
      if (permalink) {
        permalink.href = "/c/" + body.hash;
      }
      if (embed) {
        var url = document.URL;
        url = url.replace('/c/', '/iframe/');
        embed.value = '<iframe src="' + url + '" width="740" height="550" style="border: solid #00a 5px; border-radius: 5px;"/>'
      }
      if (embedButton && embedButton.hasAttribute('disabled')) {
        embedButton.removeAttribute('disabled');
      }
    }


    /**
     * Callback where there's an XHR error.
     * @param e The callback event.
     */
    function codeError(e) {
      endWait();
      alert('Something bad happened: ' + e);
    }


    function onSubmitCode() {
      beginWait();
      var req = new XMLHttpRequest();
      req.addEventListener('load', codeComplete);
      req.addEventListener('error', codeError);
      req.overrideMimeType('application/json');
      req.open('POST', '/', true);
      req.setRequestHeader('content-type', 'application/json');
      req.send(JSON.stringify({"code": code.value, "name": workspaceName}));
    }
    run.addEventListener('click', onSubmitCode);


    function onEmbedClick() {
      embed.style.display='inline';
    }

    if (embedButton) {
      embedButton.addEventListener('click', onEmbedClick);
    }


    // Add the images to the history if we are on a workspace page.
    if (tryHistory && history) {
      for (var i=0; i<history.length; i++) {
        addToHistory(history[i].hash, '/i/'+history[i].hash+'.png');
      }
    }

})(workspaceName);
