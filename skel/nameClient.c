

/*
*  C Implementation: nameClient
*
* Description: 
*
*
* Author: MCarmen de Toro <mc@mc>, (C) 2015
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "nameClient.h"

/**
 * Function that sets the field addr->sin_addr.s_addr from a host name 
 * address.
 * @param addr struct where to set the address.
 * @param host the host name to be converted
 * @return -1 if there has been a problem during the conversion process.
 */
int setaddrbyname(struct sockaddr_in *addr, char *host)
{
  struct addrinfo hints, *res;
	int status;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; 
 
  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return -1;
  }
  
  addr->sin_addr.s_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
  
  freeaddrinfo(res);
    
  return 0;  
}

/**
 * Function that gets the dns_file name and port options from the program 
 * execution.
 * @param argc the number of execution parameters
 * @param argv the execution parameters
 * @param reference parameter to set the host name.
 * @param reference parameter to set the port. If no port is specified 
 * the DEFAULT_PORT is returned.
 */
int getProgramOptions(int argc, char* argv[], char *host, int *_port)
{
  int param;
  *_port = DEFAULT_PORT;

  // We process the application execution parameters.
	while((param = getopt(argc, argv, "h:p:")) != -1){
		switch((char) param){		
			case 'h':
				strcpy(host, optarg);				
				break;
			case 'p':
				// Donat que hem inicialitzat amb valor DEFAULT_PORT (veure common.h) 
				// la variable port, aquest codi nomes canvia el valor de port en cas
				// que haguem especificat un port diferent amb la opcio -p
				*_port = atoi(optarg);
				break;				
			default:
				printf("Parametre %c desconegut\n\n", (char) param);
				return -1;
		}
	}
	
	return 0;
}

/**
 * Shows the menu options. 
 */
void printa_menu()
{
		// Mostrem un menu perque l'usuari pugui triar quina opcio fer

		printf("\nAplicatiu per la gestió d'un DNS Server\n");
		printf("  0. Hola mon!\n");
		printf("  1. Llistat dominis\n");
		printf("  2. Consulta domini\n");
		printf("  3. Alta Ip\n");
		printf("  4. Alta Ips\n");
		printf("  5. Modificacio Ip\n");
		printf("  6. Baixa Ip\n");
		printf("  7. Baixa Domini\n");
		printf("  8. Sortir\n\n");
		printf("Escolliu una opcio: ");
}

/**
 * Function that sends a list request receives the list and displays it.
 * @param s The communications socket. 
 */


/** 
 * Function that process the menu option set by the user by calling 
 * the function related to the menu option.
 * @param s The communications socket
 * @param option the menu option specified by the user.
 */
void process_menu_option(int s, int option)
{		  
  switch(option){
    // Opció HELLO
    case MENU_OP_HELLO:
      	process_HELLO_operation(s);
      break;
    case MENU_OP_LIST:
      process_list_operation(s);
      break;
		case MENU_OP_DOMAIN_RQ:
			process_domain(s);
			break;
		case MENU_OP_ADD_DOMAIN_IP:
			process_ADD_DOMAIN_operation(s);
			break;
		case MENU_OP_ADD_DOMAIN_IPS:
			process_ADD_DOMAIN_operation(s);
			break;
		case MENU_OP_CHANGE:
			process_CHANGE_DOMAIN_operation(s);
			break;
		case MENU_OP_DELETE_DOMAIN :
			process_DELETE_DOMAIN_operation(s);
			break;
    case MENU_OP_FINISH:
			sendOpCodeMSG(s,MSG_FINISH);
			exit(0);
      break;
                
    default:
          printf("Invalid menu option\n");
  		}
}

void process_HELLO_operation(int sock){

	printf("Processing operation 0: Hello World\n");

	char buffer[MAX_BUFF_SIZE];

	sendOpCodeMSG(sock, MSG_HELLO_RQ);

	memset(buffer, '\0',sizeof(buffer));

	recv(sock,buffer, sizeof(buffer), 0 );

	printf("message: %s\n", buffer +sizeof(short));

}

void process_list_operation(int sock)
{
	printf("Processing operation 1: List of domains\n");
  char buffer[DNS_TABLE_MAX_SIZE];
  int msg_size;

  sendOpCodeMSG(sock, MSG_LIST_RQ);

  memset(buffer, '\0', sizeof(buffer));

	msg_size = recv(sock, buffer, sizeof(buffer), 0);

	printf("DNS TABLE: \n");

  printDNSTableFromAnArrayOfBytes(buffer+sizeof(short), msg_size-sizeof(short));
}

void process_domain(int sock) 
{
	int offset=0;
	int msg_size;
	char *domainName;
	char userInput[MAX_BUFF_SIZE];
	char buffer[MAX_BUFF_SIZE];
	unsigned short code;

	memset(buffer, '\0',sizeof(buffer));

	printf("Domain name to search: ");
	scanf("%s", userInput);

	strcpy(domainName, userInput);

	stshort(MSG_DOMAIN_RQ, buffer);

	offset+=sizeof(short);

	strcpy(buffer + offset, domainName);

	offset+=strlen(domainName);
	offset+=1;

	send(sock, buffer, offset, 0);

	// RECEIVE AND PROCESS REPLY

	memset(buffer, '\0', sizeof(buffer));

	msg_size = recv(sock, buffer, sizeof(buffer), 0);

	code = ldshort(buffer);

	if (code == 12) {
		printf("DOMAIN DOESN'T EXIST\n");
	} else {

		int numDomains = msg_size/sizeof(struct in_addr);

		printf("%d IPs received\n", (msg_size-sizeof(short))/sizeof(struct in_addr));

		struct in_addr add;

		offset = sizeof(short);

		for (int i = 0; i<numDomains; i++) {
			add = ldaddr(buffer+offset);
			printf("ip #%d: %s\n", i+1, inet_ntoa(add));
			offset+=sizeof(struct in_addr);
		}
	}
}

