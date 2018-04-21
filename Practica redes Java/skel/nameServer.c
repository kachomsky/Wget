
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
void process_LIST_RQ_msg(int sock, struct _DNSTable *dnsTable)
{
  char *dns_table_as_byteArray;
  char *msg;
  int dns_table_size;
  int msg_size = sizeof(short);
  //char buffer[DNS_TABLE_MAX_SIZE]; 
  char buffer[MAX_BUFF_SIZE]; 
  dns_table_size = getDNSTableSize(dnsTable);//sizeof(dnsTable);

  dns_table_as_byteArray = dnsTableToByteArray(dnsTable, &dns_table_size);
  
  msg_size += dns_table_size;

  stshort(MSG_LIST, buffer);

  msg_size += sizeof(short);
  
  msg = malloc(msg_size);
  //TODO: set the operation code and the table data

  memcpy(buffer+sizeof(short), dns_table_as_byteArray, msg_size);
  //TODO: send the message
  send(sock, buffer, msg_size, 0);
  
}

void process_MSG_DOMAIN_RQ(int sock, struct _DNSTable *dnsTable, char buffer[MAX_BUFF_SIZE]){
  struct _DNSEntry *dnsEntry;
  int tamany_buffer = 0;
  dnsEntry = dnsTable->first_DNSentry;
  struct _IP* pIP;
  _Bool dnsEntryFound = 0;
  //char buffer[MAX_BUFF_SIZE];
  do{
      if(strcmp(buffer+sizeof(short),dnsEntry->domainName)==0){
        dnsEntryFound = 1;
        //delete buffer;
        memset(buffer, '\0', sizeof(buffer));        
        //staddr(pIP->IP, dnsEntry->first_ip);
        pIP = dnsEntry->first_ip;
        stshort(MSG_IP_LIST, buffer);
        tamany_buffer += sizeof(short);
        do{
          strcpy(buffer+tamany_buffer,inet_ntoa(pIP->IP));
          tamany_buffer += strlen(inet_ntoa(pIP->IP));

          strcpy(buffer+tamany_buffer," ");
          tamany_buffer += 1;
          
          pIP = pIP->nextIP;  
        }while(pIP != NULL);
      }
    }while((dnsEntry=dnsEntry->nextDNSEntry) != NULL);
    if(dnsEntryFound){
      send(sock, buffer, tamany_buffer, 0);
    }else{
      //stshort(MSG_OP_ERR,errorMessage);
      //stshort(ERR_2, errorMessage+sizeof(short));
      //send(sock, errorMessage, 4, 0);
      sendOpCodeMSG(sock, MSG_OP_ERR);
    }
}

