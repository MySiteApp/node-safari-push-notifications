var pushLib = require('../src/index')
  path = require('path'),
  concat = require('concat-stream'),
  fs = require('fs'),
  JSZip = require('jszip');

var name = 'My Site',
  websitePushID = 'web.com.mysite.news',
  allowedDomains = ['https://push.mysite.com'],
  urlFormatString = 'http://mysite.com/news?id=%@',
  webServiceURL = 'https://news.mysite.com/push';


test('websiteJSON returns the right array', function() {
  var authenticationToken = 10291
  var websiteJSON = pushLib.websiteJSON(name, websitePushID, allowedDomains,
    urlFormatString, authenticationToken, webServiceURL);
  expect(websiteJSON.websiteName).toBe(name);
  expect(websiteJSON.websitePushID).toBe(websitePushID);
  expect(websiteJSON.allowedDomains).toBe(allowedDomains);
  expect(websiteJSON.urlFormatString).toBe(urlFormatString);

  var tokenStr = new String(authenticationToken),
    fullLengthToken = '0'.repeat(16 - tokenStr.length) + tokenStr;
  expect(websiteJSON.authenticationToken).toBe(fullLengthToken)
  expect(websiteJSON.webServiceURL).toBe(webServiceURL);
});

describe('testing signing', function() {
  var websiteJSON = null,
    authenticationToken = 0,
    basePath = path.join('test', 'files')
    iconsDir = path.join(basePath, 'icons'),
    cert = {
      cert: fs.readFileSync(path.join(basePath, 'cert.cert.pem')),
      intermediate: fs.readFileSync(path.join(basePath, 'intermediate.cert.pem')),
      root: fs.readFileSync(path.join(basePath, 'ca.cert.pem')),
      key: fs.readFileSync(path.join(basePath, 'cert.key.pem'))
    };

  function signAndVerify(json, icons, certObj, withIntermediate) {
    var sig;

    return new Promise(function (resolve, reject) {
      function zipLoaded(zip) {
        zip.file('signature').async('nodebuffer')
          .then(function (s) {
            sig = s;
            return zip.file('manifest.json').async('nodebuffer')
          })
          .then(function (manifest) {
            expect(pushLib.verify(manifest, sig, certObj.cert, certObj.intermediate, certObj.root)).toBeTruthy();
            resolve();
          })
          .catch(function (err) {
            reject(err);
          });
      }

      pushLib.generatePackage(json, icons, certObj.cert, certObj.key, null)
        .pipe(concat(function(final) {
          var zip = new JSZip();
          zip.loadAsync(final).then(zipLoaded);
        }));
    });
  }

  beforeEach(function() {
    authenticationToken = parseInt(Math.random() * 1000000000);
    websiteJSON = pushLib.websiteJSON(name, websitePushID, allowedDomains,
      urlFormatString, authenticationToken, webServiceURL);
  });

  afterEach(function() {
    websiteJSON = null;
    authenticationToken = 0;
  });

  test('disallows invalid params', function() {
    var invalidFunc = function(json, icons) {
      pushLib.generatePackage(json, icons || iconsDir, null, null, null);
    };
    expect(invalidFunc.bind(null, {})).toThrow(/websiteJSON should be generated/);
    expect(invalidFunc.bind(null, websiteJSON, 123)).toThrow(/iconsDir not recognized/);
  });

  test('valid signature without intermediate', function () {
    return signAndVerify(websiteJSON, iconsDir, cert, false);
  });

  test('valid signature with intermediate', function () {
    return signAndVerify(websiteJSON, iconsDir, cert, true);
  });
});