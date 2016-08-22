/** 
This node.js Lambda function code creates and attaches an IoT policy to the certificate
registered. It also activates the certificate. The Lambda function is attached as the rules engine action to
the registration topic aws/events/certificates/registered/<caCertificateID>
**/

var AWS = require('aws-sdk');
var region;
var iot;
var accountId;
var certificateARN;
var certificateId;
var thingPolicy;
const awsPolicyName = 'thingPolicy';

// Delay time tracking
var eventTime;

exports.handler = function(event, context, callback)
{
	// Step 1: Create the policy. 
	// Step 2: Attach the policy to the certificate
	// Step 3: Activate the certificate. 
	//			Optionally, you can have your custom Certificate Revocation List (CRL) check logic here and
	//			ACTIVATE the certificate only if it is not in the CRL .Revoke the certificate if it is in the CRL

    // Capture the event time & delay for Lambda execution
    var currentTime = (new Date()).getTime();
    eventTime = event.timestamp;
    var eventDelay = currentTime - eventTime;
    console.log("Lambda event delay: " + eventDelay);

	// Replace it with the AWS region the lambda will be running in
	region = "us-west-2";
	
	// Get the AWS account ID
	accountId = event.awsAccountId.toString().trim();

    // Create the Iot object
	iot = new AWS.Iot({'region': region, apiVersion: '2015-05-28'});
	certificateId = event.certificateId.toString().trim();

    // Construct the ARN for the Thing certificate
	certificateARN = `arn:aws:iot:${region}:${accountId}:cert/${certificateId}`;
	
	// Create and attach the thingPolicy
	awsCreatePolicy();
};

function awsCreatePolicy()
{
	// Step 1: Create the policy
    // Policy definition
    var policy = {
        "Version": "2012-10-17",
        "Statement":
        [
            /* Connect */
            {
                "Effect": "Allow",
                "Action":
                [
                    "iot:Connect"
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}`
            },
            /* Publish, Receive */
            {
                "Effect": "Allow",
                "Action":
                [
                    "iot:Publish",
                    "iot:Receive"
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}`
            },
            /* Subscribe */
            {
                "Effect": "Allow",
                "Action":
                [
                    "iot:Subscribe",
                ],
                "Resource": `arn:aws:iot:${region}:${accountId}`
            }
        ]
    };
    // Create the policy
    iot.createPolicy(
    {
        policyDocument: JSON.stringify(policy),
        policyName: awsPolicyName 
    }, (err, data) =>
    {
        // Log the delay for the createPolicy() callback
        var currentTime = (new Date()).getTime();
        var callbackDelay = currentTime - eventTime;
        console.log("awsCreatePolicy() Delay: " + callbackDelay);
        
        // Ignore if the policy already exists
        if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException'))
        {
            console.log(err);
            return;
        }
        // Set the thingPolicy to the return data
        thingPolicy = data;

		// Step 2: Attach the policy to the certificate
		awsAttachPolicy();
    });
}

function awsAttachPolicy()
{
	// Step 2: Attach the policy to the certificate
	// Attach policy to certificate
	iot.attachPrincipalPolicy(
	{
		policyName: awsPolicyName,
		principal: certificateARN
	}, (err, data) =>
	{
        // Log the delay for the attachPrincipalPolicy() callback
        var currentTime = (new Date()).getTime();
        var callbackDelay = currentTime - eventTime;
        console.log("awsAttachPolicy() Delay: " + callbackDelay);

		// Ignore if the policy is already attached
		if (err && (!err.code || err.code !== 'ResourceAlreadyExistsException'))
		{
			console.log("Failed to attach Policy to \"Thing\" certificate\n" + err);
			return;
		}
		// We've attached the policy, now activate the certificate
		awsActivateThing();
	});
}

function awsActivateThing()
{
    //Step 3: Activate the certificate. 
    //			Optionally, you can have your custom Certificate Revocation List (CRL) check logic here and
    //			ACTIVATE the certificate only if it is not in the CRL .Revoke the certificate if it is in the CRL
    iot.updateCertificate(
    {
        certificateId: certificateId,
        newStatus: 'ACTIVE'
    }, (err, data) =>
    {
        // Log the delay for the updateCertificate() (activate) callback
        var currentTime = (new Date()).getTime();
        var callbackDelay = currentTime - eventTime;
        console.log("awsActivateThing() Delay: " + callbackDelay);

        if (err)
        {
            console.log("Thing activation failed."); 
        }
        else
        {
            console.log("Thing activated successfully.");   
        }
    });
}