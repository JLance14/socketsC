/*
*  C Implementation: nameServer
*
* Description: 
*
*
* Author: MCarmen de Toro <mc@mc>, (C) 2015
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "nameServer.h"



/* Reads a line ended with \n from the file pointer.  */
/* Return: a line ended not with an EOL but with a 0 or NULL if the end of the
file is reached */
char *readLine(FILE *file, char *line, int sizeOfLine)
{
  
  int line_length;

  if (fgets(line, sizeOfLine, file) != NULL)
  {
    line_length = strlen(line)-1;
    line[line_length] = 0;    
  } 
  else
  {
    line = NULL;
  }

  return line;
}


/**
 * Creates a DNSEntry variable from the content of a file line and links it 
 * to the DNSTable. 
 * @param line the line from the file to be parsed
 * @param delim the character between tokens.
 */
struct _DNSEntry* buildADNSEntryFromALine(char *line, char *token_delim)
{
  
  char *token;
  struct _IP *ip_struct = malloc(sizeof(struct _IP));
  struct _IP *last_ip_struct;
  struct _DNSEntry* dnsEntry = malloc(sizeof(struct _DNSEntry)); 
  int firstIP = 1;
 

  //getting the domain name
  token = strtok(line, token_delim);
  strcpy(dnsEntry->domainName, token);
  dnsEntry->numberOfIPs = 0;

  //getting the Ip's
  while ((token = strtok(NULL, token_delim)) != NULL)
  {
    ip_struct = malloc(sizeof(struct _IP));
    inet_aton((const char*)token, &(ip_struct->IP));
    ip_struct->nextIP = NULL;
    (dnsEntry->numberOfIPs)++;
    if (firstIP == 1)
    {
      dnsEntry->first_ip = ip_struct;
      last_ip_struct = ip_struct;
      firstIP = 0;
    }
    else
    {
      last_ip_struct->nextIP = ip_struct;
      last_ip_struct = ip_struct;
    }
  }  
    
    return dnsEntry;
}

/* Reads a file with the dns information and loads into a _DNSTable structure.
Each line of the file is a DNS entry. 
RETURNS: the DNS table */
struct _DNSTable* loadDNSTableFromFile(char *fileName)
{
  FILE *file;
  char line[1024];
  struct _DNSEntry *dnsEntry;
  struct _DNSEntry *lastDNSEntry;
  struct _DNSTable *dnsTable = malloc(sizeof(struct _DNSTable)); 
  int firstDNSEntry = 1;

  file = fopen(fileName, "r");
  if (file==NULL)
  {
    perror("Problems opening the file");
    printf("Errno: %d \n", errno);
  }
  else
  {
    //reading the following entries in the file
    while(readLine(file, line, sizeof(line)) != NULL)
    {
      dnsEntry = buildADNSEntryFromALine(line, " ");
      dnsEntry->nextDNSEntry = NULL;
      if (firstDNSEntry == 1)
      {
        dnsTable->first_DNSentry = dnsEntry;
        lastDNSEntry = dnsEntry;
        firstDNSEntry = 0;
      }
      else
      {
        lastDNSEntry->nextDNSEntry = dnsEntry;
        lastDNSEntry = dnsEntry;        
      }  
    } 
      
    
    fclose(file);
  }
  
  return dnsTable;
}


/**
 * Calculates the number of bytes of the DNS table as a byte array format. 
 * It does not  include the message identifier. 
 * @param dnsTable a pointer to the DNSTable in memory.
 */
int getDNSTableSize(struct _DNSTable* dnsTable)
{
  int table_size = 0;
  int numberOfIPs_BYTES_SIZE = sizeof(short);
  
  
  struct _DNSEntry *dnsEntry;

  dnsEntry = dnsTable->first_DNSentry;
  if(dnsEntry != NULL)
  {
    do
    {    
      table_size +=  ( strlen(dnsEntry->domainName) + SPACE_BYTE_SIZE +
        numberOfIPs_BYTES_SIZE + (dnsEntry->numberOfIPs * sizeof (in_addr_t)) );
    }while((dnsEntry=dnsEntry->nextDNSEntry) != NULL);
  }
 

  return table_size; 
}



/*Return a pointer to the last character copied in next_DNSEntry_ptr + 1 */
/**
 * Converts the DNSEntry passed as a parameter into a byte array pointed by 
 * next_DNSEntry_ptr. The representation will be 
 * domain_name\0number_of_ips[4byte_ip]*]. 
 * @param dnsEntry the DNSEntry to be converted to a Byte Array.
 * @param next_DNSEntry_ptr a pointer to Byte Array where to start copying 
 * the DNSEntry. The pointer moves to the end of the ByteArray representation.
 */
