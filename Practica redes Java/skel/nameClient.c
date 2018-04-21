

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
  int port = DEFAULT_PORT;

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
				port = atoi(optarg);
				*_port = port;
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

void process_hello_operation(int s){
  unsigned short code_response = 0;
  int bytes_rebuts = 0;
  char buffer[MAX_BUFF_SIZE];
  sendOpCodeMSG(s, MSG_HELLO_RQ);   
  bytes_rebuts = recv(s, buffer, sizeof(buffer), 0);
  code_response = ldshort(buffer);
  printf("**************************\n");
  printf("%s\n\n", buffer+sizeof(short));
  printf("El codigo de respuesta es: ");
  printf("%i", code_response);
  printf("\n**************************");
}

/**
 * Function that sends a list request receives the list and displays it.
 * @param s The communications socket. 
 */
void process_list_operation(int s)
{
  //char buffer[DNS_TABLE_MAX_SIZE];
  char buffer[MAX_BUFF_SIZE];
  int msg_size;
  short op_code;

  //TODO: uncomment sendOpCodeMSG(s, MSG_LIST_RQ); //remember to implement sendOpCode in common.c
  sendOpCodeMSG(s, MSG_LIST_RQ);
  memset(buffer, '\0', sizeof(buffer));
  //TODO: rebre missatge LIST
  msg_size = recv(s, buffer, sizeof(buffer), 0);
  op_code = ldshort(buffer);
  //TODO: Descomentar la següent línia
  printf("La tabla DNS es la siguiente:\n");
  printf("****************************************\n");
  printDNSTableFromAnArrayOfBytes(buffer+sizeof(short), msg_size-sizeof(short));
  printf("El codigo de respuesta es: %i", op_code);
  printf("\n\n****************************************");
}

void process_domain_operation(int s){
  unsigned short code_response = 0;
  int tamany_missatge = 0;
  char nomDomini[NAME_LENGTH]  = "";
  char buffer[MAX_BUFF_SIZE];
  int bytes_rebuts = 0;

  stshort(MSG_DOMAIN_RQ, buffer);
  
  printf("Introduce el nombre del dominio que quieres consultar:");
  scanf("%s",&nomDomini);

  tamany_missatge = strlen(nomDomini);
  strcpy(buffer+sizeof(short), nomDomini);
  
  tamany_missatge += sizeof(short);
  
  send(s, buffer, tamany_missatge, 0);
  
  memset(buffer, '\0', sizeof(buffer));
  bytes_rebuts = recv(s, buffer, sizeof(buffer), 0);
  code_response = ldshort(buffer);
  
  if(code_response == MSG_IP_LIST){
    printf("********************************\n");
    printf("La lista de IPs para este dominio:\n\n %s\n", buffer+sizeof(short));
    printf("El codigo de respuesta es: %i\n", code_response);
    printf("********************************\n");
  }else{
    printf("********************************\n");
    printf("El codigo de respuesta es: %i\n\n", code_response);
    
    printf("El dominio solicitado no existe\n");
    printf("********************************\n");
  }
}

void process_add_ip(int s){
  unsigned short code_response = 0;
  int tamany_missatge = 0;
  char nomDomini[NAME_LENGTH]  = "";
  char buffer[MAX_BUFF_SIZE];
  char ipAdd[NAME_LENGTH];
  stshort(MSG_ADD_DOMAIN, buffer);
  
  printf("Introduce el nombre del dominio al que añadiras la ip:");
  scanf("%s",&nomDomini);
  
  printf("Introduce la IP:");
  scanf("%s",&ipAdd);
  
  tamany_missatge += sizeof(short);
  
  strcpy(buffer+tamany_missatge,nomDomini);
  tamany_missatge += strlen(nomDomini);

  strcpy(buffer+tamany_missatge,"-"); 
  tamany_missatge += 1;       
  
  strcpy(buffer+tamany_missatge, ipAdd);
  tamany_missatge += strlen(ipAdd);
  
  send(s, buffer, tamany_missatge, 0);
  tamany_missatge = recv(s, buffer, sizeof(buffer), 0);
  
  code_response = ldshort(buffer);
  if(code_response == MSG_OP_OK){
    printf("********************************\n");
    printf("El codigo de respuesta es: %i\n", code_response);
    printf("La ip se ha añadido correctamente\n");
    printf("********************************\n");
  }else{
    printf("********************************\n");
    printf("El codigo de respuesta es: %i\n", code_response);
    printf("No se ha encontrado el dominio solicitado \n");
    printf("********************************\n");
  }
}

