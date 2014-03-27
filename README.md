node-safari-push-notifications
==============================

Helper methods for generating resources required by [Apple's Safari Push Notifications](http://l.brow.si/1rAeIvg).

# Note
When building this module, the `PKCS7_Sign` and `SMIME_write_PKCS7` embedded in OS X 10.9 continued to fail.
So I copied [OpenSSL 0.9.8](https://www.openssl.org/source/openssl-0.9.8.tar.gz)'s version and it works just as the companion example from Apple.

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
