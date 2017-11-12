# FITSec - The ETSI TS 103 097 implementation

## Overview ##

The library provides the security envelop to be used for the Intelligent Transport Systems G5 communication based on
[ETSI EN 302 636-4-1](http://www.etsi.org/deliver/etsi_en/302600_302699/3026360401/01.02.01_60/en_3026360401v010201p.pdf).
The library is fully conformed to [ETSI TS 103 097 v1.2.1](http://www.etsi.org/deliver/etsi_ts/103000_103099/103097/01.02.01_60/ts_103097v010201p.pdf)
with optional support of non-published v1.2.5.

The library is written in plain C in cross-platform manner and has been compiled and tested in Linux(gcc) and Windows(mingw32,cygwin and Visual C 13) environments.

It implements the plugin interface to use various crypto engines. For the moment, only the [OpenSSL](https://www.openssl.org/) engine is implemented, but support of HSM (Hardware Security Module) is in the plan.
Please see the _fitsec_openssl.c_ for the example of plugin implementation.

## User API ##
The main API is defined in the [_fitsec.h_](https://github.com/DanyaFilatov/FITSec/blob/master/libfitsec/fitsec.h) header.
I really invite you to have a look into this file to understand the meaning of various parameters.

### Initialization ###
1. First of all, the instance of the engine must be created using the __*FitSec_New*__.
This function takes a configuration strucutre as a parameter.
This configuration strucure can be initialized using the __*FitSecConfig_InitDefault*__.

2. Install all necessary certificates using function __*FitSec_InstallCertificate*__.
All certificates installed by this function will be considered as trusted. Any types of certificates can be installed.
Authorization tickets shall be followed by the correspondent private keys.
(Note: This behavior will be changed in order to support HSM).

### Outgoing Messages ###
Processing of outgoing messages is splitted to two stages:
- preparation of the message header
- signing and/or encryption of the message

In order to optimize the memory manipulatoin efforts, all operations are done directly in the buffer provided by the facility layer.
This buffer can be passed later to the transport layer. The size of the buffer shall be well enough to contain all security headers,
certificates and the payload of the message.
The GeoNetworking Security Header takes place between Basic Header and Common Header elements in GeoNetworking message strucure.
So, to send a secured GN message, facylity layer shall perform following actions:
- prepare Basic GN Header
- call __*FitSec_PrepareMessage*__ to prepare Security header
- fill the payload buffer with the payload data, starting from Common GN Header, update the payload size field in message information structure.
  (Note: Please don't spend too much time in this stage because it can violate the CAM signing rules.)
- call __*FitSec_SignMessage*__ to encrypt and/or sign the message.

Please see the [_fitsec.h_](libfitsec/fitsec.h) for function descriptions.

#### Preparation of message header ####
As soon as GeoNetworking layer prepared the Basic Header, it can call __*FitSec_PrepareMessage*__ to create the Security Header element and to put it into outgoing buffer.
This function must be called with following parameters:
- buffer and maximum buffer length. This parameter shall point to the outgoing memory buffer right after the Basic Header element.
- message information strucuture, containing:
  - payload type (signed, encrypted, etc.)
  - current position (not needed for CAM)
  - the timestamp, when the last GPS fix has been occured.
  - Application ID and SSP bits, describing the content of the message. The ITS AID list can be found on [ISO TS17419 V2016-02-09: "ITS-AID_AssignedNumbers")(http://standards.iso.org/iso/ts/17419/TS17419%20Assigned%20Numbers/TS17419_ITS-AID_AssignedNumbers.pdf)

The function creates the ITS Security Header in the provided buffer and fill the message information structure with:
- payload offset - the pointer to the memory buffer where facility layer can put the payload
- max payload size - the maximum size of payload buffer. Facility layer shall put the actual payload size there
- certificate to be used - it takes into account provided ITS AID, SSP, timestamp and location
- signer type (certificate, digest, chain).

The function can override the payload type if it is required by the security profile. The CAM and DENM security profile requires the payload to be signed but not encrypted.

The data in the message information strucure will be used on the next stage, so please keep it unchanged, excepting of payload size.
  
The function returns the offset in the buffer to copy the payload or -1 if case of some error. The error id and error description are provided in the message information strucure.

Please see the [_fitsec.h_](libfitsec/fitsec.h) for function descriptions.

#### Preparing of payload ####
The GeoNetworking layer needs to create the Common Header and, optionally the Extended Header elements, and to put the facility layer payload into the outgoing buffer.

The total length of these elements shall be set in the payload size field in the message information strucure.

#### Signing and/or encrypting of the message ####

When payload creation is well done, the message could be encrypted and/or signed, depending of the payload type.
The GN layer must call __*FitSec_SignMessage*__ to perform these tasks.
This functions takes same parameters as the __*FitSec_PrepareMessage*__.

_Note: encryption is not implemented yet. See Limitations section_

The function returns the full size of secured packet or -1 in case of error. The error id and error description are provided in the message information strucure.

### Incoming Messages ###

The function __*FitSec_Verify*__ can be used to verify the incoming message signature.

It will fill the content of the message information structure:
- payload, payload size and payload type
- position and generation time
- signer info type and signing certificate (if any)
- Application ID and SSP bits of certificate.

Function verifies the message signature, returns true or false and fill the error id and the error description elements in the message information structure.

It is up to facility layer to check the conformance of the incoming message with correspondent SSP bitfield and to take or doesn't take it into account.

The function __*FitSec_Decrypt*__ will be implemented soon to be able to use encrypted messages.

## Limitations ##
- The library doesn't support encryption for the moment.

## Author ##
The library was created in 2015-2016 by Denis Filatov (danya.filatov()gmail.com) in order to validate the ETSI's ITS security test suite. The library is free for non-commercial and not-for-profit usage, otherwise please contact the author.
