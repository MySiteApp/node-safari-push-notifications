node-safari-push-notifications
==============================

Helper methods for generating resources required by [Apple's Safari Push Notifications](http://l.brow.si/1rAeIvg).
This library was written while trying to implement node.js server that answers Safari, but it seems that OpenSSL's PKCS7 functions weren't available.

## Installation

Stable version:

	$ npm install safari-push-notifications

From git:

	$ npm install git://github.com/MySiteApp/node-safari-push-notifications.git

# Example
Certificate and Private Key should be in PEM format (pKey without password for now...)

```javascript
    var fs = require('fs'),
        pushLib = require('safari-push-notifications');

	var cert = fs.readFileSync('cert.pem'),
        key = fs.readFileSync('key.pem'),
        websiteJson = pushLib.websiteJSON(
            "My Site", // websiteName
            "web.com.mysite.news", // websitePushID
            ["http://push.mysite.com"], // allowedDomains
            "http://mysite.com/news?id=%@", // urlFormatString
            0123456789012345, // authenticationToken (zeroFilled to fit 16 chars)
            "https://" + baseUrl + "/push" // webServiceURL (Must be https!)
        );
    var zipBuffer = pushLib.generatePackage(
            websiteJson, // The object from before / your own website.json object
            path.join("assets", "safari_assets"), // Folder containing the iconset
            cert, // Certificate
            key // Private Key
        );

    fs.writeFileSync("pushPackage.zip", zipBuffer);
```

# Note
When building this module, the `PKCS7_Sign` and `SMIME_write_PKCS7` embedded in OS X 10.9 continued to fail.
So I copied [OpenSSL 0.9.8](https://www.openssl.org/source/openssl-0.9.8.tar.gz)'s version and it works just as the companion example from Apple.

# Changelog

## 0.0.1
- First version.

# License

The MIT License (MIT)

Copyright (c) 2014 Brow.si

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
