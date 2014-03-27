var utils = require("./utils"),
    pkcs7 = require("bindings")("pkcs7");

var push = module.exports = {},
    PKCS7_CONTENT_REGEX = /Content-Disposition:[^\n]+\s*?([A-Za-z0-9+=/\r\n]+)\s*?-----/;

push.websiteJSON = function (name, pushId, allowedDomains, urlFormattingString, authToken, webServiceURL) {
    return {
        "websiteName": name,
        "websitePushID": pushId,
        "allowedDomains": Array.isArray(allowedDomains) ? allowedDomains : [allowedDomains],
        "urlFormatString": urlFormatString,
        "authenticationToken": zeroFill(authToken, 16),
        "webServiceURL": webServiceURL
    };
};

push.generatePackage = function(websiteJSON, iconsDir, certData, pKeyData) {
    if (typeof websiteJSON !== 'object' && !('websitePushID' in websiteJSON)) {
        throw new Error("websiteJSON should be generated using websiteJSON() method");
    }
    var pkg = new AdmZip(),
        manifest = {};

    // Replace addFile method, to calculate the sha1 on every file
    var _addFile = pkg.addFile;
    pkg.addFile = function (entryName, content/** Other params omitted */) {
        manifest[entryName] = utils.sha1(content);
        _addFile.apply(pkg, Array.prototype.splice.call(arguments, 0));
    };

    // website.json
    pkg.addFile("website.json", new Buffer(JSON.stringify(websiteJSON)));

    // icon.iconset
    pkg.addLocalFolder(iconsDir, "icon.iconset");

    // manifest.json
    var manifestContent = new Buffer(JSON.stringify(manifest));
    pkg.addFile("manifest.json", manifestContent);

    // signature
    if (typeof certData == 'string') {
        certData = new Buffer(certData);
    }
    if (typeof pKeyData == 'string') {
        pKeyData = new Buffer(pKeyData);
    }
    var pkcs7sig = pkcs7.sign(certData, pKeyData, manifestContent),
        content = PKCS7_CONTENT_REGEX.exec(pkcs7sig.toString());
    content = new Buffer(content, 'base64').toString('ascii');
    pkg.addFile("signature", content);
    return pkg.toBuffer();
};
