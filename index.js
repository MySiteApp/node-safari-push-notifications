var fs = require('fs'),
    path = require('path'),
    utils = require("./utils"),
    pkcs7 = require('bindings')('pkcs7'),
    JSZip = require('jszip');

var push = module.exports = {},
    PKCS7_CONTENT_REGEX = /Content-Disposition:[^\n]+\s*?([A-Za-z0-9+=/\r\n]+)\s*?-----/;

push.websiteJSON = function(name, pushId, allowedDomains, urlFormattingString, authToken, webServiceURL) {
    return {
        "websiteName": name,
        "websitePushID": pushId,
        "allowedDomains": Array.isArray(allowedDomains) ? allowedDomains : [allowedDomains],
        "urlFormatString": urlFormattingString,
        "authenticationToken": utils.zeroFill(authToken, 16),
        "webServiceURL": webServiceURL
    };
};

push.generatePackage = function(websiteJSON, iconsDir, certData, pKeyData, intermediate, callback) {
    if (typeof callback == 'undefined') {
        throw new Error('this function needs an asynchronous callback')
    }
    if (typeof websiteJSON !== 'object' && !('websitePushID' in websiteJSON)) {
        throw new Error('websiteJSON should be generated using websiteJSON() method');
    }
    var zip = new JSZip(),
        manifest = {};

    // Replace addFile method, to calculate the sha1 on every file
    var _addFile = zip.file;
    zip.file = function(name, data, o) {
        manifest[name] = utils.sha1(data);
        _addFile.apply(zip, Array.prototype.splice.call(arguments, 0));
    };

    // website.json
    zip.file('website.json', JSON.stringify(websiteJSON));

    // icon.iconset
    var icons = zip.folder('icon.iconset');
    if (typeof iconsDir == 'string') {
        // expect directory name
        fs.readdirSync(iconsDir).forEach(function (name) {
            var filePath = path.join(iconsDir, name);
            if (fs.statSync(filePath).isFile()) {
                icons.file(name, fs.readFileSync(filePath));
            }
        });
    } else if (typeof iconsDir == 'object') {
        // expect object containing file buffers
        Object.keys(iconsDir).forEach(function(name, i) {
            icons.file(name, iconsDir[i]);
        });
    } else {
        throw new Error('iconsDir not recognized');
    }
    icons.file();

    // manifest.json
    var manifestContent = new Buffer(JSON.stringify(manifest));
    zip.file('manifest.json', manifestContent);

    // signature
    if (typeof certData == 'string') {
        certData = new Buffer(certData);
    }
    if (typeof pKeyData == 'string') {
        pKeyData = new Buffer(pKeyData);
    }
    if (typeof intermediate == 'string') {
        intermediate = new Buffer(intermediate);
    }
    var pkcs7sig = intermediate ? pkcs7.sign(certData, pKeyData, manifestContent, intermediate) : pkcs7.sign(certData, pKeyData, manifestContent),
        content = PKCS7_CONTENT_REGEX.exec(pkcs7sig.toString());

    content = new Buffer(content[1], 'base64');
    zip.file('signature', content);

    zip.generateNodeStream({ type: 'nodebuffer', streamFiles: true });
};
