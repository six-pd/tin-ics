#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ICS_H
#define ICS_H
/*
 * Header file for the ICS API.
 * Bartlomiej Partyka
*/

/*
 * Klasa ics_server zarzadza polaczeniem z serwerem ics. Klient moze utrzymywac polaczenie tylko z jednym serwerem na raz.
 */
class ics_server 
{
	int sock;
	struct hostent *hp;
	struct sockaddr_in6 server;

	/*
	 * Funkcja prywatna zajmujaca sie autoryzacja oraz polaczeniem z serwerem.
	 */
	int ics_handshake();
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
