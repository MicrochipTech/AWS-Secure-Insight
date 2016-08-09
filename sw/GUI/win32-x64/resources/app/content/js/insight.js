/*
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Constant value for the timeout
const UPDATE_RATE = 400;      //400ms
const awsPolicyName = 'PubSubToAnyTopic';
var operatingSys;

//setup the window interval timer
window.onload = function()
{
	// First get the OS
	operatingSys = os.platform();

	// check if the config file is present
	checkAndLoad();
	setTimeout(function()
	{
		//here we update the HID stuff
		jsonDataDesired = JSON.stringify({"state":{"reported":{"insight_on_things":{"insight_on_things_desktop_version" : app_version}}}});
		updateThingStatus();
		updateAMIStatus();
	}, 1000);
	scanConnections();
}

//This is the AWS IOT main code
//Include the required node modules
var AWS = require('aws-sdk');
var $ = require('jquery');
var fs = require('fs');
var os = require('os');

// App version sent to reported state as insight_desktop_version
var app_version = "1.1.0";
// AWS API version
var awsApiVersion = "2015-05-28";

//Access keys are not global variables, passed as function
var credThingName;
var credRegion;
var credThingEndpoint;
var credAwsEndpoint;
var credWiFiSsid;
var credWiFiPassword;

//global variables
var reportedData;
var desiredData;
var jsonDataDesired;


function getThingStatus()
{
	// If this is not the active.html page, just return
	if (document.URL.toString().search('active.html') < 0) return;

	try
	{
		var params =
		{
			thingName: credThingName
		};
		iotdata.getThingShadow(params, function(err, data)
		{
			if (err) 
				console.log('errors: '+err, err.stack); // an error occurred
			else
			{
				reportedData = JSON.parse(data.payload).state.reported;
				desiredData = JSON.parse(data.payload).state.desired
			}
			//buttonPressed++;
		});
	}
	catch (ex)
	{
		// do nothing
	}
}

function updateThingStatus()
{
	// If this is not the active.html page, just return
	if (document.URL.toString().search('active.html') < 0) return;

	try
	{
		var params =
		{
			payload: jsonDataDesired,
			thingName: credThingName
		};
		
		iotdata.updateThingShadow(params, function(err, data)
		{
			if (err) console.log(err, err.stack);
			else console.log(data);
		});
	}
	catch (ex)
	{
		// do nothing
	}
}

function initAws(credAccessKey, credSecretKey)
{
/*
	endpointElements = credThingEndpoint.split(".");
	credRegion = endpointElements[2];
	endpointElements.shift();
	credAwsEndpoint = endpointElements.join(".");
*/

	// Create the security admin AWS connection
	// Initialize the AWS IoT connection
	credAwsEndpoint = `iot.${credRegion}.amazonaws.com`;
	var options = 
	{
		accessKeyId: credAccessKey,
		secretAccessKey: credSecretKey,
		region: credRegion,
		endpoint: credAwsEndpoint,
		apiVersion: awsApiVersion
	}
	awsIot = new AWS.Iot(options);

	// Get the thing endpoint
	var params = {};
	awsIot.describeEndpoint(params, function(err, data)
	{
		if (err) console.log(err, err.stack); // an error occurred
		else
		{
			// successful response
			credThingEndpoint = data.endpointAddress;
			console.log('Thing Endpoint: ' + credThingEndpoint);
			
			// Now call the thing init.  It needs the credThingEndpoint.
			initThing(credAccessKey, credSecretKey);
		}
	});
}

function initThing(credAccessKey, credSecretKey)
{
	// Create the shadow AWS IoT connection
	var options =
	{
		accessKeyId: credAccessKey,
		secretAccessKey: credSecretKey,
		region: credRegion,
		endpoint: credThingEndpoint,
	}
	iotdata = new AWS.IotData(options);

/*
	// Set the thingName in the title bar.
	document.getElementById("thingname").innerHTML = "AWS Thing Name: " + credThingName;
*/

	// Set the refresh intervals
	setInterval(updateAMIStatus, UPDATE_RATE);
}

// This function set the button as colored when called
function setButtonColor(buttonId, buttonValue)
{
	// Load which button by DOM ID
	var whichButton = document.getElementById(buttonId);
	// If false, then set to the following values
	if (buttonValue === "up")
	{
		whichButton.class = "button_icon";
	}
	else if (buttonValue === "down")
	{
		// If true, then set to the following values
		whichButton.class = "button_icon mdl-color--primary";
	}
}

/*
function setPotentiometerValue(potentiometerValue)
{
	// Update the text to show the actual potentiometer value
	document.getElementById("potValue").innerHTML = potentiometerValue;
	
	// Now we have to update the progress bar
	potPercentValue = Math.round((potentiometerValue * 100) / 1024);
	
	document.querySelector('.mdl-js-progress').MaterialProgress.setProgress(potPercentValue);
}
*/

// This function is called on a timer and updates the status of the web page if there are changes in the database
function updateAMIStatus()
{
	// If this is not the active.html page, just return
	if (document.URL.toString().search('active.html') < 0) return;

	// Get the current status of the thingShadow registers
	getThingStatus();
	
/*
*/
	try
	{
		//this code is very messy will need to clean up
		if(reportedData.button1 === "down")
		{
			//Set the MDL - color primary
			document.getElementById("button1").className += " mdl-color--primary";
		}
		else
		{
			//Remove the MDL - color primary
			document.getElementById("button1").className = "button_icon"
		}
		//Now lets do the remaining buttons
		if(reportedData.button2 === "down")
		{
			//Set the MDL - color primary
			document.getElementById("button2").className += " mdl-color--primary";
		}
		else
		{
			//Remove the MDL - color primary
			document.getElementById("button2").className = "button_icon"
		}
		if(reportedData.button3 === "down")
		{
			//Set the MDL - color primary
			document.getElementById("button3").className += " mdl-color--primary";
		}
		else
		{
			//Remove the MDL - color primary
			document.getElementById("button3").className = "button_icon"
		}
		//now we have to update the frontpage leds
		if(desiredData.led1 === "on")
		{
			document.querySelectorAll('.mdl-js-switch')[0].MaterialSwitch.on();
		}
		else
		{
			document.querySelectorAll('.mdl-js-switch')[0].MaterialSwitch.off();
		}
		if(desiredData.led2 === "on")
		{
			document.querySelectorAll('.mdl-js-switch')[1].MaterialSwitch.on();
		}
		else
		{
			document.querySelectorAll('.mdl-js-switch')[1].MaterialSwitch.off();
		}
		if(desiredData.led3 === "on")
		{
			document.querySelectorAll('.mdl-js-switch')[2].MaterialSwitch.on();
		}
		else
		{
			document.querySelectorAll('.mdl-js-switch')[2].MaterialSwitch.off();
		}
	}
	catch (ex)
	{
		// do nothing
	}
}


