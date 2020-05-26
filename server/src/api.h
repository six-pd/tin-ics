/*
 * Protocol flags file
 *
 * Autor: Krzysztof Blankiewicz
 *
 * Data utworzenia: 10/04/2020
 */

const int BUF_SIZE = 1024; // trzeba to potem stad zabrac

// nazywam te stale flagami, chociaz nie wiem czy to dobre okreslenie...

const int CL_CONNECTION_REQ		= 1;
const int SRV_CHALLENGE_REQ		= 2;
const int CL_CHALLENGE_RESP		= 3;
const int SRV_CHALLENGE_ACC		= 4;
const int CL_SSID_REQ		    = 5;
const int SRV_NEW_SSID		    = 6;
const int SRV_SSID_ACC		    = 7;
const int CL_NAME		        = 8;
const int SRV_NAME_ACC		    = 9;

const int CL_END_REQ		    = 11;
const int SRV_END_ACC		    = 12;

const int CL_UPLOAD_REQ		    = 21;
const int SRV_UPLOAD_ACC		= 22;
const int CL_UPLOAD_SEG_SIZE	= 23;
const int SRV_UPLOAD_SEG_NUM	= 24;
const int CL_UP_PAYLOAD		    = 25;
const int SRV_UP_PAYLOAD_ACC	= 26;
const int SRV_UP_COMPLETE		= 27;
const int SRV_RECEIVER_INFO		= 28;
const int CL_RCVR_INFO_ACC		= 29;

const int SRV_DOWNLOAD_REQ		= 31;
const int CL_DOWNLOAD_RESP		= 32;
const int SRV_DOWNLOAD_SEG_NUM	= 33;
const int CL_DOWNLOAD_READY		= 34;
const int SRV_DOWN_PAYLOAD		= 35;
const int CL_DOWN_ACC		    = 36;
const int CL_DOWN_COMPLETE		= 37;

const int CL_LIST_REQ           = 41;
const int SRV_LIST_RESP         = 42;
const int CL_LIST_ACC           = 43;

const int ERROR                 = 90;
