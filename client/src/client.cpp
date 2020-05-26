/*
 * ICS Client Driver File (Main Function)
 *
 * Autor: Bartlomiej Partyka
 *
 * Data utworzenia: 10/05/2020
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ics.h"

ics_server ics_serv;
ics_input_handler handler(&ics_serv);
int main(){
	for(;;){
		int b = handler.get_command();
		if(b == 45456)
			break;
	}

	return 0;
}
