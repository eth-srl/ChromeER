// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.document;

import android.content.Intent;

import org.chromium.content_public.common.Referrer;

/**
 * Data that will be used later when a tab is opened via an intent. Often only the necessary
 * subset of the data will be set. All data is removed once the tab finishes initializing.
 */
public class PendingDocumentData {
    /** Pending native web contents object to initialize with. */
    public long nativeWebContents;

    /** The url to load in the current tab. */
    public String url;

    /** Data to send with a POST request. */
    public byte[] postData;

    /** Extra HTTP headers to send. */
    public String extraHeaders;

    /** HTTP "referer". */
    public Referrer referrer;

    /** The original intent */
    public Intent originalIntent;
}