void dnsEntryToByteArray(struct _DNSEntry* dnsEntry, char **next_DNSEntry_ptr)
{
  
  struct _IP* pIP;
 
  fflush(stdout);
  
  strcpy(*next_DNSEntry_ptr, dnsEntry->domainName);
  //we leave one 0 between the name and the number of IP's of the domain
  *next_DNSEntry_ptr += (strlen(dnsEntry->domainName) + 1);
  stshort(dnsEntry->numberOfIPs, *next_DNSEntry_ptr);
  *next_DNSEntry_ptr += sizeof(short);
  if((pIP = dnsEntry->first_ip) != NULL)
  {    
    do    
    { 
      staddr(pIP->IP, *next_DNSEntry_ptr);      
      *next_DNSEntry_ptr += sizeof(in_addr_t);
    }while((pIP = pIP->nextIP) != NULL);
  }
 
}


/*Dumps the dnstable into a byte array*/
/*@Return a pointer to the byte array representing the DNS table */
/*@param dnsTable the table to be serialized into an array of byes */
/*@param _tableSize reference parameter that will be filled with the table size*/
char *dnsTableToByteArray(struct _DNSTable* dnsTable, int *_tableSize)
{ 
  int tableSize = getDNSTableSize(dnsTable);
  *_tableSize = tableSize;

  char *dns_as_byteArray = malloc(tableSize);
  char *next_dns_entry_in_the_dns_byteArray_ptr = dns_as_byteArray;
  struct _DNSEntry *dnsEntry;

  
  bzero(dns_as_byteArray, tableSize);
  
  dnsEntry = dnsTable->first_DNSentry;
  do
  {
    dnsEntryToByteArray(dnsEntry, &next_dns_entry_in_the_dns_byteArray_ptr);
  }while((dnsEntry=dnsEntry->nextDNSEntry) != NULL);

  return dns_as_byteArray;
  
}

/**
 * Function that gets the dns_file name and port options from the program 
 * execution.
 * @param argc the number of execution parameters
 * @param argv the execution parameters
 * @param reference parameter to set the dns_file name.
 * @param reference parameter to set the port. If no port is specified 
 * the DEFAULT_PORT is returned.
 */
