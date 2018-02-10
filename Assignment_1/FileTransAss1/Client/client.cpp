#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <arpa/inet.h>
using namespace std;

int create_socket(int,char *);

#define GetCurrentDir getcwd

#define MAXLINELENGTH 4096 /*max text line length*/

int
main(int argc, char **argv)
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINELENGTH], recvline[MAXLINELENGTH];

 //basic check of the arguments
 //additional checks can be inserted
 if (argc !=3) {
  cerr<<"Usage: ./a.out <IP address of the server> <port number>"<<endl;
  exit(1);
 }

 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  cerr<<"Problem in creating the socket"<<endl;
  exit(2);
 }

 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr= inet_addr(argv[1]);
 servaddr.sin_port =  htons(atoi(argv[2])); //convert to big-endian order

 //Connection of the client to the socket
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  cerr<<"Problem in connecting to the server"<<endl;
  exit(3);
 }

 cout<<"shell>";

 while (fgets(sendline, MAXLINELENGTH, stdin) != NULL) {

  send(sockfd, sendline, MAXLINELENGTH, 0);
  char *token,*dummy;
  dummy=sendline;
  token=strtok(dummy," ");
   
   if (strcmp("quit\n",sendline)==0)  {
   	//close(sockfd);
	return 0;
   }

   else if (strcmp("ls\n",sendline)==0)  {
   	char buff[MAXLINELENGTH],check[MAXLINELENGTH]="1",port[MAXLINELENGTH];
	int data_port,datasock;
	recv(sockfd, port, MAXLINELENGTH,0);				//reciening data connection port
	data_port=atoi(port);
	datasock=create_socket(data_port,argv[1]);
	while(strcmp("1",check)==0){ 				//to indicate that more blocks are coming
		recv(datasock,check,MAXLINELENGTH,0);
		if(strcmp("0",check)==0)			//no more blocks of data
		break;
		recv(datasock, buff, MAXLINELENGTH,0);
		cout<<buff;
	}
	
   }
	
   else if (strcmp("!ls\n",sendline)==0)  {
   	system("ls");
        cout<<"\n";
   }

   else if (strcmp("pwd\n",sendline)==0)  {
   	char curr_dir[MAXLINELENGTH];
	recv(sockfd, curr_dir, MAXLINELENGTH,0);
	cout<<curr_dir<<endl;
   }

   else if (strcmp("!pwd\n",sendline)==0)  {
   	system("pwd");
   }

   else if (strcmp("cd",token)==0)  {
	char check[MAXLINELENGTH];
	token=strtok(NULL," \n");
	cout<<"Path given is: "<<token<<endl;
	recv(sockfd,check,MAXLINELENGTH,0);
	if(strcmp("0",check)==0){
		cerr<<"Directory doesn't exist. Check Path"<<endl;
	}
	
   }

   else if (strcmp("!cd",token)==0)  {
	token=strtok(NULL," \n");
	cout<<"Path given is: "<<token<<endl;
	if(chdir(token)<0){
		cerr<<"Directory doesn't exist. Check path"<<endl;
	}
   }

   else if (strcmp("put",token)==0)  {
   	char port[MAXLINELENGTH], buffer[MAXLINELENGTH],char_num_blks[MAXLINELENGTH],char_num_last_blk[MAXLINELENGTH];
	int data_port,datasock,lSize,num_blks,num_last_blk,i;
	FILE *fp;
	recv(sockfd, port, MAXLINELENGTH,0);				//receiving the data port
	data_port=atoi(port);
	datasock=create_socket(data_port,argv[1]);
	token=strtok(NULL," \n");
	if ((fp=fopen(token,"r"))!=NULL)
	{
		//size of file
		send(sockfd,"1",MAXLINELENGTH,0);
		fseek (fp , 0 , SEEK_END);
		lSize = ftell (fp);
		rewind (fp);
		num_blks = lSize/MAXLINELENGTH;
		num_last_blk = lSize%MAXLINELENGTH; 
		sprintf(char_num_blks,"%d",num_blks);
		send(sockfd, char_num_blks, MAXLINELENGTH, 0);
		//cout<<num_blks<<"	"<<num_last_blk<<endl;

		for(i= 0; i < num_blks; i++) { 
			fread (buffer,sizeof(char),MAXLINELENGTH,fp);
			send(datasock, buffer, MAXLINELENGTH, 0);
			//cout<<buffer<<"	"<<i<<endl;
		}
		sprintf(char_num_last_blk,"%d",num_last_blk);
		send(sockfd, char_num_last_blk, MAXLINELENGTH, 0);
		if (num_last_blk > 0) { 
			fread (buffer,sizeof(char),num_last_blk,fp);
			send(datasock, buffer, MAXLINELENGTH, 0);
			//cout<<buffer<<endl;
		}
		fclose(fp);
		cout<<"File upload done.\n";
	}
	else{
		send(sockfd,"0",MAXLINELENGTH,0);
		cerr<<"Error in opening file. Check filename\nUsage: put filename"<<endl;
	}
   }

   else if (strcmp("get",token)==0)  {
   	char port[MAXLINELENGTH], buffer[MAXLINELENGTH],char_num_blks[MAXLINELENGTH],char_num_last_blk[MAXLINELENGTH],message[MAXLINELENGTH];
	int data_port,datasock,lSize,num_blks,num_last_blk,i;
	FILE *fp;
	recv(sockfd, port, MAXLINELENGTH,0);
	data_port=atoi(port);
	datasock=create_socket(data_port,argv[1]);
	token=strtok(NULL," \n");
	recv(sockfd,message,MAXLINELENGTH,0);
	if(strcmp("1",message)==0){
		if((fp=fopen(token,"w"))==NULL)
			cout<<"Error in creating file\n";
		else
		{
			recv(sockfd, char_num_blks, MAXLINELENGTH,0);
			num_blks=atoi(char_num_blks);
			for(i= 0; i < num_blks; i++) { 
				recv(datasock, buffer, MAXLINELENGTH,0);
				fwrite(buffer,sizeof(char),MAXLINELENGTH,fp);
				//cout<<buffer<<endl;
			}
			recv(sockfd, char_num_last_blk, MAXLINELENGTH,0);
			num_last_blk=atoi(char_num_last_blk);
			if (num_last_blk > 0) { 
				recv(datasock, buffer, MAXLINELENGTH,0);
				fwrite(buffer,sizeof(char),num_last_blk,fp);
				//cout<<buffer<<endl;
			}
			fclose(fp);
			cout<<"File download done."<<endl;
		}
	}
	else{
		cerr<<"Error in opening file. Check filename\nUsage: get filename"<<endl;
	}
   }
   else{
	cerr<<"Error in command. Check Command"<<endl;
   }
   cout<<"shell>";

 }

 exit(0);
}


int create_socket(int port,char *addr)
{
 int sockfd;
 struct sockaddr_in servaddr;


 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  cerr<<"Problem in creating the socket"<<endl;
  exit(2);
 }

 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr= inet_addr(addr);
 servaddr.sin_port =  htons(port); //convert to big-endian order

 //Connection of the client to the socket
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  cerr<<"Problem in creating data channel"<<endl;
  exit(3);
 }

return(sockfd);
}
