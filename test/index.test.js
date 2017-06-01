var pushLib = require('../src/index');

test('websiteJSON returns the right array', function() {
  var name = 'My Site',
    websitePushID = 'web.com.mysite.news',
    allowedDomains = ['https://push.mysite.com'],
    urlFormatString = 'http://mysite.com/news?id=%@',
    authenticationToken = 10291,
    webServiceURL = 'https://news.mysite.com/push';

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

test('')