function onLedPress(ledId)
{
	// This function will be called when a LED button is pressed
	//formats the JSON payload and updates the thingShadow registers
	switch (ledId)
	{
	case "led1":
		// led1Value = 'off';
		if(document.getElementById('switch-1').checked === true){
			led1Value = 'on';
		document.querySelectorAll('.mdl-js-switch')[0].MaterialSwitch.on();
		}else {
			led1Value = 'off';
			document.querySelectorAll('.mdl-js-switch')[0].MaterialSwitch.off();
		};
		jsonDataDesired = JSON.stringify({"state":{"desired":{"led1" : led1Value}}});
		// if(document.getElementById('switch-1').checked === true) led1Value = 'on';
		// jsonDataDesired = JSON.stringify({"state":{"desired":{"led1" : led1Value}}});
		break;
	case "led2":
		// led2Value = 'off';
		if(document.getElementById('switch-2').checked === true){
			led2Value = 'on';
			document.querySelectorAll('.mdl-js-switch')[1].MaterialSwitch.on();
		}else {
			led2Value = 'off';
			document.querySelectorAll('.mdl-js-switch')[1].MaterialSwitch.off();
		};
		jsonDataDesired = JSON.stringify({"state":{"desired":{"led2" : led2Value}}});
		// if(document.getElementById('switch-2').checked === true) led2Value = 'on';
		// jsonDataDesired = JSON.stringify({"state":{"desired":{"led2" : led2Value}}});
		break;
	case "led3":
		// led3Value = 'off';
		if(document.getElementById('switch-3').checked === true){
			led3Value = 'on';
			document.querySelectorAll('.mdl-js-switch')[2].MaterialSwitch.on();
		}else {
			led3Value = 'off';
			document.querySelectorAll('.mdl-js-switch')[2].MaterialSwitch.off();
		};
		jsonDataDesired = JSON.stringify({"state":{"desired":{"led3" : led3Value}}});
		// if(document.getElementById('switch-3').checked === true) led3Value = 'on';
		// jsonDataDesired = JSON.stringify({"state":{"desired":{"led3" : led3Value}}});
		break;
	case "led4":
		// led4Value = 'off';
		if(document.getElementById('switch-4').checked === true){
			led4Value = 'on';
			document.querySelectorAll('.mdl-js-switch')[3].MaterialSwitch.on();
		}else {
			led4Value = 'off';
			document.querySelectorAll('.mdl-js-switch')[3].MaterialSwitch.off();
		};
		jsonDataDesired = JSON.stringify({"state":{"desired":{"led4" : led4Value}}});
		// if(document.getElementById('switch-4').checked === true) led4Value = 'on';
		// jsonDataDesired = JSON.stringify({"state":{"desired":{"led4" : led4Value}}});
		break;
	}
	//Update the thingStatus
	updateThingStatus();
	updateAMIStatus();
}

// This function checks for the file ~/.mchpiot and if present loads the credentials
// If this file is not there sends to settings.html to enter and save credentials
function checkAndLoad()
{
	var homeDirectory = process.env.HOME || process.env.USERPROFILE;
	var credentialsInFile;
	
	if(operatingSys === 'darwin' || 'linux')
	{
		var fullPathName = homeDirectory + "/.insight";
	}
	else
	{
		var fullPathName = homeDirectory + "\\.insight";
	}
	fs.readFile(fullPathName, function (err, data)
	{
		if (err)
		{
			newCredentials();
		}
		else
		{
			credentialsInFile = JSON.parse(data).credentials;

			//now load the credentials
//			credThingEndpoint = credentialsInFile.endpoint;
			credRegion = credentialsInFile.region;
			credaccess_key = credentialsInFile.access;
			credsecret_key = credentialsInFile.secret;
			credThingName= credentialsInFile.thing;
			credWiFiSsid = credentialsInFile.wifissid;
			credWiFiPassword= credentialsInFile.wifipassword;
			initAws(credaccess_key, credsecret_key);
		}
	});
}

function newCredentials()
{
	window.location = 'settings.html';
	console.log("update Credentials");
}

function saveCredentialsNew()
{
	//this function save a json packet to the file system with the credentials
	savethingname = document.getElementById('thingname').value;
//	saveendpoint = document.getElementById('endpoint').value;
	saveregion = document.getElementById('region').value;
	saveaccess_key = document.getElementById('accesskey').value;
	savesecret_key = document.getElementById('secretkey').value;
	savewifissid = document.getElementById('wifissid').value;
	savewifipassword = document.getElementById('wifipassword').value;

	var credentialsToSave = JSON.stringify(
	{
		"credentials":
		{
//			"endpoint" : saveendpoint,
			"region" : saveregion,
			"access" : saveaccess_key,
			"secret" : savesecret_key,
			"thing" : savethingname,
			"wifissid" : savewifissid,
			"wifipassword" : savewifipassword
		}
	});
	var homeDirectory = process.env.HOME || process.env.USERPROFILE;
	fs.writeFile(homeDirectory + '/.insight', credentialsToSave, function(err)
	{
		console.log('Error saving credentials');
	})
	window.location = 'securething.html';
}

