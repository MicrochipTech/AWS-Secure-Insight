var HID = require('node-hid');
var ComPort = require('serialport');

var foundAtmelHIDs;
var ECC508;
var HIDString;
var regCode;



// Initialize all aspects of security
function initSecurity()
{
	// Scan the HID & CDC connections
	scanConnections();
}

// Function to check both the HID and Serial Port for the required kits
function scanConnections()
{
	scanHIDs();
	scanCom();
}

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
			
			//Now lets save that into an array for use
			//foundAtmelHIDs[0].productId = foundHID[loop].productId;
			opentheHIDdevice(foundHID[loop].path);
			//console.log('Returned '+returnedstring);
		}
	}
}

function scanCom()
{
	ComPort.list(function (err, ports)
	{
	  ports.forEach(function(port)
	  {
	    console.log(port.comName);
	    console.log(port.pnpId);
	    console.log(port.manufacturer);
	  });
	});
}



function opentheHIDdevice(hidpath)
{
	ECC508 = new HID.HID(hidpath);
	
	ECC508.on("data", function(data)
	{
		console.log('Data returned: '+ data);
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
		else
		{
			console.log('Found Atmel Kit');
		}
	});
	
	ECC508.on("error", function(error)
	{
		console.log('Error returned: '+ error);
	});
	
	var ECCCommand = "_aws:s(00)\n";
	SendECCCommand(ECCCommand);
}

function SendECCCommand(ECCCommand)
{
	var map = Array.prototype.map;
	var command = map.call(ECCCommand, function(x) { return x.charCodeAt(0); });
	//ECC508.write([0, 10]);
	//command = Array.from(ecccommand).map(ASCII);
	//command = ecccommand.split('').map();
	console.log("command " + ECCCommand);
	console.log("command " + command);
	ECC508.write(command);
}

function registerSigner()
{
	// Check that a signer is attached
		// If not attached, then prompt the user to insert
	// Get the AWS registration code
	getRegistrationCode();
	// Initialize the Signer. Send the regCode.
}


function prepareThing()
{
	// Check that a signer is attached
		// If not attached, then prompt the user to insert
	// Get the AWS registration code
	// Send the 
}

function getRegistrationCode()
{
	// Call to AWS SDK to get the registration code
	regCode = AWS.
}



