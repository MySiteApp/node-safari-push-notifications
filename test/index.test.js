var pushLib = require('../src/index')
  path = require('path'),
  concat = require('concat-stream'),
  fs = require('fs'),
  JSZip = require('jszip'),
  execFileSync = require('child_process').execFileSync;

var name = 'My Site',
  websitePushID = 'web.com.mysite.news',
  allowedDomains = ['https://push.mysite.com'],
  urlFormatString = 'http://mysite.com/news?id=%@',
  webServiceURL = 'https://news.mysite.com/push';


describe('test websiteJSON', function() {
  test('returns the right array', function() {
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

  test('accepts single domain as allowedDomains', function() {
    var authenticationToken = 10291
    var websiteJSON = pushLib.websiteJSON(name, websitePushID, allowedDomains[0],
      urlFormatString, authenticationToken, webServiceURL);
    expect(websiteJSON.websiteName).toBe(name);
    expect(websiteJSON.websitePushID).toBe(websitePushID);
    expect(websiteJSON.allowedDomains).toEqual(allowedDomains);
    expect(websiteJSON.urlFormatString).toBe(urlFormatString);
  });
});

describe('testing signing', function() {
  var websiteJSON = null,
    authenticationToken = 0,
    basePath = path.join('test', 'files'),
    iconsDir = path.join(basePath, 'icons'),
    certsPath = path.join(basePath, 'certs'),
    cert = {
      cert: fs.readFileSync(path.join(certsPath, 'cert.cert.pem')),
      intermediate: fs.readFileSync(path.join(certsPath, 'intermediate.cert.pem')),
      root: fs.readFileSync(path.join(certsPath, 'ca.cert.pem')),
      key: fs.readFileSync(path.join(certsPath, 'cert.key.pem'))
    },
    certStrings = {
      cert: fs.readFileSync(path.join(certsPath, 'cert.cert.pem')).toString('utf8'),
      intermediate: fs.readFileSync(path.join(certsPath, 'intermediate.cert.pem')).toString('utf8'),
      root: fs.readFileSync(path.join(certsPath, 'ca.cert.pem')).toString('utf8'),
      key: fs.readFileSync(path.join(certsPath, 'cert.key.pem')).toString('utf8')
    },
    selfSigned = {
      cert: fs.readFileSync(path.join(certsPath, 'ca.cert.pem')),
      key: fs.readFileSync(path.join(certsPath, 'ca.key.pem'))
    },
    invalidCert = {
      cert: fs.readFileSync(path.join(basePath, 'invalid.cert.pem')),
      root: fs.readFileSync(path.join(certsPath, 'ca.cert.pem')),
      key: fs.readFileSync(path.join(certsPath, 'cert.key.pem'))
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
            if (withIntermediate) {
              expect(pushLib.verify(manifest, sig, certObj.cert, certObj.intermediate, certObj.root)).toBeTruthy();  
            } else {
              expect(pushLib.verify(manifest, sig, certObj.cert)).toBeTruthy();
            }
            resolve();
          })
          .catch(function (err) {
            reject(err);
          });
      }

      pushLib.generatePackage(json, icons, certObj.cert, certObj.key, withIntermediate ? certObj.intermediate : null)
        .pipe(concat(function(final) {
          var zip = new JSZip();
          zip.loadAsync(final).then(zipLoaded);
        }));
    });
  }

  beforeAll(function() {
    execFileSync(path.join(certsPath, 'generate.sh'));
  });

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

  test('iconsDir can be an object', function() {
    var icons = {};
    var byContent = false;
    fs.readdirSync(iconsDir).forEach(function (icon) {
      if (icon === 'empty') {
        return;
      }
      icons[icon] = path.join(iconsDir, icon);
      if (!byContent) {
        icons[icon] = fs.readFileSync(icons[icon]);
        byContent = true;
      }
    });
    return signAndVerify(websiteJSON, icons, cert, true);
  });

  test('valid signature without intermediate', function () {
    return signAndVerify(websiteJSON, iconsDir, selfSigned, false);
  });

  test('valid signature with intermediate', function () {
    return signAndVerify(websiteJSON, iconsDir, cert, true);
  });

  test('valid string certs signature with intermediate', function () {
    return signAndVerify(websiteJSON, iconsDir, certStrings, true);
  });

  test('invalid cert fails decode', function() {
    expect.assertions(1);
    return signAndVerify(websiteJSON, iconsDir, invalidCert, true)
      .catch(function (e) {
        expect(e.message).toMatch(/bad base64 decode/)
      });
  });

  test('invalid cert fails verification', function() {
    expect.assertions(1);
    return signAndVerify(websiteJSON, iconsDir, cert, false) // Cert will fail without root
      .catch(function (e) {
        expect(e.message).toMatch(/Invalid signature/)
      });
  });
});