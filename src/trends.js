var xhrRequest = function (url, type, cb) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    cb(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
var dictionary = {
  KEY_TRENDS: ''
};
var getTopTrend = function () {
  var url = 'http://hashtagify.me/data/popular/30/en';
  xhrRequest(url, 'GET', function (responseText) {
    var trends = JSON.parse(responseText);  
    dictionary.KEY_TRENDS = '#' + trends.breakout_data[0].name;
    Pebble.sendAppMessage(dictionary, function (e) {
      console.log('Trend sent successfully');
    }, function (e) {
      console.log('Error sending trend : ', e);
    });
  });
};

Pebble.addEventListener('ready', function (e) {
  console.log('PebbleKit JS ready!');
  getTopTrend();
});

Pebble.addEventListener('appmessage', function (e) {
  console.log("AppMessage received!");
  getTopTrend();
});



