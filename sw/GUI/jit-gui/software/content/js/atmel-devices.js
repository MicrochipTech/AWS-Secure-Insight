var HID = require('node-hid');
var foundAtmelHIDs;
var ECC508;
var HIDString;

// Scan for Atmel kits
scanHIDs();

function scanHIDs(){
	//perform a HID and CDC scan
	foundHID = HID.devices();
	console.log('Found Hid Devices '+foundHID);
	
	var numberofHIDs = foundHID.length;   //number of entries
	for (var loop = 0; loop < numberofHIDs; loop++ )
	{
		//lets loop through and find the Atmel stuff
		if (foundHID[loop].manufacturer === "ATMEL")
		{
			console.log('PID = '+foundHID[loop].productId);
			console.log('VID = '+foundHID[loop].vendorId);
			console.log('Path = '+foundHID[loop].path);
			
			opentheHIDdevice(foundHID[loop].path);
		}
	}
}


function opentheHIDdevice(hidpath)
{
	ECC508 = new HID.HID(hidpath);
	
	ECC508.on("data", function(data)
	{
		console.log("Data returned: " + data);
		HIDString = data.toString();
		if(HIDString.search('ROOT') > 1)
		{
			document.getElementById('rootversion').value = data;
			$('#rootversion').parent('div').addClass('is-dirty');
			document.getElementById('rootpath').value = hidpath;
		    $('#rootpath').parent('div').addClass('is-dirty');
		}
		else if(HIDString.search('SIGNER') > 1)
		{
			document.getElementById('signerversion').value = data;
			$('#signerversion').parent('div').addClass('is-dirty');
			document.getElementById('signerpath').value = hidpath;
			$('#signerpath').parent('div').addClass('is-dirty');
		}
	});
	
	ECC508.on("error", function(error)
	{
		console.log('Error returned: '+ error);
	});

// Test 1xx:
//	var ECCCommand = "aws:i(1f3a3692797a8d23f9d896a83c1403ed13111115a06f4dd708de6b15308e9368)\n";

// Test 2xx:
// 200 - Compute the Public key generated from AWS_SIGNING_PRIVATE_KEY_SLOT = 0x03
//	var ECCCommand = "aws:p(03)\n";
// 201 - Compute the Public key generation from AWS_VERIF_PRIVATE_KEY_SLOT = 0x00 
//	var ECCCommand = "aws:p(00)\n";

// Test 3xx:
//	var ECCCommand = "aws:si(03,8CB505A17014D48F4C6140964E584723FB348CB0895AC7077D7F18847DA9B8A9)\n";

// Test 4xx:
// 400 - Wrong number of parameters. cert_id is missing. Must fail with C3()
//	var ECCCommand = "aws:ss(9686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 401 - Wrong number of parameters. Signature is missing. Must fail with C3()
//	var ECCCommand = "aws:ss(00)\n";
// 402 - Incorrect cert_id size. Must be exactly 1 byte. Must fail with C3()
//	var ECCCommand = "aws:ss(0000,9686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 403 - Incorrect cert_id size. Must be exactly 1 byte. Must fail with C3()
//	var ECCCommand = "aws:ss(,9686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 404 - Incorrect signature size (too short). Must be exactly 64 byte. Must fail with C3()
//	var ECCCommand = "aws:ss(01,86EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 405 - Incorrect signature size (too long). Must be exactly 64 byte. Must fail with C3()
//	var ECCCommand = "aws:ss(01,969686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 406 - Incorrect cert_id. Must be AWS_SIGNER_CERT_ID (0x01) for the signer. Must fail with C3()
//	var ECCCommand = "aws:ss(03,9686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";
// 407 - Correctly formatted command. Must return 00()
	var ECCCommand = "aws:ss(01,9686EE16F01C4F127442713613F7B5AFC89795C2F2221D74DAF01E1C1D370362976426930D14DCF2C4CE5D4E28DAC0F55321FB773E38FD09E272351FBA5C8A94)\n";

// Test 5xx:
//	var ECCCommand = "aws:g(02,02)\n";


	SendECCCommand(ECCCommand);
}

function SendECCCommand(ECCCommand)
{
	var map = Array.prototype.map;
	var command = map.call(ECCCommand, function(x) { return x.charCodeAt(0); });
	console.log("command " + ECCCommand);
	console.log("command " + command);
	ECC508.write(command);
}


