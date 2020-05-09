#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

#ifndef ICS_H
#define ICS_H
/*
 * Header file for the ICS API.
 * Bartlomiej Partyka
*/

#define BUF_SIZE 		1024
#define CH_LEN 			3	

#define CL_CONNECTION_REQ	"01"
#define SRV_CHALLENGE_REQ	"02"
#define CL_CHALLENGE_RESP	"03"
#define SRV_CHALLENGE_ACC	"04"
#define CL_SSID_REQ		"05"
#define SRV_NEW_SSID		"06"
#define SRV_SSID_ACC		"07"
#define CL_NAME			"08"
#define SRV_NAME_ACC		"09"

#define CL_END_REQ		"11"
#define SRV_END_ACC		"12"

#define CL_UPLOAD_REQ		"21"
#define SRV_UPLOAD_ACC		"22"
#define CL_UPLOAD_SEG_SIZE	"23"
#define SRV_UPLOAD_SEG_NUM	"24"
#define CL_UP_PAYLOAD		"25"
#define SRV_UP_PAYLOAD_ACC	"26"
#define SRV_UP_COMPLETE		"27"
#define SRV_RECEIVER_INFO	"28"
#define CL_RCVR_INFO_ACC	"29"

#define SRV_DOWNLOAD_REQ	"31"
#define CL_DOWNLOAD_RESP	"32"
#define SRV_DOWNLOAD_SEG_NUM	"33"
#define CL_DOWNLOAD_READY	"34"
#define SRV_DOWN_PAYLOAD	"35"
#define CL_DOWN_ACC		"36"
#define CL_DOWN_COMPLETE	"37"

#define CL_LIST_REQ		"41"
#define SRV_LIST_RESP		"42"
#define CL_LIST_ACC		"43"

#define ERROR			"90"
/*
 * Klasa ics_server zarzadza polaczeniem z serwerem ics. Klient moze utrzymywac polaczenie tylko z jednym serwerem na raz.
 */
class ics_server 
{
	int sock;
	char buf[BUF_SIZE];
	char* msg; 
	struct sockaddr_in6 server;
	char* dirpath = ".ics/";	
	
	/*
	 * Funkcja obslugujaca odbieranie komunikatow
	 */

	int ics_recv(int len, char* flag, int tries);

	/*
	 * Funkcja zajmujaca sie autoryzacja oraz polaczeniem z serwerem.
	 */
	int ics_handshake();


	char* ics_auth(char* password, char* challenge);	
	
public:
	ics_server();
	/*
	 * Funkcja obslugujaca pierwsze polaczenie.
	 */
	int ics_connect(char* address, int port);
	/*
	 * Wyswietlanie listy klientow na serwerze
	 */
	char* ics_clist();
	/*
	 * Zakonczenie sparowania klient-serwer. Mozliwe jest tylo za zgoda uzytkownika, zwykle rozlaczenie nie zakonczy sesji klienta na serwerze.
	 */
	int ics_disconnect();


};

#endif
