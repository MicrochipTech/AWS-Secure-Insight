var AWS = require('aws-sdk');
var myService = new AWS.Iot({
    accessKeyId: "AKIAIW5N2D3ZXLH6KD5Q",
    secretAccessKey: "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",
    region: "us-west-2",
    endpoint: "iot.us-west-2.amazonaws.com",
    apiVersion: '2015-05-28'
  
});
var params = {};
myService.describeEndpoint(params, function(err, data) {
  console.log(data.endpointAddress);
});
