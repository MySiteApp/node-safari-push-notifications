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

push.generatePackage = function(websiteJSON, iconsDir, certData, pKeyData, intermediate) {
    if (typeof websiteJSON !== 'object' && !('websitePushID' in websiteJSON)) {
        throw new Error('websiteJSON should be generated using websiteJSON() method');
    }
    var zip = new JSZip(),
        manifest = {};

    // website.json
    var websiteContent = new Buffer(JSON.stringify(websiteJSON));
    manifest['website.json'] = utils.sha1(websiteContent);
    zip.file('website.json', websiteContent);

    // icon.iconset
    var addIconFile = function(name, content) {
        manifest['icon.iconset/' + name] = utils.sha1(content);
        icons.file(name, content);
    };
    var icons = zip.folder('icon.iconset');
    if (typeof iconsDir == 'object') {
        // expect object containing file buffers
        Object.keys(iconsDir).forEach(function(iconName) {
            var icon = iconsDir[iconName];
            if (typeof icon == 'string') {
                // filename given
                addIconFile(iconName, fs.readFileSync(icon));
            } else if (icon instanceof Buffer) {
                // buffer given
                addIconFile(iconName, icon);
            }
        });
    } else if (typeof iconsDir == 'string') {
        // expect directory name
        fs.readdirSync(iconsDir).forEach(function(iconName) {
            var filePath = path.join(iconsDir, iconName);
            if (fs.statSync(filePath).isFile()) {
                addIconFile(iconName, fs.readFileSync(filePath));
            }
        });
    } else {
        throw new Error('iconsDir not recognized');
    }

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

    return zip.generateNodeStream({ type: 'nodebuffer', streamFiles: false });
};
