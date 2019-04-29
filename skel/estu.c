// NAMESERVER

int main (int argc, char * argv[])
{
  struct _DNSTable *dnsTable;
  int port ;
  char dns_file[MAX_FILE_NAME_SIZE] ;
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

  bind(sServer, (struct sockaddr *)&server_addr, sizeof(server_addr));

  listen(sServer,MAX_QUEUED_CON);
  
  
  
  while(1) {

    printf("Listening \n");
    if (sClient = accept(sServer, (struct sockaddr *)&client_addr, &client_addr_len)== 0){
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
  }
  
  return 0;
}

// NAMECLIENT

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
		printf("Successful \n");
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

