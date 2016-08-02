### AWS-IoT-Zero-Touch
#### AWS JIT and Bring your Own Certificate Demo User Guide.

---

### Introduction
[AWS IoT](http://aws.amazon.com/iot/) is a service that will allow internet of things (IoT) devices to be easily and securely connected to Amazon Web Services (AWS).  The __AWS-IoT-Zero-Touch__ desktop application has been designed to work with this service via the shadow registers in AWS IoT to control Microchip's IoT demo.

Security for AWS IoT is provided by Microchip's __ATECC508A CryptoAuthentication__ devices. ATECC508A provides an easy solution to bring __Hardware Security__ by stablishing itself as a Secure Hardware Trust Anchor for the IoT End Node. With the capability of managing Private Keys Securely (Private Keys never leave ECC508A), the use of ECDSA for Signing and Verifying and ECDH to create a pre-master key, ECC508A stands as a strong solution for cloud computing and IoT based systems.

Currently this demo is designed to work with the following demos:
- [Microchip AWS IoT Zero Touch Powered By AWS](https://github.com/MicrochipTech/aws-iot-firmware-pic32mz)

For more information on the current Microchip IoT demos please go to [Microchip's IoT Page](http://www.microchip.com/iot).

---

### Required Tools and Applications
#### OS Requirements
We have build and tested the IoT Zero Touch Application on the following OS's:
- __OS X:__ 10.11.x
- __Linux:__ Debian Based - Ubuntu 14.04LTS
- __Windows:__ Windows 7

#### AWS Tools and Applications
- You will need an AWS account to use AWS IoT.

  - For more information how to setup an AWS Account please go to [AWS](http://aws.amazon.com) main page
  - Please check with your IT or IS department on your company's policies on using cloud computing, or to see if you have a corporate AWS account that should be used.

- To use the AWS IoT services you will need to make use of _AWS Command Line Interface_ (awscli) tool.  For more information on how to install, configure, and use the awscli tool please go to [AWS Command Line Interface User Guide](http://docs.aws.amazon.com/cli/latest/userguide/cli-chap-welcome.html).

> NOTE: If you are a **Microchip Employee** we have a corporate AWS account that must be used.  To setup your AWS IoT device please go to https://setup.iot.microchip.com for more information

---
### Setting Up the AWS Command Line Interface
#### Linux, Unix, and Windows Installations
Follow the instructions in the [Getting Set Up with the AWS Command Line Interface](http://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-set-up.html) for installations for your operating system.

#### Mac OS X Installation
This assumes that you have not installed previous version of Python, awscli, or openssl; if you have you will need to uninstall those versions.  You will need to use a version of openssl 1.0.0 or later to connect to AWS IoT services as TLS 1.1 is required.  To do this follow the these steps:

1. Use [Homebrew](http://brew.sh/) to install the latest version of openssl with the following commands in a terminal window.  You will need to force the links so that Python will use the right version of openssl.

  ```
  brew update
  brew install openssl
  brew link --force openssl
  ```
- Reinstall Python to use the latest openssl:

  ```
  brew install python --with-brewed-openssl
  ```
- To verify that python was updated run the following command and make sure the version os greater than 1.0.2d

  ```
  python -c 'import ssl; print ssl.OPENSSL_VERSION'
  ```
- Install/upgrade the AWS CLI:

  ```
  pip install --upgrade awscli
  ```
- To verify the version of the AWS CLI tool installed run the following command and look for version 1.10.x or greater:

  ```
  aws --version
  ```

---
### Setting Up the AWS IoT Service for the Insight on Things Application

To setup your AWS IoT service to work with the Insight on Things Desktop Application you will need to work with the awscli that you installed in the previous section.

1. Using the awscli you will need to find the endpoint that will be used for your AWS IoT account.  To do this run the following command in your terminal/command window.

  ```
  aws iot describe-endpoint
  ```
- Create a group that will hold the policy allowing access to the AWS IoT shadow registers

  ```
  aws iam create-group --group-name iotDemo
  ```
- Create the following policy that will allow your Insight on Things Desktop Application to connect to the AWS IoT service and save the file to your computer
  > NOTE: Make sure you replace `<AWS IoT Region>` and `<AWS Account Number>` with the AWS-IoT region and your AWS account number

  ```
  }
    "Version": "2012-10-17",
    "Statement": [{
      "Effect": "Allow",
      "Action": [
        "iot:GetThingShadow",
        "iot:UpdateThingShadow"
        ],
      "Resource": [
        "arn:aws:iot:<AWS IoT Region>:<AWS Account Number>:thing/*"
      ]
    }]
  }
  ```
- Load the policy to AWS with the following command:

  ```
  aws iam create-policy --policy-name iotDemoPolicy --policy-document file://<path-to-your-policy-document>
  ```
- You will need to attached this policy to a group to allow the users the right to access the AWS IoT shadow registers with the following command:
  > NOTE: Make sure you replace `<AWS Account Number>` with your AWS account number

  ```
  aws iam attach-group-policy --policy-arn arn:aws:iam:<AWS Account Number>:aws:policy/iotDemoPolicy --group-name iotDemo
  ```
- Next you will need to create a user for the Insight on Things Desktop Application

  ```
  aws iam create-user --user-name InsightOnThings
  ```
- Add the user to the iotDemo group

  ```
  aws iam add-user-to-group --user-name InsightOnThings --group-name iotDemo
  ```
- To allow the Insight on Things Desktop Application to access AWS you will need to download the required key and tokens for this user
  > NOTE: The output of this command is the only time that you will be able to see this information.  Store in a secure location. If  it  is  lost,  it cannot be recovered, and you must create a new access key.

  ```
  aws iam create-access-key --user-name InsightOnThings
  ```

---
### Insight On Things Desktop Application Installation
You will need to [download the latest version](https://github.com/MicrochipTech/aws-iot-insight-on-things-desktop-app/releases/latest) of __Insight on Things__ for your operating system.


##### Mac OS X Installation
1. To install on MAC OS X download the 'Insight.dmg' file and double click on the dmg icon. The installer will open as shown below:

  ![Mac install](documents/images/mac_install.png)
- To install the application drag the Insight app icon to the Applications folder in the install screen.

#### Linux Installation
The Linux application is designed to run on Debian based platforms, the most popular being Ubuntu or Linux Mint.

1. To install the application download the Insight.deb file

2. Double clicking on the .deb file will open up the default installer

3.  Right clicking on the file will allow you to open the package installer

  ![Linux install](documents/images/linux_install.png)
4. The application is located in the /opt/ directory. To run the application open a Terminal window and execute the following command:

  ```
    linux> /opt/insight/Insight
  ```

#### Windows Installation
1. To install on Windows download the 'Insight Setup.exe' file and double click on the .exe to start the installer. This application uses a standard installer shown below:

  ![Windows install](documents/images/windows_install.png)

---
### Running the Demo

1. Start the Insight on Things Desktop Application on your computer.
- When the application starts it will look for a file name `.insight` in the Users home directory.
  - This file contains the information that the app will use to access and authenticate you with AWS IoT
  - If the `.insight` file is not found the application, it will take you to a screen to enter the credentials that you generated in the [Setting Up the AWS IoT Service for the Insight on Things Application](Setting Up the AWS IoT Service for the Insight on Things Application).
  - Press the `Create New Thing` button to save the credentials into that file.
  - Bellow is what the Insight on Things Desktop Application will start with if no credential are found.

    ![No credentials found](documents/images/insight_no_credentials.png)
- If the `.insight` file is found, and has correct credentials in it, the application will default to the data screen and show the current status of the shadow registers.

  ![Credentials found](documents/images/insight_normal_operation.png)
  - By using one of the demos listed in the [Introduction](Introduction) section you can change data on that device and see it on the Insight on Things Desktop Application
  - You can also control the demo by change the status of the LED on this application
- The credentials and the thing that the application is looking at can be changed at any time, by selecting `Thing -> Settings` from the application menu. This will take you to a screen that will load the current credentials and allow them to be updated.

  ![Update Credentials](documents/images/insight_update_credentials.png)
  - Update any credentials and press the `Update Credentials` button to save the new credentials into the `.insight` file.

---
### Troubleshooting
If you are having trouble connecting with AWS IoT and your thing, check to make sure that each of the issues bellow are resolved.

#### Connection Issue
- Check that you have a valid internet connection on your network/access point.
- Ensure that port 443 is open on your network/access point.
- Ensure that your AWS IoT service is setup properly.

#### Credential Issue
- The credentials are case sensitive.

