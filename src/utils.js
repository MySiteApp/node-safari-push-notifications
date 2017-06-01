var crypto = require('crypto');

var utils = module.exports = {};

utils.zeroFill = function(number, width) {
    width -= number.toString().length;
    if (width > 0) {
        return new Array( width + (/\./.test( number ) ? 2 : 1) ).join( '0' ) + number;
    }
    return number + ""; // always return a string
}

utils.sha1 = function (content) {
    var shasum = crypto.createHash('sha1');
    shasum.update(content);
    return shasum.digest('hex');
}