function populateCredentials(new_creds)
{
	if (new_creds === 'NO')
	{
		checkAndLoad();
		//load the creds from file...
		setTimeout(function()
		{
			console.log('loading credentials');
			document.getElementById('thingname').value = credThingName;
			$('#thingname').parent('div').addClass('is-dirty');
//			document.getElementById('endpoint').value = credThingEndpoint;
//			$('#region').parent('div').addClass('is-dirty');
			document.getElementById('region').value = credRegion;
			$('#region').parent('div').addClass('is-dirty');
			document.getElementById('accesskey').value = credaccess_key;
			$('#accesskey').parent('div').addClass('is-dirty');
			document.getElementById('secretkey').value = credsecret_key;
			$('#secretkey').parent('div').addClass('is-dirty');
			document.getElementById('wifissid').value = credWiFiSsid;
			$('#wifissid').parent('div').addClass('is-dirty');
			document.getElementById('wifipassword').value = credWiFiPassword;
			$('#wifipassword').parent('div').addClass('is-dirty');
		}, 1000);
		console.log('Loading Credentials');
	}
	else
	{
		console.log('Create new credentials');
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Security functions


// HID globals
var HID = require('node-hid');
var foundAtmelHIDs;
var eccSigner;
var eccRoot;
var HIDString;

// Serial Port globals
var SerialPort= require('serialport');
var eccThing;

// AWS globals
var regCode;
var awsIot;

// Asynchronous communication globals
var rootReply = "";
var signerReply = "";
var thingReply = "";

// State machine for asynchronous communication
var rootState = "";
var signerState = "";
var thingState = "";
var rootSign = true;

// Crypto globals
var rootCert = "";
var verifyCert = "";
var signerCert = "";
var thingCert = "";
var rootPubKey = "";
var signerPubKey = "";

// Constants
var RootSlotId = "03";
var SignerSlotId = "03";
var RootCertId = "00";
var SignerCertId = "01";
var VerifyCertId = "02";
var ThingCertId = "03";

// Function to check both the HID and Serial Port for the required kits
function scanConnections()
{
	scanHIDs();
	scanSerialPorts();
}

function scanHIDs()
{
	//perform a HID and CDC scan
	foundHID = HID.devices();
	console.log('Found Hid Devices '+ foundHID);

	var numberofHIDs = foundHID.length;   //number of entries
	for (var loop = 0; loop < numberofHIDs; loop++ )
	{
		//lets loop through and find the Atmel stuff
		var hidItem = foundHID[loop];
		if (hidItem.manufacturer === "Microchip")
		{
			// A Microchip kit was detected print to the console
			console.log('Microchip Kit Detected: ' + hidItem.path);

			// Now see if it is a Root or signer
			if (hidItem.product.search("Root") > 0 && eccRoot == null)
			{
				// A Root device was detected, Set the current HID device to the Root
				eccRoot = new HID.HID(hidItem.path);

				// Set the callback methods for data and errors
				eccRoot.on("data", receiveRootBytes);
				eccRoot.on("error", receiveRootError);

				// Send a query for the kit name
				rootState = "KitName";
				sendRootBytes("b:f(00)\n");
			}
			else if (hidItem.product.search("Signer") > 0 && eccSigner == null)
			{
				// A Signer device was detected, Set the current HID device to the Signer
				eccSigner = new HID.HID(hidItem.path);

				// Set the callback methods for data and errors
				eccSigner.on("data", receiveSignerBytes);
				eccSigner.on("error", receiveSignerError);

				// Send a query for the kit name
				signerState = "KitName";
				sendSignerBytes("b:f(00)\n");
			}
		}
	}
}
/*
Root Module:
interface: -1
manufacturer: "Microchip"
path: "\\?\hid#vid_04d8&pid_0f30#9&1391e279&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
product: "AWS Root dongle"
productId: 3888
release: 4096
vendorId: 1240

Signer Module:
interface: -1
manufacturer: "Microchip"
path: "\\?\hid#vid_04d8&pid_0f31#8&38085d7a&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
product: "AWS Signer dongle"
productId: 3889
release: 4096
vendorId: 1240
*/

// Scan through the Serial ports and select the first matching port
function scanSerialPorts()
{
	SerialPort.list(function (err, ports)
	{
		ports.forEach(function(port)
		{
			checkSerialPort(port);
		});
	});
}

// Check the serial port and return true if it matches
function checkSerialPort(port)
{
	// Print the port parameters to the log
	console.log('comName: ' + port.comName);
	console.log('pnpId: ' + port.pnpId);
	console.log('manufacturer: ' + port.manufacturer);
	// Check if this is the 'thing'
	var manufacturer = port.manufacturer.toString();	// ATMEL, Inc.
	var pnpId = port.pnpId.toString(); 					// USB\VID_03EB&PID_2404\7&308DAD13&0&3

	// Set the search string for pnpId to 'VID_03EB&PID_2111' for EDBG
	// Set the search string for pnpId to 'VID_03EB&PID_2404' for USB/UART
	if (pnpId.search('VID_03EB&PID_2404') > 0 && eccThing == null)
	{
		// Open the serial port of the thing
		//	autoOpen: false,
		var options = 
		{
			baudRate: 115200,
			parser: SerialPort.parsers.raw
		}
		eccThing = new SerialPort(port.comName, options);

		eccThing.on('open', function()
		{
			// Send a query for the kit name
			thingState = "KitName";
			eccThing.write('b:f(00)\n', function(err)
			{
				if (err)
				{
					return console.log('Error on write: ', err.message);
				}
				console.log('message written');
			});
		});
		
		// Setup the error callback handler
		eccThing.on('error', receiveThingError);

		// Setup the receive callback handler
		eccThing.on('data', receiveThingBytes);
		return true;
	}
	return false;
}

/*
// String-ASCII-Hex conversion tests
var hexStr = strToHexAscii('2460');				// returns '32343630'
var str = hexAsciiToStr('32343630'); 			// returns '2460'
var hexBytes = hexAsciiToByteArray('32343630');	// returns {0x02, 0x04, 0x06, 0x00}
var bytes =  strToByteArray('2460'); 			// returns {0x32, 0x34, 0x36, 0x30}
//var strBytes = byteArrayToStr(bytes);			// returns '2460'
*/

// strToHexAscii('2460'); // returns '32343630'
function strToHexAscii(str)
{
	// Force conversion of str to string
	var txt = str.toString();
	var arr = [];
	for (var i = 0, l = txt.length; i < l; i=i+1)
	{
		var hex = Number(txt.charCodeAt(i)).toString(16);
		arr.push(hex);
	}
	return arr.join('');
}

// hexAsciiToStr('32343630'); // returns '2460'
function hexAsciiToStr(hexAscii)
{
	// Force conversion of hexAscii to string
	var hex = hexAscii.toString();
	var str = '';
	for (var i = 0; i < hex.length; i=i+2)
		str += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
	return str;
}

// hexAsciiToByteArray('32343630'); // returns {0x02, 0x04, 0x06, 0x00}
function hexAsciiToByteArray(hexAscii)
{
	// Force conversion of str to string
	var hex= hexAscii.toString();
	var arr = [];
	for (var i = 0, l = hex.length; i < l; i=i+2)
	{
		var hexByte = Number(String.fromCharCode(parseInt(hex.substr(i, 2), 16)));
		arr.push(hexByte);
	}
	return arr;
}

// strToByteArray('2460'); // returns {0x32, 0x34, 0x36, 0x30}
function strToByteArray(str)
{
	var map = Array.prototype.map;
	var bytes = map.call(str, function(x) { return x.charCodeAt(0); });
	return bytes;
}

// byteArrayToStr({0x32, 0x34, 0x36, 0x30}); // returns '2460'
function byteArrayToStr(bytes)
{
	// Simple conversion from bytes to string... Just create a new string using the bytes.
	return new String(bytes);
}

function sendThingBytes(data)
{
	// Pre-condition the data to send for kit protocol... 
	var strData = data.toString();

	// Convert the string to bytes for sending
	var map = Array.prototype.map;
	var bytes = map.call(strData, function(x) { return x.charCodeAt(0); });
	console.log("thingSend: " + data);
	//console.log("data bytes: " + bytes);
	
	// Clear the reply bytes for a new reply
	thingReply = "";

	// Get the number of bytes in the bytes array.
	var maxComBytes= 1024;
	if (data.length > maxComBytes)
	{
		// Send maxComBytes bytes at a time
		var bytesLength = bytes.length ;
		console.log('The array is ' + bytesLength + ' bytes. Writing ' + maxComBytes+ ' bytes at a time');
		// Loop through the array
 		do
		{
			// Take a chunk of maxComBytes
			// Note: The byte at endIndex is not included in splice result
			var comChunk = bytes.splice(0, maxComBytes);

			eccThing.write(comChunk );
		} while (bytes.length > 0);
	}
	else
	{
		eccThing.write(bytes);
	}
}

function receiveThingBytes(reply)
{
	var checkReply = { partialReply: reply, replyStr: thingReply };
	var fullReply = checkKitReply(checkReply);
	thingReply = checkReply.replyStr;
	if (!fullReply)
	{
		//console.log('receiveThingBytes partial reply: ' + thingReply);
		return;
	}
	console.log('thingState: ' + thingState);
	console.log('thingReply: ' + thingReply);

	// Parse the message
	var name = kitMsgTxt(thingReply);
	var status = kitStatus(thingReply);
	var data = kitData(thingReply);

	// If the status is not 00, then alert and stop
	if (status != "00")
	{
		alert("Error communicating to Thing.\Thing Status: " + status);
		thingState = "";
		return;
	}
	switch (thingState)
	{
	case "KitName":

		// Set the GUI string if we are on the securething.html page
		if (document.URL.toString().search('securething.html') >= 0)
		{
			document.getElementById('awsthingstring').value = name;
			$('#awsthingstring').parent('div').addClass('is-dirty');
		}
		// Nothing to do next
		thingState = "";
		break;
	case "Init":
		// Receive the TBS from the Init command sent from signerState="PubKey"
		var tbs = data;

		// Send the TBS reply to the Signer to be signed
		sendSignerBytes('aws:si(' + SignerSlotId + ',' + tbs + ')\n');
		signerState = "SignThing";
		break;
	case "SaveSig":
		// Receive the reply to the SaveSig command sent from signerState="SignThing"
		// Should be status success
		
		// Save the AWS host information 
		var chunkId = '00';
		sendThingBytes('aws:sh(' + strToHexAscii(credThingEndpoint) + ',' + strToHexAscii(credThingName) + ')\n');
		thingState = "SaveHostInfo";
		break;
	case "SaveHostInfo":
		// Receive the reply to the SaveHostInfo command 
		// Should be status success

		// Request the Thing certificate 
		var chunkId = '00';
		sendThingBytes('aws:g(' + ThingCertId + ',' + chunkId  + ')\n');
		thingState = "GetThingCert";

		// Clear any previous contents of the signer certificate
		thingCert = "";
		break;
	case "GetThingCert":
		// Receive the Thing certificate in one call, no chunks
		//var checkCert = {certId: ThingCertId, partialCert: kitData(thingReply), certificate: thingCert };
		//var fullCert = checkCertReply(checkCert);
		//thingCert = checkCert.certificate;
		//if (!fullCert) break;
		
		// Log the thingCert to the console
		thingCert = hexAsciiToStr(data);
		console.log(thingCert);

		// Save the signer certificate 
		sendThingBytes('aws:sc(' + SignerCertId + ',' + signerCert  + ')\n');
		thingState = "SaveSignerCert";
		break;
	case "SaveSignerCert":
		// Receive the reply to the SaveSignerCert command 
		// Should be status success

		// Register the AWS thing
		awsRegisterThing();

		// Save the WiFi credentials 
		sendThingBytes('aws:sw(' + strToHexAscii(credWiFiSsid) + ',' + strToHexAscii(credWiFiPassword) + ')\n');
		thingState = "SaveWiFiCred";
		break;
	case "SaveWiFiCred":
		// Receive the reply to the SaveWiFiCred command 
		// Should be status success
		
		// If we got here successfully, then navigate to the active.html page
		//window.location = 'active.html';

		// Nothing to do next
		thingState = "";
		break;
	default:
		// Not sure what signer bytes were sent.  Log to the console.
		console.log("receiveThingBytes(): thingState not set.\ndata: " + thingReply);

		// Clear the state of the expected reply
		thingState = "";
		break;
	}
}

function receiveThingError(error)
{
	// Send an alert message to the user 
	alert('Thing Communication: ' + error);
	console.log('Thing Communication: ' + error);
}

function sendRootBytes(data)
{
	// Pre-condition the data to send for kit protocol... Remove white space and internal \n
	var strData = data.toString();

	// Convert the string to bytes for sending
	var map = Array.prototype.map;
	var bytes = map.call(strData, function(x) { return x.charCodeAt(0); });
	var reportIdByte = 0x00;
	console.log("rootSend: " + data);
	//console.log("data bytes: " + bytes);

	// Clear the reply bytes for a new reply
	rootReply = "";

	// Get the number of bytes in the bytes array.
	var maxHidBytes = 64;
	if (data.length > maxHidBytes)
	{
		// Send 64 bytes at a time
		var bytesLength = bytes.length ;
		console.log('The array is ' + bytesLength + ' bytes. Writing ' + maxHidBytes + ' bytes at a time');
		// Loop through the array
 		do
		{
			// Take a chunk of maxHidBytes
			// Note: The byte at endIndex is not included in splice result
			var hidChunk = bytes.splice(0, maxHidBytes);
			if (operatingSys === 'win32')
			{
				hidChunk.splice(0, 0, reportIdByte );
			}
			//console.log("hidChunk bytes: " + hidChunk);
			eccRoot.write(hidChunk);
		} while (bytes.length > 0);
	}
	else
	{
		// Add the prefix for Windows, then write the bytes
		if (operatingSys === 'win32')
		{
			bytes.splice(0, 0, reportIdByte );
		}
		eccRoot.write(bytes);
	}
}

function receiveRootBytes(reply)
{
	var checkReply = { partialReply: reply, replyStr: rootReply };
	var fullReply = checkKitReply(checkReply);
	rootReply = checkReply.replyStr;
	if (!fullReply)
	{
		//console.log('receiveSignerBytes partial reply: ' + rootReply);
		return;
	}
	console.log('rootState: ' + rootState);
	console.log('rootReply: ' + rootReply);

	// Parse the message
	var name = kitMsgTxt(rootReply);
	var status = kitStatus(rootReply);
	var data = kitData(rootReply);

	// If the status is not 00, then alert and stop
	if (status != "00")
	{
		alert("Error communicating to Root.\nRoot Status: " + status);
		rootState = "";
		return;
	}
	switch (rootState)
	{
	case "KitName":
		// Receive the Kit Name

		// Set the GUI string if we are on the securething.html page
		if (document.URL.toString().search('securething.html') >= 0)
		{
			document.getElementById('rootstring').value = name;
			$('#rootstring').parent('div').addClass('is-dirty');
		}
		// Clear the state of the expected reply
		rootState = "";
		break;
	case "PubKey":
		// Receive the Root Public key 
		rootPubKey = data;

		// Request the root certificate 
		var chunkId = '00';
		sendRootBytes('aws:g(' + RootCertId + ',' + chunkId  + ')\n');
		rootState = "GetRootCert";

		// Clear any previous contents of the signer certificate
		rootCert = "";
		break;
	case "GetRootCert":
		// Receive the Root certificate in chunks
		var checkCert = {certId: RootCertId, partialCert: data, certificate: rootCert };
		var fullCert = checkCertReply(checkCert);
		rootCert = checkCert.certificate;
		if (!fullCert) break;
		// Print Root Certificate to the console
		console.log('Root Certificate:\n' + rootCert);

		// We have the root certificate, do the next step
		if (signerState == "Init")
		{
			// Check that the regCode is initilized
			if (regCode != null)
			{
				// During Signer initialization, send the signer init command
				sendSignerBytes('aws:i(' + regCode + ',' + rootPubKey + ')\n');
				signerState = "Init";
			}
			else
			{
				alert("Could not initialize Registration Code. Check AWS credenials");
				signerState = "";
			}
		}
		// Nothing else for the Root to do, clear the state of the expected reply
		rootState = "";
		break;
	case "SignSigner":
	case "Sign":
		// Receive the Signature from the Sign command
		var sig = data;

		if (rootState == "SignSigner")
		{
			// Send the Signature to the Signer to be saved
			sendSignerBytes('aws:ss(' + SignerCertId + ',' + sig + ')\n');
			signerState = "SaveSig";
		}
		break;
	default:
		// Not sure what signer bytes were sent.  Log to the console.
		console.log("receiveRootBytes(): signerState not set.\ndata: " + rootReply);

		// Clear the state of the expected reply
		rootState = "";
		break;
	}
}

function receiveRootError(error)
{
	// Send an alert message to the user
	alert('Root Communication: ' + error);
	console.log('Root Communication: ' + error);
}

function sendSignerBytes(data)
{
	// Pre-condition the data to send for kit protocol... Remove white space and internal \n
	var strData = data.toString();

	// Convert the string to bytes for sending
	var map = Array.prototype.map;
	var bytes = map.call(strData, function(x) { return x.charCodeAt(0); });
	var reportIdByte = 0x00;
	console.log("signerSend: " + data);
	//console.log("data bytes: " + bytes);
	
	// Clear the reply bytes for a new reply
	signerReply = "";

	// Get the number of bytes in the bytes array.
	var maxHidBytes = 64;
	if (data.length > maxHidBytes)
	{
		// Send 64 bytes at a time
		var bytesLength = bytes.length ;
		console.log('The array is ' + bytesLength + ' bytes. Writing ' + maxHidBytes + ' bytes at a time');
		// Loop through the array
 		do
		{
			// Take a chunk of maxHidBytes
			// Note: The byte at endIndex is not included in splice result
			var hidChunk = bytes.splice(0, maxHidBytes);
			if (operatingSys === 'win32')
			{
				hidChunk.splice(0, 0, reportIdByte );
			}
			//console.log("hidChunk bytes: " + hidChunk);
			eccSigner.write(hidChunk);
		} while (bytes.length > 0);
	}
	else
	{
		// Add the prefix for Windows, then write the bytes
		if (operatingSys === 'win32')
		{
			bytes.splice(0, 0, reportIdByte );
		}
		eccSigner.write(bytes);
	}
}

function receiveSignerBytes(reply)
{
	var checkReply = { partialReply: reply, replyStr: signerReply };
	var fullReply = checkKitReply(checkReply);
	signerReply = checkReply.replyStr;
	if (!fullReply)
	{
		//console.log('receiveSignerBytes partial reply: ' + signerReply);
		return;
	}
	console.log('signerState: ' + signerState);
	console.log('signerReply: ' + signerReply);

	// Parse the message
	var name = kitMsgTxt(signerReply );
	var status = kitStatus(signerReply );
	var data = kitData(signerReply );

	// If the status is not 00, then alert and stop
	if (status != "00")
	{
		alert("Error Communicating to Signer.\nSigner Status: " + status);
		SignerState = "";
		return;
	}
	switch (signerState)
	{
	case "KitName":
		// Receive the Kit Name

		// Set the GUI string if we are on the securething.html page
		if (document.URL.toString().search('securething.html') >= 0)
		{
			document.getElementById('signerstring').value = name;
			$('#signerstring').parent('div').addClass('is-dirty');
		}
		// Clear the state of the expected reply
		signerState = "";
		break;
	case "Init":
		// Receive the TBS from the Init command
		var tbs = data;

		// Send the TBS reply to be signed
		if (rootSign == true)
		{
			sendRootBytes('aws:si(' + RootSlotId + ',' + tbs + ')\n');
			rootState = "SignSigner";
		}
		else
		{
			// Signer self-sign
			sendSignerBytes('aws:si(' + SignerSlotId + ',' + tbs + ')\n');
			signerState = "Sign";
		}
		break;
	case "SignThing":
	case "Sign":
		// Receive the Signature from the Sign command
		var sig = data;

		if (signerState == "SignThing")
		{
			// Send the Signature to the Thing to be saved
			sendThingBytes('aws:ss(' + ThingCertId + ',' + sig + ')\n');
			thingState = "SaveSig";
		}
		else if (rootSign == false)
		{
			// Send the Signature to the Signer to be saved
			sendSignerBytes('aws:ss(' + SignerCertId + ',' + sig + ')\n');
			signerState = "SaveSig";
		}
		break;
	case "SaveSig":
		// Receive the reply to the SaveSig command
		// Should be status success

		// Request the Signer certificate 
		var chunkId = '00';
		sendSignerBytes('aws:g(' + SignerCertId + ',' + chunkId  + ')\n');
		signerState = "GetSignerCert";

		// Clear any previous contents of the signer certificate
		signerCert = "";
		break;
	case "GetSignerCert":
		// Receive the Signer certificate in chunks
		var checkCert = {certId: SignerCertId, partialCert: data, certificate: signerCert };
		var fullCert = checkCertReply(checkCert);
		signerCert = checkCert.certificate;
		if (!fullCert) break;

		// We have the signer public key, do the next step
		if (thingState == "Init")
		{
			// During Thing initialization, we're done with signer queries
			// Initiate the thing initialization sequence. Initialize the Thing by sending the signer public key.
			sendThingBytes('aws:i(' + signerPubKey + ')\n');
			thingState = "Init";
		}
		else
		{
			// Request the Verification certificate
			var chunkId = '00';
			sendSignerBytes('aws:g(' + VerifyCertId + ',' + chunkId  + ')\n');
			signerState = "GetVerifyCert";
		}
		// Clear any previous contents of the verification certificate
		verifyCert = "";
		break;
	case "GetVerifyCert":
		// Receive the Verification certificate in chunks
		var checkCert = {certId: VerifyCertId, partialCert: data, certificate: verifyCert };
		var fullCert = checkCertReply(checkCert);
		verifyCert = checkCert.certificate;
		if (!fullCert) break;

		// Register the Signer with AWS
		awsRegisterSigner();

		// Request the signer public key
		sendSignerBytes('aws:p(' + SignerSlotId + ')\n');
		signerState = "PubKey";
		break;
	case "PubKey":
		// Receive the signer Public key 
		signerPubKey = data;

		// We have the signer public key, do the next step
		if (thingState == "Init")
		{
			// During Thing initialization, get the Signer certificate next
			var chunkId = '00';
			sendSignerBytes('aws:g(' + SignerCertId + ',' + chunkId  + ')\n');
			signerState = "GetSignerCert";
			// Clear any previous contents of the signer certificate
			signerCert = "";
		}
		else
		{
			// Nothing else to do, clear the state of the expected reply
			signerState = "";
		}
		break;
	default:
		// Not sure what signer bytes were sent.  Log to the console.
		console.log("receiveSignerBytes(): signerState not set.\ndata: " + signerReply);

		// Clear the state of the expected reply
		signerState = "";
		break;
	}
}

function receiveSignerError(error)
{
	// Send an alert message to the user
	alert('Signer Communication: ' + error);
	console.log('Signer Communication: ' + error);
}

// Construct a full kit reply
function checkKitReply(checkReply)
{
	// Convert the reply to a string
	var str = byteArrayToStr(checkReply.partialReply);
	
	// Trim the 00s from the reply
	str.trim();
	
	// Concat the partial reply to the reply
	checkReply.replyStr = checkReply.replyStr.concat(str);

	// A complete message will include a \n
	if(str.indexOf('\n') != -1)
	{
		// the message is complete
		return true;
	}
	return false;
}

// Construct a full certificate
function checkCertReply(checkCert)
{
	// Convert the reply to a string
	var str = byteArrayToStr(checkCert.partialCert);

	// Trim the 00s from the reply
	str.trim();

	// Parse the chunk id, total number of chunks, and this partial cert chunk
	var chunkId = parseInt(str[0]);
	var totalChunks = parseInt(str[1]);
	var certChunk = str.slice(2);

	// Concat the reply to the partial reply
	var certChunkStr = hexAsciiToStr(certChunk);
	checkCert.certificate = checkCert.certificate.concat(certChunkStr);

	// If this is not the last chunk, then request the next chunk. Id is 0 indexed.
	var nextChunkId = chunkId + 1;
	if (nextChunkId < totalChunks)
	{
		// Get the next chunk
		var nextChunkIdStr = "0" + nextChunkId.toString();
		if (checkCert.certId == RootCertId)
		{
			sendRootBytes('aws:g(' + checkCert.certId + ',' + nextChunkIdStr + ')\n');
		}
		else
		{
			sendSignerBytes('aws:g(' + checkCert.certId + ',' + nextChunkIdStr + ')\n');
		}
		return false;
	}
	return true;
}

function kitMsgTxt(kitReply)
{
	// The response from the kit will be in the form:
	// <optionalMessageText> <statusByte>(<optionalData>)\n
	// Example: AT88CKECCSIGNER 00(010005)\n

	// Extract the message text
	var openParenPos = kitReply.indexOf("(");

	// Check the return values
	if (openParenPos == -1) return "";

	// Return the message text slice
	return kitReply.slice(0, openParenPos - 2);
}

function kitStatus(kitReply)
{
	// The response from the kit will be in the form:
	// <optionalMessageText> <statusByte>(<optionalData>)\n
	// Example: AT88CKECCSIGNER 00(010005)\n

	// Extract the status byte
	var openParenPos = kitReply.indexOf("(");

	// Check the return values
	if (openParenPos == -1) return "";

	// Return the status slice converted to a byte
	var statusStr = kitReply.slice(openParenPos - 2, openParenPos);
	return statusStr;
	//return statusStr.charCodeAt(0);
}

function kitData(kitReply)
{
	// The response from the kit will be in the form:
	// <optionalMessageText> <statusByte>(<optionalData>)\n
	// Example: AT88CKECCSIGNER 00(010005)\n

	// Extract the data bytes
	var openParenPos = kitReply.indexOf("(");
	var closeParenPos = kitReply.indexOf(")");

	// Check the return values
	if (closeParenPos == -1 || openParenPos == -1) return "";

	// Return the data slice
	return kitReply.slice(openParenPos + 1, closeParenPos);
}

function registerSigner()
{
	// First scan connections
	scanConnections();

	// Check that the root is attached
	if (eccRoot == null)
	{
		// If not attached, then prompt the user to insert
		alert("Please insert the USB Root.");
		return;
	}
	// Check that the signer is attached
	if (eccSigner == null)
	{
		// If not attached, then prompt the user to insert
		alert("Please insert the USB Signer.");
		return;
	}
	// Get the registration code for the current account
	getRegCode();

	// Initiate the Signer registration sequence. 
	signerState = "Init";

	// Start by getting the Root public key
	sendRootBytes("aws:p(" + RootSlotId + ")\n");
	rootState = "PubKey";
}

function prepareThing()
{
	// First scan connections
	scanConnections();

	// Check that the thing is attached
	if (eccThing == null)
	{
		// If not attached, then prompt the user to insert
		alert("Please attach the thing.");
		return;
	}
	// Check that a signer is attached
	if (eccSigner == null)
	{
		// If not attached, then prompt the user to insert
		alert("Please insert the USB Signer.");
		return;
	}

	// Set the thingState to "Init" while the signer information is gathered
	thingState = "Init";

	// Start by getting the signer public key
	sendSignerBytes('aws:p(' + SignerSlotId + ')\n');
	signerState = "PubKey";
}

// Get the registration code
function getRegCode()
{
	// Get the registration code
	try
	{
		// Get the registration code from the current account 
		var params = {};
		awsIot.getRegistrationCode(params, function(err, data)
		{
			if (err)
			{
				// an error occurred
				console.log(err, err.stack);
			}
			else
			{
				// successful response
				regCode = data.registrationCode;
			}
		});
	}
	catch (ex)
	{
		alert("getRegCode Exception: " + ex);
		return false;
	}
	// Log the registration code.  Return false if it is undefined.
	console.log('regCode: ' + regCode);
	if (regCode == null) return false;
	
	// If we got here, then everything is ok
	return true;
}

function awsRegisterSigner()
{
	// Print the certificates
	console.log('signerCert:\n' + signerCert);
	console.log('verifyCert:\n' + verifyCert);
	
	// Send the signer & verification cert to AWS
	var params =
	{
		caCertificate: signerCert, /* required */
		verificationCertificate: verifyCert, /* required */
		setAsActive: true,
		allowAutoRegistration: true
	};
	awsIot.registerCACertificate(params, function(err, data)
	{
		if (err) console.log(err, err.stack); // an error occurred
		else     console.log(data);           // successful response
	});
}

// {certificateArn: '', certificateId: '' }
var thingCert;
function awsRegisterThing()
{
	// Print the certificates
	console.log('thingCert:\n' + thingCert);
	console.log('signerCert:\n' + signerCert);
	
	// Send the thing cert to AWS
	var params =
	{
		certificatePem: thingCert, /* required */
		caCertificatePem: signerCert,
		setAsActive: true
	};
	awsIot.registerCertificate(params, function(err, data)
	{
		if (err) console.log(err, err.stack); // an error occurred
		else
		{
			// successful response
			console.log(data);
			thingCert = data;
		}
		awsGetPolicy();
	});
}

// {policyName : '', policyArn: '', policyDocument : '', defaultVersionId : '' }
var pubSubAnyTopic;
function awsGetPolicy()
{
	var params =
	{
		policyName: awsPolicyName  /* required */
	};
	awsIot.getPolicy(params, function(err, data)
	{
		if (err)
		{
			// console.log(err, err.stack)
			// could not get policy, create it
			awsCreatePolicy();
		}
		else
		{
			// successful response
			console.log(data);
			pubSubAnyTopic = data;

			//awsCreatePolicy();
			awsAttachPolicy();
		}
	});
}

function awsCreatePolicy()
{
    // Policy definition
    var policy =
    {
		"Version": "2012-10-17",
		"Statement":
		[{
			"Effect": "Allow",
			"Action":["iot:*"],
			"Resource": ["*"]
		}]
    };
    // Create the policy
    awsIot.createPolicy(
    {
        policyDocument: JSON.stringify(policy),
        policyName: awsPolicyName 
    }, (err, data) =>
    {
        //Ignore if the policy already exists
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException'))
        {
            console.log(err);
            callback(err, data);
            return;
        }
        pubSubAnyTopic = data;
        console.log(data);

		//awsCreatePolicy();
		awsAttachPolicy();
    });
}

function awsAttachPolicy()
{
	// Attach policy to certificate
	awsIot.attachPrincipalPolicy(
	{
		policyName: pubSubAnyTopic.policyName,
		principal: thingCert.certificateArn
	}, (err, data) =>
	{
		//Ignore if the policy is already attached
		if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException'))
		{
			console.log(err);
			callback(err, data);
			return;
		}
		console.log(data);
	});
}

/** 
///////////////////////////////////////////////////////////////////////
// Test Lambda Function Implementaion
exports.handler = function(event, context, callback) {
    
    //Replace it with the AWS region the lambda will be running in
    var region = "us-west-2";
    
    // Make a local accountId variable
    //var accountId = "299183337826";
    var accountId = event.awsAccountId.toString().trim();

    var iot = new AWS.Iot({'region': region, apiVersion: '2015-05-28'});
    var certificateId = event.certificateId.toString().trim();
    
    //Replace it with your desired topic prefix
    //var topicName = `$aws/things/light_test/shadow/update/${certificateId}`

    var certificateARN = `arn:aws:iot:${region}:${accountId}:cert/${certificateId}`;
    var policyName = 'Policy_' + certificateId;
    
    //Policy that allows connect, publish, subscribe and receive
    var policy = {
	"Version": "2012-10-17",
	"Statement": [{
		"Effect": "Allow",
		"Action":["iot:*"],
		"Resource": ["*"]
	}]
    };

    //
    // Step 1) Create a policy
    //
    iot.createPolicy({
        policyDocument: JSON.stringify(policy),
        policyName: policyName
    }, (err, data) => {
        //Ignore if the policy already exists
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException')) {
            console.log(err);
            callback(err, data);
            return;
        }
        console.log(data);

    });
 
    //
    // Step 2) Attach the policy to the certificate
    //
    iot.attachPrincipalPolicy({
        policyName: policyName,
        principal: certificateARN
    }, (err, data) => {
        //Ignore if the policy is already attached
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException')) {
            console.log(err);
            callback(err, data);
            return;
        }
        console.log(data);
    });
    
    //
    // Step 3) Activate the certificate. Optionally, you can have your custom Certificate Revocation List (CRL) check
    // logic here and ACTIVATE the certificate only if it is not in the CRL .Revoke the certificate if it is in the CRL
    //
    iot.updateCertificate({
        certificateId: certificateId,
        newStatus: 'ACTIVE'
    }, (err, data) => {
        if (err) {
            console.log(err, err.stack); 
            callback(err, data);
        }
        else {
            console.log(data);   
            callback(null, "Success, created, attached policy and activated the certificate " + certificateId);
        }
    });
}

///////////////////////////////////////////////////////////////////////
// Reference Lambda Function Implementaion
This node.js Lambda function code creates and attaches an IoT policy to the certificate
registered. It also activates the certificate. The Lambda function is attached as the rules engine action to
the registration topic aws/events/certificates/registered/<caCertificateID>
**/
/*
var AWS = require('aws-sdk');
    
exports.handler = function(event, context, callback) {
    
    //Replace it with the AWS region the lambda will be running in
    var region = "us-east-1";
    
    //Replace it with your AWS account ID
    var accountId = "XXXXXXXXXXXX";

    var iot = new AWS.Iot({'region': region, apiVersion: '2015-05-28'});
    var certificateId = event.certificateId.toString().trim();
    
     //Replace it with your desired topic prefix
    var topicName = `foo/bar/${certificateId}`

    var certificateARN = `arn:aws:iot:${region}:${accountId}:cert/${certificateId}`;
    var policyName = 'Policy_' + certificateId;
    
    //Policy that allows connect, publish, subscribe and receive
    var policy = {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Effect": "Allow",
                "Action": [
                    "iot:Connect"
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}:client/${certificateId}`
            },
            {
                "Effect": "Allow",
                "Action": [
                    "iot:Publish",
                    "iot:Receive"
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}:topic/${topicName}/*`
            },
            {
                "Effect": "Allow",
                "Action": [
                    "iot:Subscribe",
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}:topicfilter/${topicName}/#`
            }
        ]
    };

    //
    // Step 1) Create a policy
    //
    iot.createPolicy({
        policyDocument: JSON.stringify(policy),
        policyName: policyName
    }, (err, data) => {
        //Ignore if the policy already exists
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException')) {
            console.log(err);
            callback(err, data);
            return;
        }
        console.log(data);

    });
 
    //
    // Step 2) Attach the policy to the certificate
    //
    iot.attachPrincipalPolicy({
        policyName: policyName,
        principal: certificateARN
    }, (err, data) => {
        //Ignore if the policy is already attached
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException')) {
            console.log(err);
            callback(err, data);
            return;
        }
        console.log(data);
    });
    
    //
    // Step 3) Activate the certificate. Optionally, you can have your custom Certificate Revocation List (CRL) check
    // logic here and ACTIVATE the certificate only if it is not in the CRL. Revoke the certificate if it is in the CRL
    //
    iot.updateCertificate({
        certificateId: certificateId,
        newStatus: 'ACTIVE'
    }, (err, data) => {
        if (err) {
            console.log(err, err.stack); 
            callback(err, data);
        }
        else {
            console.log(data);   
            callback(null, "Success, created, attached policy and activated the certificate " + certificateId);
        }
    });
}
*/

