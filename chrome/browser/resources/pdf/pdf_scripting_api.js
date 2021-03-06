// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Create a new PDFScriptingAPI. This provides a scripting interface to
 * the PDF viewer so that it can be customized by things like print preview.
 * @param {Window} window the window of the page containing the pdf viewer.
 * @param {Object} plugin the plugin element containing the pdf viewer.
 */
function PDFScriptingAPI(window, plugin) {
  this.loaded_ = false;
  this.pendingScriptingMessages_ = [];
  this.setPlugin(plugin);

  window.addEventListener('message', function(event) {
    if (event.origin != 'chrome-extension://mhjfbmdgcfjbbpaeojofohoefgiehjai') {
      console.error('Received message that was not from the extension: ' +
                    event);
      return;
    }
    switch (event.data.type) {
      case 'viewport':
        if (this.viewportChangedCallback_)
          this.viewportChangedCallback_(event.data.pageX,
                                        event.data.pageY,
                                        event.data.pageWidth,
                                        event.data.viewportWidth,
                                        event.data.viewportHeight);
        break;
      case 'documentLoaded':
        this.loaded_ = true;
        if (this.loadCallback_)
          this.loadCallback_();
        break;
      case 'getAccessibilityJSONReply':
        if (this.accessibilityCallback_) {
          this.accessibilityCallback_(event.data.json);
          this.accessibilityCallback_ = null;
        }
        break;
    }
  }.bind(this), false);
}

PDFScriptingAPI.prototype = {
  /**
   * @private
   * Send a message to the extension. If messages try to get sent before there
   * is a plugin element set, then we queue them up and send them later (this
   * can happen in print preview).
   * @param {Object} message The message to send.
   */
  sendMessage_: function(message) {
    if (this.plugin_)
      this.plugin_.postMessage(message, '*');
    else
      this.pendingScriptingMessages_.push(message);
  },

 /**
  * Sets the plugin element containing the PDF viewer. The element will usually
  * be passed into the PDFScriptingAPI constructor but may also be set later.
  * @param {Object} plugin the plugin element containing the PDF viewer.
  */
  setPlugin: function(plugin) {
    this.plugin_ = plugin;

    // Send an initialization message to the plugin indicating the window to
    // respond to.
    if (this.plugin_) {
      this.sendMessage_({
        type: 'setParentWindow'
      });

      // Now we can flush pending messages
      while (this.pendingScriptingMessages_.length > 0)
        this.sendMessage_(this.pendingScriptingMessages_.shift());
    }
  },

  /**
   * Sets the callback which will be run when the PDF viewport changes.
   * @param {Function} callback the callback to be called.
   */
  setViewportChangedCallback: function(callback) {
    this.viewportChangedCallback_ = callback;
  },

  /**
   * Sets the callback which will be run when the PDF document has finished
   * loading. If the document is already loaded, it will be run immediately.
   * @param {Function} callback the callback to be called.
   */
  setLoadCallback: function(callback) {
    this.loadCallback_ = callback;
    if (this.loaded_ && callback)
      callback();
  },

  /**
   * Resets the PDF viewer into print preview mode.
   * @param {string} url the url of the PDF to load.
   * @param {boolean} grayscale whether or not to display the PDF in grayscale.
   * @param {Array.<number>} pageNumbers an array of the page numbers.
   * @param {boolean} modifiable whether or not the document is modifiable.
   */
  resetPrintPreviewMode: function(url, grayscale, pageNumbers, modifiable) {
    this.sendMessage_({
      type: 'resetPrintPreviewMode',
      url: url,
      grayscale: grayscale,
      pageNumbers: pageNumbers,
      modifiable: modifiable
    });
  },

  /**
   * Load a page into the document while in print preview mode.
   * @param {string} url the url of the pdf page to load.
   * @param {number} index the index of the page to load.
   */
  loadPreviewPage: function(url, index) {
    this.sendMessage_({
      type: 'loadPreviewPage',
      url: url,
      index: index
    });
  },

  /**
   * Get accessibility JSON for the document.
   * @param {Function} callback a callback to be called with the accessibility
   *     json that has been retrieved.
   * @param {number} [page] the 0-indexed page number to get accessibility data
   *     for. If this is not provided, data about the entire document is
   *     returned.
   * @return {boolean} true if the function is successful, false if there is an
   *     outstanding request for accessibility data that has not been answered.
   */
  getAccessibilityJSON: function(callback, page) {
    if (this.accessibilityCallback_)
      return false;
    this.accessibilityCallback_ = callback;
    var message = {
      type: 'getAccessibilityJSON',
    };
    if (page || page == 0)
      message.page = page;
    this.sendMessage_(message);
    return true;
  },

  /**
   * Send a key event to the extension.
   * @param {number} keyCode the key code to send to the extension.
   */
  sendKeyEvent: function(keyCode) {
    this.sendMessage_({
      type: 'sendKeyEvent',
      keyCode: keyCode
    });
  },
};

/**
 * Creates a PDF viewer with a scripting interface. This is basically 1) an
 * iframe which is navigated to the PDF viewer extension and 2) a scripting
 * interface which provides access to various features of the viewer for use
 * by print preview and accessibility.
 * @param {string} src the source URL of the PDF to load initially.
 * @return {HTMLIFrameElement} the iframe element containing the PDF viewer.
 */
function PDFCreateOutOfProcessPlugin(src) {
  var iframe = window.document.createElement('iframe');
  iframe.setAttribute(
      'src',
      'chrome-extension://mhjfbmdgcfjbbpaeojofohoefgiehjai/index.html?' + src);
  var client = new PDFScriptingAPI(window);
  iframe.onload = function() {
    client.setPlugin(iframe.contentWindow);
  };
  // Add the functions to the iframe so that they can be called directly.
  iframe.setViewportChangedCallback =
      client.setViewportChangedCallback.bind(client);
  iframe.setLoadCallback = client.setLoadCallback.bind(client);
  iframe.resetPrintPreviewMode = client.resetPrintPreviewMode.bind(client);
  iframe.loadPreviewPage = client.loadPreviewPage.bind(client);
  iframe.sendKeyEvent = client.sendKeyEvent.bind(client);
  return iframe;
}
