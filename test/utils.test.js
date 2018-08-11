var pushUtils = require('../src/utils');

describe('test zeroFill', function() {
  test('basic', function() {
    expect(pushUtils.zeroFill(1, 3)).toBe("001");
  });

  test('no zeros to fill', function() {
    var number = 123
    var result = pushUtils.zeroFill(number, 3);

    expect(typeof result).toBe('string');
    expect(result).toBe(String(number));
  });

  test('decimal point isn\'t considered', function() {
    var number = 456.7
    var result = pushUtils.zeroFill(number, 6);

    expect(typeof result).toBe('string');
    expect(result).toBe('00456.7');
  });
});

describe('test sha512', function() {
  test('it works', function() {
    expect(pushUtils.sha512('itworks')).toEqual({"hashType": "sha512", "hashValue": "ff3470c61d4e3f66e69bf472fd48ff67b42be150625aa0d825f0fa86dd7cb6c01e54f0c810b9208371ae3295cf892ae051e93455a041dcdafcf2cffb5d2a245a"});
  });
});