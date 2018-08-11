var fs = require('fs'),
  path = require('path'),
  utils = require("./utils"),
  pkcs7 = require('bindings')('pkcs7'),
  JSZip = require('jszip');

var PKCS7_CONTENT_REGEX = /Content-Disposition:[^\n]+\s*?([A-Za-z0-9+=/\r\n]+)\s*?-----/;

var websiteJSON = function(name, pushId, allowedDomains, urlFormattingString, authToken, webServiceURL) {
  return {
    "websiteName": name,
    "websitePushID": pushId,
    "allowedDomains": Array.isArray(allowedDomains) ? allowedDomains : [allowedDomains],
    "urlFormatString": urlFormattingString,
    "authenticationToken": utils.zeroFill(authToken, 16),
    "webServiceURL": webServiceURL
  };
};

var generatePackage = function(websiteJSON, iconsDir, certData, pKeyData, intermediate) {
  if (typeof websiteJSON !== 'object' || !('websitePushID' in websiteJSON)) {
    throw new Error('websiteJSON should be generated using the websiteJSON() method');
  }
  var zip = new JSZip(),
    manifest = {};

  // website.json
  var websiteContent = new Buffer(JSON.stringify(websiteJSON));
  manifest['website.json'] = utils.sha512(websiteContent);
  zip.file('website.json', websiteContent);

  // icon.iconset
  var icons = zip.folder('icon.iconset'),
    addIconFile = function(name, content) {
      content = typeof content === 'string' ? fs.readFileSync(content) : content;
      manifest['icon.iconset/' + name] = utils.sha512(content);
      icons.file(name, content);
    };
  if (typeof iconsDir === 'object') {
    // expect object containing file buffers
    Object.keys(iconsDir).forEach(function(iconName) {
      addIconFile(iconName, iconsDir[iconName]);
    });
  } else if (typeof iconsDir === 'string') {
    // expect directory name
    fs.readdirSync(iconsDir).forEach(function(iconName) {
      var filePath = path.join(iconsDir, iconName);
      if (fs.statSync(filePath).isFile()) {
        addIconFile(iconName, filePath);
      }
    });
  } else {
    throw new Error('iconsDir not recognized');
  }

  // manifest.json
  var manifestContent = new Buffer(JSON.stringify(manifest));
  zip.file('manifest.json', manifestContent);

  // signature
  if (typeof certData === 'string') {
    certData = new Buffer(certData);
  }
  if (typeof pKeyData === 'string') {
    pKeyData = new Buffer(pKeyData);
  }
  if (typeof intermediate === 'string') {
    intermediate = new Buffer(intermediate);
  }
  var pkcs7sig = pkcs7.sign(certData, pKeyData, manifestContent, intermediate),
    content = PKCS7_CONTENT_REGEX.exec(pkcs7sig.toString());

  content = new Buffer(content[1], 'base64');
  zip.file('signature', content);

  return zip.generateNodeStream({ type: 'nodebuffer', streamFiles: false });
};

var verify = function(file, signature, cert, intermediate, rootCA) {
  if (typeof cert === 'string') {
    cert = new Buffer(cert);
  }
  if (typeof intermediate === 'string') {
    intermediate = new Buffer(intermediate);
  }
  if (typeof rootCA === 'string') {
    rootCA = new Buffer(rootCA);
  }
  var result = pkcs7.verify(file, signature, cert, intermediate, rootCA);
  if (result !== 1) {  // 1 == successful verification
    throw new Error('Invalid signature');
  }
  return true;
};

module.exports = {
  websiteJSON: websiteJSON,
  generatePackage: generatePackage,
  verify: verify
};