void process_add_ips(int s){
  unsigned short code_response = 0;
  int tamany_missatge = 0;
  char nomDomini[NAME_LENGTH]  = "";
  char buffer[MAX_BUFF_SIZE];
  char ipAdd[NAME_LENGTH];
  stshort(MSG_ADD_DOMAIN, buffer);
  char opcio = 's';
  struct in_addr ipEntry;
  
  printf("Introduce el nombre del dominio al que añadiras la ip:");
  scanf("%s",&nomDomini);
  
  tamany_missatge += sizeof(short);
  
  strcpy(buffer+tamany_missatge,nomDomini);
  tamany_missatge += strlen(nomDomini);

  while(opcio != 'n'){
    printf("Introduce la IP:");
    scanf("%s",&ipAdd);
    strcpy(buffer+tamany_missatge,"-"); 
    tamany_missatge += 1;    
    strcpy(buffer+tamany_missatge, ipAdd);
    //inet_aton(buffer+tamany_missatge, &ipEntry);
    //staddr(ipEntry, buffer+tamany_missatge);
    tamany_missatge += strlen(ipAdd);
    printf("Desea introducir mas IPs? s/n\n");
    scanf("%s",&opcio);
  }
  
  send(s, buffer, tamany_missatge, 0);
  tamany_missatge = recv(s, buffer, sizeof(buffer), 0);
  
  code_response = ldshort(buffer);
  if(code_response == MSG_OP_OK){
    printf("********************************\n");
    printf("El codigo de respuesta es: %i\n", code_response);
    printf("Las ips se ha añadido correctamente\n");
    printf("********************************\n");
  }else{
    printf("********************************\n");
    printf("El codigo de respuesta es: %i\n", code_response);
    printf("No se ha encontrado el dominio solicitado \n");
    printf("********************************\n");
  }
}

/** 
 * Function that process the menu option set by the user by calling 
 * the function related to the menu option.
 * @param s The communications socket
 * @param option the menu option specified by the user.
 */
void process_menu_option(int s, int option)
{		  	   
  switch(option){
  	//memset(buffer,'\0', sizeof(buffer));
    // Opció HELLO
    case MENU_OP_HELLO:
      //TODO:
      process_hello_operation(s);
      break;
    case MENU_OP_LIST:
      process_list_operation(s);
      break;

    case MENU_OP_DOMAIN_RQ:
      process_domain_operation(s);
      break;

    case MENU_OP_ADD_DOMAIN_IP:
      process_add_ip(s);    	
		//TODO: Descomentar la següent línia
     break;

    case MENU_OP_ADD_DOMAIN_IPS:
      process_add_ips(s);
    break;

    case MENU_OP_FINISH:
      //TODO:
    	sendOpCodeMSG(s,MSG_FINISH);
      break;
                
    default:
          printf("Invalid menu option\n");
  		}
}

int main(int argc, char *argv[])
{
	int port = DEFAULT_PORT; // variable per al port inicialitzada al valor DEFAULT_PORT (veure common.h)
	char host[MAX_HOST_SIZE]; // variable per copiar el nom del host des de l'optarg
	int option = 0; // variable de control del menu d'opcions
	int ctrl_options;
	int s;
	int connError;
  
  ctrl_options = getProgramOptions(argc, argv, host, &port);

	// Comprovem que s'hagi introduit un host. En cas contrari, terminem l'execucio de
	// l'aplicatiu	
	if(ctrl_options<0){
		perror("No s'ha especificat el nom del servidor\n\n");
		return -1;
	}

 //TODO: setting up the socket for communication
	s = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

	struct sockaddr_in target;	
	target.sin_family = AF_INET;
	target.sin_port = htons(port);
	setaddrbyname((struct sockaddr_in*)&target, host);
	
	connError = connect(s, (const struct sockaddr*)&target,sizeof(target)); //retorna 0 si todo ha ido bien, sino retornara un codigo de error
	
	if(connError != 0){
		perror("Ha ocurrido un error en la conexion");
	}
  do{
	//connError = connect(s, (const struct sockaddr*)&target,sizeof(target)); //retorna 0 si todo ha ido bien, sino retornara un codigo de error
	printa_menu();
	  // getting the user input.
	scanf("%d",&option);
	printf("\n\n");
	
	process_menu_option(s, option);

	  }while(option != MENU_OP_FINISH); //end while(opcio)
    // TODO

  return 0;
}