void process_ADD_DOMAIN_operation(int sock) {

	char buffer[MAX_BUFF_SIZE];

	char newDomain[MAX_HOST_SIZE];

	int offset=0;

	int numIP=0;

	char userIn[MAX_HOST_SIZE];

	unsigned short opCode;

	memset(buffer, '\0', sizeof(buffer));

	stshort(MSG_ADD_DOMAIN, buffer);

	offset+=sizeof(short);


	printf("Enter domain name: \n");
	scanf("%s", newDomain);

	strcpy(buffer+offset, newDomain);

	offset += strlen(newDomain);
	offset += 1;

	printf("number of IP addresses to add to this domain:\n");
	scanf("%s", userIn);

	sscanf(userIn, "%d", &numIP);

	printf("Ok, let's add %d IP addresses.\n", numIP);

	
	char value[50];

	struct in_addr add;

	
	printf("Enter IP addresses\n");

	for (int i=0; i<numIP; i++) {
		scanf("%s", value);
		inet_aton(value, &add);
	
		staddr(add, buffer+offset);
		printf("IP ADDED: %s with offset of : %d\n", value, offset);
		offset+=sizeof(struct in_addr);
	}



	add = ldaddr(buffer+sizeof(short)+strlen(newDomain)+1);
	
	printf("TEST -> FIRST IP INSERTED: %s\n", inet_ntoa(add));

	//sendOpCodeMSG(sock, MSG_ADD_DOMAIN);
	
	send(sock, buffer, offset, 0);

	recv(sock,buffer, sizeof(buffer), 0);	

	opCode = ldshort(buffer);

	if (opCode == 12) {
		printf("DOMAIN ALREADY EXISTS\n");
	} else {
		printf("DOMAIN ADDED");
	}

}

void process_CHANGE_DOMAIN_operation(int sock) {

	struct in_addr addrOld, addrNew;
	char buffer[MAX_BUFF_SIZE];
	char oldIP[50];
	char newIP[50];
	char domainName[MAX_BUFF_SIZE];
	unsigned short opCode;

	int offset = 0;

	stshort(MSG_CHANGE_DOMAIN, buffer);

	printf("Enter domain name: \n");
	scanf("%s", domainName);


	offset+=sizeof(short);

	strcpy(buffer+offset, domainName);

	offset+=strlen(domainName);
	offset+=1;

	struct in_addr add, add2;

	printf("Enter old IP: ");
	scanf("%s", oldIP);
	inet_aton(oldIP, &add);

	

	printf("Enter new IP: ");
	scanf("%s", newIP);

	inet_aton(newIP, &add2);

	addrOld = ldaddr(buffer+offset);

	

	offset+=sizeof(sizeof(add));

	staddr(add, buffer+2+strlen(domainName) + 1);
	
	staddr(add2, buffer+2+strlen(domainName)+ 1 + sizeof(add));

	send(sock, buffer, offset, 0);

	recv(sock,buffer, sizeof(buffer), 0);	

	opCode = ldshort(buffer);

	if (opCode == 12) {
		printf("DOMAIN OR IP DOESN'T EXIST\n");
	} else {
		printf("DOMAIN UPDATED");
	}
}

void process_DELETE_DOMAIN_operation(int sock) {
	char buffer[MAX_BUFF_SIZE];
	int offset = 0;
	char domainName[MAX_BUFF_SIZE];
	unsigned short opCode;

	stshort(MSG_DEL_DOMAIN, buffer);
	offset+=sizeof(short);

	printf("Enter domain name to delete: \n");
	scanf("%s", domainName);

	strcpy(buffer+offset, domainName);
	offset+=strlen(domainName);
	offset+=1;

	send(sock, buffer, offset, 0);

	recv(sock,buffer, sizeof(buffer), 0);	

	opCode = ldshort(buffer);

	if (opCode == 12) {
		printf("DOMAIN DOESN'T EXIST\n");
	} else {
		printf("DOMAIN DELETED");
	}
}


int main(int argc, char *argv[])
{
	int port; // variable per al port inicialitzada al valor DEFAULT_PORT (veure common.h)
	char host[MAX_HOST_SIZE]; // variable per copiar el nom del host des de l'optarg
	int option = 0; // variable de control del menu d'opcions
	int ctrl_options;

  
	

  ctrl_options = getProgramOptions(argc, argv, host, &port);

	// Comprovem que s'hagi introduit un host. En cas contrari, terminem l'execucio de
	// l'aplicatiu	
	if(ctrl_options<0){
		perror("No s'ha especificat el nom del servidor\n\n");
		return -1;
	}
 	
	int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in client_addr;

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	setaddrbyname(&client_addr, host);
	//short request = 1;


	if (connect(s,(struct sockaddr *)&client_addr, sizeof(client_addr))!= 0 ){
		printf("Failed\n");
		exit(0);
	}

	else {
		printf("Connected to server\n");
	}



while(1){

  do{
      printa_menu();
		  // getting the user input.
		  scanf("%d",&option);
		  printf("\n\n"); 
			
		  process_menu_option(s, option)                  ;

	  }while(option != MENU_OP_FINISH); //end while(opcio)
  	close(s);
}
  return 0;
}

