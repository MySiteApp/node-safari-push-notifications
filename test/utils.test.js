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

describe('test sha1', function() {
  test('it works', function() {
    expect(pushUtils.sha1('itworks')).toBe('fe077c8079ace921b8f9bd00d7e50e049acfda94');
  });
});