void process_MSG_ADD_DOMAIN_RQ(int sock, struct _DNSTable *dnsTable, char* buffer, int tamany_missatge){
  struct _DNSEntry* dnsEntryFromClient;
  struct _DNSEntry *dnsSearch = dnsTable->first_DNSentry;
  struct _IP* pIP;
  struct _IP* ipEntryClient;
  struct _IP* lastIP;
  _Bool dnsEntryFound = 0;
  int tamany_acumulat = sizeof(short)+strlen(buffer);
  

  dnsEntryFromClient = buildADNSEntryFromALine(buffer+sizeof(short), "-");
  ipEntryClient = dnsEntryFromClient->first_ip;

  do{
      if(strcmp(buffer+tamany_acumulat,dnsSearch->domainName)==0){
        
        dnsEntryFound = 1;
        pIP = dnsSearch->first_ip;
        do{
          lastIP = pIP;
          pIP = pIP->nextIP;
        }while(pIP != NULL);
          //lastIP->nextIP = pIP;
          pIP = malloc(sizeof(struct _IP));
          pIP->IP = ipEntryClient->IP;
          pIP->nextIP = NULL;
          lastIP->nextIP = pIP;
          lastIP = pIP;
          //pIP = pIP->nextIP;
          //pIP->nextIP = NULL;
          //lastIP->nextIP = pIP;
          //lastIP = pIP;
          (dnsSearch->numberOfIPs)++;
         // ipEntryClient = ipEntryClient->nextIP;

          while((ipEntryClient = ipEntryClient->nextIP) != NULL){
            //ipEntryClient = inet_aton(ipEntryClient);
            pIP = malloc(sizeof(struct _IP));
            pIP->IP = ipEntryClient->IP;
            pIP->nextIP = NULL;
            lastIP->nextIP = pIP;
            lastIP = pIP;
            (dnsSearch->numberOfIPs)++;
      }
    }
  }while((dnsSearch = dnsSearch->nextDNSEntry) != NULL);

  /*do{
      if(strcmp(buffer+tamany_acumulat,dnsSearch->domainName)==0){
        
        dnsEntryFound = 1;
        pIP = dnsSearch->first_ip;

        do{
          lastIP = pIP;
          pIP = pIP->nextIP;
        }while(pIP != NULL);
        tamany_acumulat += strlen(buffer+tamany_acumulat)+1;
        
        while(tamany_acumulat < tamany_missatge){
          pIP = malloc(sizeof(struct _IP));
          pIP->IP = ldaddr(buffer+tamany_acumulat);
          pIP->nextIP = NULL;
          lastIP->nextIP = pIP;
          lastIP = pIP;
          tamany_acumulat += sizeof(in_addr_t);
          (dnsSearch->numberOfIPs)++;
        }    
      
      }*/

        /*while((ipEntryClient = ipEntryClient->nextIP) != NULL){
          //ipEntryClient = inet_aton(ipEntryClient);
          pIP->IP = ldaddr(ipEntryClient);
          pIP->nextIP = NULL;
          lastIP->nextIP = pIP;
          lastIP = pIP;
          //(dnsSearch->numberOfIPs)++;
        }*/

        //dnsSearch->numberOfIPs++; //MIRAR algo falla aqui
      
      
    //}while((dnsSearch = dnsSearch->nextDNSEntry) != NULL);
        
    if(dnsEntryFound){
      sendOpCodeMSG(sock, MSG_OP_OK);
      //send(sock, dnsSearch->numberOfIPs, sizeof(int), 0);
    }else{
      sendOpCodeMSG(sock, MSG_OP_ERR);
      //stshort(MSG_OP_ERR,errorMessage);
      //stshort(ERR_2, errorMessage+sizeof(short));
      //send(sock, errorMessage, 4, 0);
    }
}

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
  int bytes_rebuts = 0;
  int tamany_buffer = 0;
  int errors;
  

  //TODO: .... 
  memset(buffer, '\0', sizeof(buffer));
  
  bytes_rebuts = recv(sock, buffer, sizeof(buffer), 0);

  if(bytes_rebuts < 0){
    printf("Error al recibir los datos");
  }
  
  op_code = ldshort(buffer);
  
  //TODO: ....
  switch(op_code)
  {

    case MSG_HELLO_RQ:
      //TODO
      memset(buffer, '\0', sizeof(buffer));
      stshort(2, buffer);
      strcpy(buffer+sizeof(short),"Hello World");
      tamany_buffer = sizeof(short)+strlen(buffer+1);
      send(sock, buffer, tamany_buffer, 0); 
      break;  
    case MSG_LIST_RQ:
      //perror("Hola has llegado al servidor :D");
      process_LIST_RQ_msg(sock, dnsTable);
      break;        
    case MSG_DOMAIN_RQ:
      //TODO
      process_MSG_DOMAIN_RQ(sock,dnsTable, buffer);
      break;          
    case MSG_ADD_DOMAIN:
      process_MSG_ADD_DOMAIN_RQ(sock, dnsTable, buffer, bytes_rebuts);
    break;
    case MSG_FINISH:
      //TODO
      done = 1;
      errors = close(sock);
      exit(0);
      
      break;
    default:
      perror("Message code does not exist.\n");
  } 

  //process_msg(sock, dnsTable);
  
  return done;
}

int main (int argc, char * argv[])
{
  struct _DNSTable *dnsTable;
  int port ;
  char dns_file[MAX_FILE_NAME_SIZE] ;
  int finish = 0;
  int pid = 0;
  
  getProgramOptions(argc, argv, dns_file, &port);
  
  dnsTable = loadDNSTableFromFile(dns_file);
  printDNSTable(dnsTable);
  
  //TODO: setting up the socket for communication
  
  //creamos la estructura socket
  int s_serv = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP); //domain:pf_inet(tcp/ip), type:sock_stream(servei orientat a conexio per flux), protocol:ipproto

  struct sockaddr_in addr; // creamos estructura que contiene todos los datos necesarios para una conexion con sockets:familia de direccones, ip, puerto
  addr.sin_family = AF_INET;
  addr.sin_port = htons(DEFAULT_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY); //inaddr_any cogera el localhost, direccion ip propia


  //asociamos la estructura addr al socket a traves de la funcion bind
  int errorBind = bind(s_serv,(struct sockaddr*)&addr, sizeof(addr));

  if(errorBind < 0){
    perror("Error en bind. Direccion en uso");
  }
  
  while(1) {
    //TODO:...
    struct sockaddr_in address_conn;//direccion que usaremos para aceptar la conexion del cliente
    listen(s_serv,SOMAXCONN);
    //int s = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    int con_sock;
    socklen_t addrlen = sizeof(address_conn);
    con_sock = accept(s_serv,(struct sockaddr*)&address_conn,&addrlen);
    //TODO: Descomentar la línia
    pid = fork();
    if(pid == 0){
      while(!finish){
        finish = process_msg(con_sock, dnsTable);
      }      
    }else if(pid == -1){
      perror("\n ************ Error al crear el proceso ************");
      exit(-1);
    }
    
  }
 
  
  return 0;
}


