node-safari-push-notifications [![Build Status](https://travis-ci.org/MySiteApp/node-safari-push-notifications.svg?branch=master)](https://travis-ci.org/MySiteApp/node-safari-push-notifications) [![Coverage Status](https://coveralls.io/repos/github/MySiteApp/node-safari-push-notifications/badge.svg?branch=master)](https://coveralls.io/github/MySiteApp/node-safari-push-notifications?branch=master) [![Total alerts](https://img.shields.io/lgtm/alerts/g/MySiteApp/node-safari-push-notifications.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/MySiteApp/node-safari-push-notifications/alerts/)
==============================

[![NPM](https://nodei.co/npm/safari-push-notifications.png)](https://nodei.co/npm/safari-push-notifications/)

Helper methods for generating resources required by [Apple's Safari Push Notifications](http://apple.co/1rAeIvg).
This library was written while trying to implement node.js server that answers Safari, but it seems that OpenSSL's PKCS7 functions weren't available.

## Upgrading from 0.3.x to 0.4.x

Node versions lower than 8 are no longer supported.

## Upgrading from 0.2.0 to 0.3.x

`generatePacakge` doesn't return Buffer anymore, but a stream.

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
		intermediate = fs.readFileSync('intermediate.crt'),
		websiteJson = pushLib.websiteJSON(
			'My Site', // websiteName
			'web.com.mysite.news', // websitePushID
      ['http://push.mysite.com'], // allowedDomains
      'http://mysite.com/news?id=%@', // urlFormatString
      0123456789012345, // authenticationToken (zeroFilled to fit 16 chars)
      'https://' + baseUrl + '/push' // webServiceURL (Must be https!)
    );
    pushLib.generatePackage(
      websiteJson, // The object from before / your own website.json object
      path.join('assets', 'safari_assets'), // Folder containing the iconset
      cert, // Certificate
      key, // Private Key
      intermediate // Intermediate certificate
    )
		.pipe(fs.createWriteStream('pushPackage.zip'))
		.on('finish', function () {
			console.log('pushPackage.zip is ready.');
		});
```

# Changelog

## 0.4.0
- Dropping support of node <8 and fixing building with node 12+ [#20](https://github.com/MySiteApp/node-safari-push-notifications/pull/20)

## 0.3.3
- Fixing a memory leak in `sign` method [#18](https://github.com/MySiteApp/node-safari-push-notifications/pull/18)

## 0.3.2
- Manifest File Creation Fix [#17](https://github.com/MySiteApp/node-safari-push-notifications/pull/17)

## 0.3.1
- Removing custom openssl functions, in favor of system libraries.
- Potentially fixing a bad data read [#12](https://github.com/MySiteApp/node-safari-push-notifications/pull/12).
- Adding some tests.

## 0.3.0
- BREAKING: JSZip instead of AdmZip, returns zip file as stream instead of buffer. [#8](https://github.com/MySiteApp/node-safari-push-notifications/pull/8)

## 0.2.0
- Supporting intermediate certificate [#5](https://github.com/MySiteApp/node-safari-push-notifications/pull/5)

## 0.1.0
- [NaN](https://github.com/rvagg/nan) 2 [#4](https://github.com/MySiteApp/node-safari-push-notifications/pull/4)
- Node 4.x support

## 0.0.2
- Node.js v0.12 support
- Using [NaN](https://github.com/rvagg/nan) for binding abstraction

## 0.0.1
- First version.

# Credits
- [@KenanSulayman](https://github.com/KenanSulayman) for node 0.12 + NaN support (v0.0.2)
- [@dezinezync](https://github.com/dezinezync) for adding support to intermediate certificates (v0.2.0)
- [@raymondflores](https://github.com/raymondflores) for updating the manifest file format (v0.3.2)
- [@loris](https://github.com/loris) for fixing a memory leak (v0.3.3)

# License

The MIT License (MIT)

Copyright (c) 2014 MySiteApp Ltd.

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
