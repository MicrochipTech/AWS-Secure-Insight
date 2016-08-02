### AWS-IoT-Zero-Touch
#### AWS JIT and Bring your Own Certificate Demo.

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

#### Application Requirements 
To Succesfully run the __AWS IoT Zero Touch Demo__ you will need the following GUI
- [Microchip's AWS IoT Zero Touch GUI](https://github.com/MicrochipTech/AWS-IoT-Zero-Touch/tree/master/software/AWS-IoT-Zero-Touch-GUI)
 
#### Required Tools
To Succesfully run the __AWS IoT Zero Touch Demo__ you will need the following tools
- [Microchip's AWS IoT Zero Touch Development Platform](http://preview.atmel.com/tools/AT88CKECC-AWS-XSTK.aspx)
- [Microchip's AWS IoT Zero Touch Development Platform FW](https://github.com/MicrochipTech/AWS-IoT-Zero-Touch/tree/master/software/AWS-IoT-Zero-Touch-FW)

---

__For more information go to [Wiki](https://github.com/MicrochipTech/AWS-IoT-Zero-Touch/wiki)__