int getProgramOptions(int argc, char* argv[], char *dns_file, int *_port)
{
  int param;
   *_port = DEFAULT_PORT;

  // We process the application execution parameters.
	while((param = getopt(argc, argv, "f:p:")) != -1){
		switch((char) param){		
			case 'f':
				strcpy(dns_file, optarg);				
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
 * Function that generates the array of bytes with the dnsTable data and 
 * sends it.
 * @param s the socket connected to the client.
 * @param dnsTable the table with all the domains
 */


/** 
 * Receives and process the request from a client.
 * @param s the socket connected to the client.
 * @param dnsTable the table with all the domains
 * @return 1 if the user has exit the client application therefore the 
 * connection whith the client has to be closed. 0 if the user is still 
 * interacting with the client application.
 */
int process_msg(int sock, struct _DNSTable *dnsTable)
{
  unsigned short op_code;
  char buffer[MAX_BUFF_SIZE];
  int done = 0;

  int msg_size = recv(sock,buffer, sizeof(buffer),0);
  op_code = ldshort(buffer);

  switch(op_code)
  {
    case MSG_HELLO_RQ:
      process_HELLO_RQ_msg(sock);
      break;  
    case MSG_LIST_RQ:
      process_LIST_RQ_msg(sock, dnsTable);
      break;  
    case MSG_DOMAIN_RQ:
      process_DOMAIN_RQ_msg(sock, buffer, dnsTable, msg_size);
      break;
    case MSG_ADD_DOMAIN:
      process_ADD_DOMAIN_msg(sock, buffer, msg_size, dnsTable); 
      break;    
    case MSG_CHANGE_DOMAIN:
      process_CHANGE_DOMAIN_msg(sock, buffer, msg_size, dnsTable);
      break;      
    case MSG_FINISH:
      done = 1;
      exit(0);
      break;
    default:exit(0);
      perror("Message code does not exist.\n");
  } 
  
  return done;
}

void process_HELLO_RQ_msg(int sock)
{
  char buffer[MAX_BUFF_SIZE];

  char *hola = "Hello World";

  int offset=0;
  int msg_size = strlen(hola);

  memset(buffer, '\0',sizeof(buffer));

  stshort(MSG_HELLO, buffer);

  printf("About to send\n");
  offset+=sizeof(short);

  strcpy(buffer + offset, hola);

  offset+=msg_size; 

  send(sock,buffer,offset + 1,0);

  printf("HELLO WORLD SENT\n");
}

void process_LIST_RQ_msg(int sock, struct _DNSTable *dnsTable)
{
  char *dns_table_as_byteArray;
  char *msg;
  int dns_table_size;
  int msg_size = sizeof(short);
  int offset=0;

  dns_table_as_byteArray = dnsTableToByteArray(dnsTable, &dns_table_size);
  
  msg_size += dns_table_size;
  
  msg = malloc(msg_size);

  stshort(MSG_LIST, msg);

  offset+=sizeof(short);

  memcpy(msg+sizeof(short), dns_table_as_byteArray, dns_table_size);

  offset+=msg_size;

  send(sock, msg, msg_size+1, 0); 

  //send(sock, msg, offset+1, 0);   

  printf("Replaced msg_size2");
}

int process_DOMAIN_RQ_msg(int sock, char* buffer, struct _DNSTable *dnsTable, int rcv)
{
  int counter=0;
  int offset=0;
  struct _DNSEntry *ptr = dnsTable->first_DNSentry;
  int numOfEntries = sizeof(dnsTable) / sizeof(short);
  int msg_size=0;

  char *domainRequested;

  char replyBuffer[MAX_BUFF_SIZE];

  int domainFound = 0;

  domainRequested = buffer + sizeof(short); 

       
  struct in_addr addr;

  stshort(MSG_DOMAIN, replyBuffer);

  offset+=sizeof(short);

  struct _IP* temp;

  temp = dnsTable->first_DNSentry->first_ip;
    while (ptr != NULL) {
      printf("Website name: %s\n", ptr->domainName);
      if (strcmp(ptr->domainName, domainRequested) == 0) {
        printf("found\n");
        domainFound = 1;

        while(ptr->first_ip != NULL) {
          counter++;
            
          struct in_addr address;

          printf("IP #%d for this address: %s\n", counter, inet_ntoa(ptr->first_ip->IP));

          address = ptr->first_ip->IP;

          staddr(address, replyBuffer+offset);

          offset+=sizeof(struct in_addr);
            
          ptr->first_ip = ptr->first_ip->nextIP;
        }
        break;
      } else {
          ptr = ptr->nextDNSEntry;
        }  
    }

    addr = ldaddr(replyBuffer+2);

    printf("reply buffer first address : %s\n", inet_ntoa(addr));

    addr = ldaddr(replyBuffer+6);
    
    printf("reply buffer first address : %s\n", inet_ntoa(addr));

    if (domainFound == 0) {
      sendOpCodeMSG(sock, MSG_OP_ERR);
    }

    send(sock, replyBuffer, offset, 0);


    printf("NUMBER OF IPs FOR THIS ADDRESS: %d\n", counter);
}

void process_ADD_DOMAIN_msg(int sock, char* buffer, int msg_size, struct _DNSTable *dnsTable) {

  struct in_addr address;

  struct in_addr *add;

  printf("PROCESSING ADD_DOMAIN\n");

  struct _DNSEntry *ptr = dnsTable->first_DNSentry;

  struct _DNSEntry *newEntry = malloc(sizeof(struct _DNSEntry));

  newEntry = dnsTable->first_DNSentry;

  newEntry->first_ip = NULL;


  struct _IP *entryIPList = malloc(sizeof(struct _IP));

  struct _IP *nextEntryIPList = malloc(sizeof(struct _IP));
  
  newEntry->nextDNSEntry = NULL;

  char domainName[NAME_LENGTH];

  int offset = 0;

  offset += sizeof(short);

  printf("MSG SIZE: %d\n", msg_size);

  strcpy(newEntry->domainName, buffer+sizeof(short));

  printf("DOMAIN NAME OF NEW ENTRY: %s\n", newEntry->domainName);

  offset += strlen(newEntry->domainName);

  offset += 1;

  newEntry->numberOfIPs = (msg_size-offset)/4;

  printf("THERE ARE %d IPs\n", newEntry->numberOfIPs);

  while (ptr->nextDNSEntry != NULL) {
    printf("DOMAIN NAME OF ENTRY: %s\n", ptr->domainName);
    
  }

  if (ptr->nextDNSEntry == NULL) {



      ptr->nextDNSEntry = newEntry;

      printf("NEW DOMAIN (PUNT): %s\n", ptr->domainName);

      entryIPList->IP = ldaddr(buffer+offset);

      printf("IP OF : %s", inet_ntoa(entryIPList->IP));

      newEntry->first_ip = entryIPList;
      
      //memcpy(&entryIPList->IP, &ldaddr(buffer+offset), sizeof(struct in_addr));

      printf("pointer 1st IP: %s\n", inet_ntoa(entryIPList->IP));

    }
  
  ptr = ptr->nextDNSEntry;
  printDNSTable(dnsTable);
  
}  

void process_CHANGE_DOMAIN_msg(int sock, char* buffer, int msg_size, struct _DNSTable *dnsTable) {

  struct _DNSEntry *ptr = malloc(sizeof(struct _DNSEntry));

  struct _IP *entryIP1 = malloc(sizeof(struct _IP));

  struct _IP *entryIP2 = malloc(sizeof(struct _IP));

  struct in_addr address;

  int domainFound = 0;
  int counter = 0;
  int IPfound = 0;
  int offset = 0;
  struct in_addr addrOld, addrNew;
  char domain[MAX_BUFF_SIZE];

  offset+=sizeof(short);

  strcpy(domain, buffer+offset);

  offset+=strlen(domain)+1;

  searchDomain(domain, dnsTable);

  printf("MSG SIZE %d\n", msg_size);

  addrOld = ldaddr(buffer+offset);

  entryIP1->IP = addrOld;

  offset+=sizeof(struct in_addr);

  addrNew = ldaddr(buffer+offset);

  printf("DOMAIN %s\n", dnsTable->first_DNSentry->domainName);

  printf("ADDR OLD: %s\n", inet_ntoa(entryIP1->IP));

  entryIP2->IP = addrNew;

  printf("NEW IP: %s\n", inet_ntoa(entryIP2->IP));

  ptr = dnsTable->first_DNSentry;

  struct _IP* temp = malloc(sizeof(struct _IP));;

  //temp = dnsTable->first_DNSentry->first_ip;
    while (ptr != NULL) {
      printf("Website name: %s\n", ptr->domainName);
      if (strcmp(ptr->domainName, domain) == 0) {

        temp = ptr->first_ip;

        printf("Domain found\n");
        domainFound = 1;

        while(temp != NULL) {
          

          address = temp->IP;

          if (address.s_addr == addrOld.s_addr) {

            temp->IP = addrNew;

            IPfound = 1;

            printf("IP FOUND AND CHANGED\n");

            printDNSTable(dnsTable);

            ptr->first_ip->IP = addrNew;

            offset+=sizeof(struct in_addr);


          }
          temp = temp->nextIP;
        }
        break;
      } else {
          ptr = ptr->nextDNSEntry;
        }  
    }

    if (domainFound == 0) {
      printf("DOMAIN NOT FOUND\n");
      sendOpCodeMSG(sock, MSG_OP_ERR);
    }

    if (IPfound == 0) {
      printf("IP NOT FOUND. NO IP MODIFIED\n");
    }
  exit(0); 
}

int searchDomain(char* domain, struct _DNSTable *dnsTable) {

  struct _DNSEntry *pointer = malloc(sizeof(struct _DNSEntry));

  pointer = dnsTable->first_DNSentry;

  int found = 0;

  while (pointer != NULL) {

    printf("SEARCHING FOR: %s\n", domain);

    printf("Domain : %s\n", pointer->domainName);

    if (strcmp(pointer->domainName, domain) == 0) {
      printf("DOMAIN FOUND\n");
      found = 1;
      return found;
    }

    pointer = pointer->nextDNSEntry;
  }
  return found;
}

int main (int argc, char * argv[])
{
  struct _DNSTable *dnsTable;
  int port ;
  char dns_file[MAX_FILE_NAME_SIZE];
  int finish = 0;
  
  

  getProgramOptions(argc, argv, dns_file, &port);
  
  int sServer;
  int sClient;


  
  dnsTable = loadDNSTableFromFile(dns_file);

  printDNSTable(dnsTable);

  
  
  struct sockaddr_in client_addr, server_addr;
  sServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);


  server_addr.sin_family= AF_INET;
  server_addr.sin_port= htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  socklen_t client_addr_len = sizeof(client_addr);

  if (bind(sServer, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    printf("Binding succesful\n");
  };

 

  while(1) {

    listen(sServer,MAX_QUEUED_CON);

    printf("Listening \n");
    
    sClient = accept(sServer, (struct sockaddr *)&client_addr, &client_addr_len);

        printf("CONNECTION ACCEPTED\n");
        /*

        Concurrencia

        if (pid = fork() == 0) {

        }

        */
        printf("Listening \n");

         do
          {
            finish = process_msg(sClient, dnsTable);
         } while(!finish);
          //close(sClient);
         exit(0);
    
  }
  
  return 0;
}


