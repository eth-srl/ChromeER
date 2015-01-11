// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.launcherPage.onTransitionChanged.addListener(function(progress) {
  if (progress == 0)
    chrome.test.sendMessage('onPageProgressAt0');
  else if (progress == 1)
    chrome.test.sendMessage('onPageProgressAt1');
})

chrome.test.sendMessage('Launched');
