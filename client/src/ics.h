/*
 * ICS Client Header File
 *
 * Autor: Bartlomiej Partyka, Michal Urbanski
 *
 * Data utworzenia 10/04/2020
 */

#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <pthread.h>


#define BUF_SIZE 			1024
#define CH_LEN 				3	

#define CL_CONNECTION_REQ		"01"
#define SRV_CHALLENGE_REQ		"02"
#define CL_CHALLENGE_RESP		"03"
#define SRV_CHALLENGE_ACC		"04"
#define CL_SSID_REQ			"05"
#define SRV_NEW_SSID			"06"
#define SRV_SSID_ACC			"07"
#define CL_NAME				"08"
#define SRV_NAME_ACC			"09"

#define CL_END_REQ			"11"
#define SRV_END_ACC			"12"

#define CL_UPLOAD_REQ			"21"
#define SRV_UPLOAD_ACC			"22"
#define CL_UPLOAD_SEG_SIZE		"23"
#define SRV_UPLOAD_SEG_NUM		"24"
#define CL_UP_PAYLOAD			"25"
#define SRV_UP_PAYLOAD_ACC		"26"
#define SRV_UP_COMPLETE			"27"
#define SRV_RECEIVER_INFO		"28"
#define CL_RCVR_INFO_ACC		"29"

#define SRV_DOWNLOAD_REQ		"31"
#define CL_DOWNLOAD_RESP		"32"
#define SRV_DOWNLOAD_SEG_NUM		"33"
#define CL_DOWNLOAD_READY		"34"
#define SRV_DOWN_PAYLOAD		"35"
#define CL_DOWN_ACC			"36"
#define CL_DOWN_COMPLETE		"37"

#define CL_LIST_REQ			"41"
#define SRV_LIST_RESP			"42"
#define CL_LIST_ACC			"43"

#define ERROR				"90"
/*
 * Klasa ics_server zarzadza polaczeniem z serwerem ics. Klient moze utrzymywac polaczenie tylko z jednym serwerem na raz.
 */
class ics_server 
{
	int sock;
	std::string buf;
	std::string msg; 
	struct sockaddr_in6 server, client;
	const std::string dirpath = ".ics/";	
	std::string name, addr;
	int port;	
	const std::string semi = ";";
	pthread_mutex_t mutex;
	pthread_t receiver;

	/*
	 * Funkcja obslugujaca odbieranie komunikatow
	 */

	int ics_recv(int len, std::string flag);

	/*
	 * Funkcja obslugujaca odbieranie komunikatow podczas wysylania plikow
	 */

	int ics_sfile_recv(std::string flag, char* fbuffer, size_t sendsize);
	
	/*
	 * Funkcja obslugujaca odbieranie komunikatow podczas odbierania plikow
	 */

	int ics_rfile_recv(char *buffer, size_t size, int* received, int segments);

	/*
	 * Funkcja zajmujaca sie autoryzacja oraz polaczeniem z serwerem.
	 */
	int ics_handshake();


	std::string ics_auth(std::string password, std::string challenge);	
	
public:
	ics_server();

	/*
	 * Funkcja obslugujaca pierwsze polaczenie.
	 */

	int ics_connect();

	/*
	 * Zebranie danych do polaczenia
	 */
	int ics_getinfo(std::string a, int p);

	/*
	 * Wyswietlanie listy klientow na serwerze
	 */

	std::string ics_clist();

	/*
	 * Zakonczenie sparowania klient-serwer. Mozliwe jest tylo za zgoda uzytkownika, zwykle rozlaczenie nie zakonczy sesji klienta na serwerze.
	 */

	int ics_disconnect();

	/*
	 * Wysylanie pliku
	 */

	int ics_send(std::string user, std::string path_to_file, int blocksize);

	/*
	 * Odbieranie pliku
	 */

	int ics_recv_file();

	/*
	 * Funkcja dla watku nasluchujacego
	 */

	void* ics_listen();

	static void* ics_listen_helper(void* context);

	void ics_setname(std::string nm);

	std::string ics_getname();

};

/*
 * Klasa zarzadzajaca danymi wejsciowymi klienta
 */

class ics_input_handler
{
	std::string command, args;
	bool set, connected, first_start;
	ics_server* sr;

	/*
	 * Wywolanie odpowiedniej metody
	 */

	int execute();

	/*
	 * User help
	 */

	void print_help();
public:

	ics_input_handler(ics_server* serv);

	int get_command();

};
