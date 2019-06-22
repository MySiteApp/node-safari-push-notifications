var crypto = require('crypto');

module.exports = {
  zeroFill: function(number, width) {
    width -= number.toString().length;
    if (width > 0) {
      return new Array( width + (/\./.test( number ) ? 2 : 1) ).join( '0' ) + number;
    }
    return number + ""; // always return a string
  },
  sha512: function (content) {
    var shasum = crypto.createHash('sha512');
    shasum.update(content);
    return {
      "hashType": "sha512",
      "hashValue": shasum.digest('hex')
    };
  }
};
