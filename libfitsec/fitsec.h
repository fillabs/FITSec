/*********************************************************************
This file is a part of FItsSec project: Implementation of ETSI TS 103 097
Copyright (C) 2015  Denis Filatov (denis.filatov()fillabs.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed under GNU LGPLv3 in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
@license LGPL-3.0+ <http://www.gnu.org/licenses/lgpl-3.0.txt>

In particular cases this program can be distributed under other license
by the simple request to the author.
*********************************************************************/

#ifndef fitsec_h
#define fitsec_h
#ifdef WIN32
#ifdef LIBFITSEC_EXPORTS
#define FITSEC_EXPORT __declspec(dllexport)
#else
#define FITSEC_EXPORT __declspec(dllimport)
#endif
#else
#define FITSEC_EXPORT
#endif

/* configuration */
#define FITSEC_HAVE_OPENSSL
#define FITSEC_AID_CAM 36
#define FITSEC_AID_DENM 37
#define FITSEC_AID_ANY -1

#include "fitsec_crypt.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct FitSec FitSec;
	typedef struct FSCertificate FSCertificate;
	typedef int    FSItsAid;

	typedef enum {
		FSFALSE,
		#define FSFALSE FSFALSE
		FSTRUE
		#define FSTRUE FSTRUE
	}FSBOOL;

	typedef struct FSLocation {
		long           latitude;
		long           longitude;
		unsigned short elevation;
	} FSLocation;

	typedef unsigned long long Time64;
	typedef unsigned long Time32;

	typedef enum  {
		FS_PAYLOAD_UNSECURED,
		FS_PAYLOAD_SIGNED,
		FS_PAYLOAD_ENCRYPTED,
		FS_PAYLOAD_SIGNED_EXTERNAL,
		FS_PAYLOAD_SIGNED_AND_ENCRYPTED,
	} FSPayloadType;

	enum FSEventId {
		FSEvent_ChangeId = 1,
	};
	typedef FSBOOL FitSec_Event_Fn(FitSec * e, void * user, int event, void * params);

	typedef struct FitSecMsgProfile
	{
		FSItsAid      aid;         // set to FITSEC_AID_ANY fro default profile
		int           certPeriod;  // maximum delay in msec between two certificates
		                           // set to -1 to do not send certificates automatically,
			                       // set to  0 to send certificates in each CAM
		unsigned int  flags;       // see FitSecEngineFlags
		FSPayloadType payloadType; // defalt payload type
	}FitSecMsgProfile; /* Unused for the moment */

	/** Configuration flags */
	enum FitSecEngineFlags {
		/** must be set to send requests for unknown certificates */
		FS_USE_CERT_REQUEST = 1,
		
		/** send requested AA certificate only if both AA and AT certificate has been requested
		    from other side and both AA and AT digests are pointed to the currently used certificates.
		    The idea is to sign messages by certificate chain only if both AA and AT has been requested.
		    !This is a non-standard behavior! */
		FS_SEND_AA_CERT_IF_BOTH_REQUESTED = 1,

		/** request both AT and AA certificates if AA certificate needs to be requested.
		    !This is a non-standard behavior! */
		FS_REQUEST_AT_CERT_WITH_AA = 2,
		
		/** do not sign next cam with certificate chain if some message signed with chain containing AA certificate was 
		received after request */
		FS_CANCEL_AA_REQUEST = 4,
	};

	typedef struct FitSecConfig
	{
		const char                * storage;        // path to load and store certificates (Deprecated and not used anymore!)
		int                         hexadecimal;    // certificate data is in hexadecimal form
		unsigned int                version;        // default protocol version (2)
		int                         certPeriod;     // set to -1 to do not send certificates in CAM,
		                                            // set to  0 to send certificates in each CAM
		                                            // set to 1000 (100 msec) to send each second.
		unsigned int                flags;          // see FitSecEngineFlags
		const FSCryptEngineConfig * crypt;          // crypto engine to be used (use openssl by default)
		FitSec_Event_Fn           * cbOnEvent;      // user callback function to be called when some event has been occured (see FSEventId)
		void                      * cbUser;         // user pointer to be passed to the callback function
	} FitSecConfig;

	/** Initialize the config structure with default values */
	FITSEC_EXPORT void  FitSecConfig_InitDefault(FitSecConfig * cfg);

	/** Create and initialize engine */
	FITSEC_EXPORT FitSec *              FitSec_New(const FitSecConfig * config);
	
	/** Install certificate (root, AA, local AT pseudonymes) */
	FITSEC_EXPORT const FSCertificate * FitSec_InstallCertificate(FitSec * e,
		const char * cert, int cert_length,
		const char * vkey, int vkey_length,
		const char * ekey, int ekey_length,
		int * perror
	);

	/** Select certificate to be used as current pseudonym.
	    This function will trigger the Id changing process */
	FITSEC_EXPORT FSBOOL                FitSec_Select(FitSec * e, unsigned int cert_id);

	/** Cleanup all data, forget all foreign certificates, 
	    clean all local certificates if clean_local flag is set */
	FITSEC_EXPORT void                  FitSec_Clean(FitSec * e, int clean_local);
	
	/** Cleanup engine and free all allocated memory */
	FITSEC_EXPORT void                  FitSec_Free(FitSec * e);

	/** Returns the message corresponding to the error value */
	FITSEC_EXPORT const char * FitSec_ErrorMessage(int err);

	typedef struct {
		FSItsAid  aid;
		union{
			struct {
				unsigned char  version;
				unsigned int   flags;
			}ssp;
			char data[32];
		}u;
	} FSItsAidSsp;

	typedef enum {
		FS_SI_SELF,
		FS_SI_DIGEST,
		FS_SI_CERTIFICATE,
		FS_SI_CERTIFICATE_CHAIN,
		FS_SI_OTHER_DIGEST,
		FS_SI_NO_SIGNATURE,
	} FSSignerInfoType;

	typedef struct FSMessageInfo {
		int                   status;         // error value. 
		const char *          errmsg;         // error description
		char *                payload;        
		int                   payloadSize;
		FSPayloadType         payloadType;
		FSLocation            position;
		Time64                generationTime;
		Time32                expirationTime;
		FSSignerInfoType      si_type;
		const FSCertificate * cert;
		FSItsAidSsp           ssp;
		
		// internal data
		void *_ptrs[8];
	} FSMessageInfo;

	/** Sign ITS message.
	 *  @param e       [In]      The engine
	 *  @param m       [In/Out]  Message Info structure
	 *  @param buf     [Out]     Pointer to the buffer to store message header
	 *  @param bufsize [In]      The max size of the buffer
	 *  @return        message size or -1 for error
	 *  Note: All parameters must be identical for both functions.
	 *        Each call to FitSec_PrepareMessage must be followed by FitSec_SignMessage.
	 *
         *  Description of fields in FSMessageInfo:
	 *                 | FitSec_PrepareMessage                      | FitSec_SignMessage
	 *  ---------------|--------------------------------------------|------------------
	 *  payload        | out: returns address to copy payload       | in: the same address as returned from FitSec_PrepareMessage
	 *  payloadSize    | -                                          | in: the size of the payload. This field MUST be initialized between functions call
	 *  payloadType    | in: the type of the payload                | -
	 *  position       | in: current position. Not needed for CAM   | -
	 *  generationTime | in: the timestamp of the last GPS fix      | -
	 *  expirationTime | - unsupported                              | - 
	 *  si_type        | out: signer info of the message            | in: must be the same as returned from FitSec_PrepareMessage
	 *  cert           | out: certificate to be used to sign message| in: the same value as returned from FitSec_PrepareMessage
	 *  ssp            | in: the ssp bits representing content of   | -
	 *                 |     the message. These bits will be used to|
	 *                 |     select proper certificate.             |
	 */
	FITSEC_EXPORT int FitSec_PrepareMessage(FitSec * e, FSMessageInfo* m, char * buf, int bufsize);
	FITSEC_EXPORT int FitSec_SignMessage(FitSec * e, FSMessageInfo* m, char * buf, int bufsize);

	/** Verify ITS message.
         *  @param e       [In]      The engine
         *  @param info    [In/Out]  Message information.
         *  @param msg     [In]      Pointer to the buffer of the message to be verified
         *  @param msgize  [In]      The size of the buffer
         *  @return        FSTRUE for Success or FSFALSE for error. See info->status for error value
         *
         *  Description of fields in FSMessageInfo:
	 *                 | FitSec_Verify
	 *  ---------------|--------------------------------------------
	 *  payload        | out: points to the first byte of payload
	 *  payloadSize    | out: the size of message payload
	 *  payloadType    | out: the type of the payload
	 *  position       | out: the remote position where message has been signed
	 *  generationTime | out: the time when message has been generated
	 *  expirationTime | - unsupported for now
	 *  si_type        | out: signer info of the message
	 *  cert           | out: the AT certificate that has been used to sign message
	 *  ssp            | out: the AID of the message and SSP of the certificate
	 */
	FITSEC_EXPORT FSBOOL FitSec_Verify(FitSec * e, FSMessageInfo * info, const void * msg, int msgsize);
	
#ifdef __cplusplus
}
#endif
#endif
