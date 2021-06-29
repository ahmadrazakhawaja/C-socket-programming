/* fpont 12/99 */
/* pont.net    */
/* tcpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>


#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x0
#define SERVER_PORT 1500
#define MAX_MSG 1000
#define THREADS 15

pthread_mutex_t mtx;

struct thread_data 
{
    int index;
    int size;
    char dest[50];
    char source[50];
    int offset;
};


void splitter();
int transfer();

int msg_send(int sd,int rc)
{
  
  char str[100];
  memset(str,0x0,100);

  printf("Type Message: ");
  fgets(str,100,stdin);

  str[strcspn(str, "\n")] = 0;
  rc = send(sd, str, strlen(str) + 1, 0);

  if(rc<0) 
  {
    perror("cannot send data ");
    close(sd);
    exit(1);
  }

  if(strcmp(str,"exit")==0)
  {
    printf("connection closed\n");
    return -1;
  }

  printf("data sent: %s\n",str);
  fflush(stdin);
  return(0);
}


int main (int argc, char *argv[]) 
{

  char source_file[100];
  char target_file[100];

  printf("Enter name of file to copy\n");
  fgets(source_file,100,stdin);
  source_file[strcspn(source_file, "\n")] = 0;

  printf("Enter name of target file\n");
  fgets(target_file,100,stdin);
  target_file[strcspn(target_file, "\n")] = 0;

  int sd, rc, i;
  char line[MAX_MSG];
  struct sockaddr_in servAddr;
  //struct hostent *h;
  
  //h = gethostbyname("127.0.0.1");
  /*
  if(h==NULL) {
    //printf("%s: unknown host '%s'\n",argv[0],argv[1]);
    pthread_exit(0);
  }
  */
  servAddr.sin_family = AF_INET;
  //memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servAddr.sin_port = htons(SERVER_PORT);

  /* create socket */
  
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket ");
    pthread_exit(0);
  }

  /*
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = INADDR_ANY;
  localAddr.sin_port = htons(0);
  

  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    //printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
    perror("error ");
    pthread_exit(0);
  }
  */

				
  /* connect to server */
 
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("cannot connect ");
    pthread_exit(0);
  }

  rc = send(sd,target_file,50,0);

  if(rc<0) 
  {
    perror("cannot send data ");
    close(sd);
    pthread_exit(0);
  }
  
  

  transfer(source_file,target_file,sd);

  return 0;
  
}





int read_line(int newSd, char *line_to_return) {
  
  int rcv_ptr=0;
  char rcv_msg[MAX_MSG];
  int n;
  int offset;

  offset=0;

  while(1) {
    if(rcv_ptr==0) {
      /* read data from socket */
      memset(rcv_msg,0x0,MAX_MSG); /* init buffer */
      n = recv(newSd, rcv_msg, MAX_MSG, 0); /* wait for data */
      if (n<0) {
	perror(" cannot receive data ");
	return ERROR;
      } else if (n==0) {
	printf(" connection closed by client\n");
	close(newSd);
	return ERROR;
      }
    }
  
    /* if new data read on socket */
    /* OR */
    /* if another line is still in buffer */

    /* copy line into 'line_to_return' */
    while(*(rcv_msg+rcv_ptr)!=END_LINE && rcv_ptr<n) {
      memcpy(line_to_return+offset,rcv_msg+rcv_ptr,1);
      offset++;
      rcv_ptr++;
    }
    
    /* end of line + end of buffer => return line */
    if(rcv_ptr==n-1) { 
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr=0;
      return ++offset;
    } 
    
    /* end of line but still some data in buffer => return line */
    if(rcv_ptr <n-1) {
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr++;
      return ++offset;
    }

    /* end of buffer but line is not ended => */
    /*  wait for more data to arrive on socket */
    if(rcv_ptr == n) {
      rcv_ptr = 0;
    } 
    
   /* while */
  }
}










void splitter(void *data)
{

  struct thread_data *datax = data;
  int sd, rc, i;
  char line[MAX_MSG];
  struct sockaddr_in servAddr;
  //struct hostent *h;
  
  //h = gethostbyname("127.0.0.1");
  /*
  if(h==NULL) {
    //printf("%s: unknown host '%s'\n",argv[0],argv[1]);
    pthread_exit(0);
  }
  */

  servAddr.sin_family = AF_INET;
  //memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servAddr.sin_port = htons(SERVER_PORT);

  /* create socket */
  
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket ");
    pthread_exit(0);
  }

  
  /*
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(datax->index);
  //printf("%d\n",localAddr.sin_port);


  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    //printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
    perror("error ");
    pthread_exit(0);
  }
  */

				
  /* connect to server */
  pthread_mutex_lock(&mtx);
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("cannot connect ");
    pthread_exit(0);
  }
  pthread_mutex_unlock(&mtx);

  printf("Thread%d: connected successfully\n",datax->index);

  
  
  
  void *buf;
  buf = malloc(1024);

  int size = 0;

  int offset = datax->index*datax->offset;

  char dest_part[30];
  sprintf(dest_part,"part%d_%s",datax->index,datax->dest);

  //printf("%u/%u/%u\n",offset,datax->index,datax->size);

  sprintf(line,"thread_accepting %s",dest_part);
  rc = send(sd,line,40,0);

  if(rc<0) 
  {
    perror("cannot send data ");
    close(sd);
    pthread_exit(0);
  }

  int source = open(datax->source, O_RDONLY);

  if (source == -1)
  {
    printf("Press any key to exit...\n");
    pthread_exit(0);
  }

  int n;
  lseek(source,offset,SEEK_SET);

  int total_block = datax->size;

  int transfer_block = datax->size;
  if(transfer_block > 1024)
  {  
      transfer_block = 1024;
  }

  if(read_line(sd,line)!=ERROR)
  {
    //printf("%s\n",line);
    if(strcmp(line,"start")!=0)
    {
      printf("%s\n","thread ended");
      pthread_exit(0);
    }
    
  }
  
  memset(line,0x0,MAX_MSG);
  
  while( (size = read(source,buf,transfer_block)) != 0)
    {
      if (size == -1)
      {
        if (errno == EINTR)
        {
          continue;
        }
        perror("read");
        break;        
      }
      memset(line,0x0,MAX_MSG);

      rc = send(sd,buf,size,0);

      if(rc<0) 
      {
        perror("cannot send data ");
        close(sd);
        close(source);
        pthread_exit(0);
      }

      n = recv(sd, line, MAX_MSG, 0);
      if (n<0) 
      {
        perror(" cannot receive data ");
        pthread_exit(0);
      } 
      else if (n==0) 
      {
        printf(" connection closed by client\n");
        close(sd);
        pthread_exit(0);
      }
    
      
      if(strcmp(line,"recieved")!=0)
      {
        break;
      }
      

      total_block = total_block - size;
  
      if(total_block < 1024)
      {
          transfer_block = total_block;
      }


      
      memset(buf,0x0,1024); 
    }

    strcpy(line,"done");
    rc = send(sd,line,MAX_MSG,0);
    if(rc<0) 
    {
      perror("cannot send data ");
      close(sd);
      close(source);
      pthread_exit(0);
    }
  
  
  
  printf("Thread %u: data transferred successfully.\n",datax->index);
  close(source);
  close(sd);
  free(buf);
  pthread_exit(0);

  
      
    
}



int transfer(char source_file[], char target_file[],int sd)
{
    //char source_file[20], target_file[20];
    int threads = THREADS;

    /*
    printf("Enter name of file to copy\n");
    gets(source_file);
    */
    

    int size;
    struct stat st;
    stat(source_file, &st);
    size = st.st_size;
    //size = lseek(source,0,SEEK_END);

    //lseek(source,0,SEEK_SET);
    int thread_size;

    thread_size = size/threads;
    int final_block = size%threads;
    if(thread_size==0)
    {
        threads = 1;
        thread_size = size;
        final_block = 0;
    }
    
    char sendx[50];
    sprintf(sendx,"threads: %d",threads);
    int rc = send(sd,sendx,50,0);

    if(rc<0) 
    {
      perror("cannot send data ");
      close(sd);
      pthread_exit(0);
    }

    
    memset(sendx,0x0,50);
    if(read_line(sd,sendx)!=ERROR)
    {
      //printf("%s\n",sendx);
      if(strcmp(sendx,"done")==0)
      {
        close(sd);
      }
  
    }

    pthread_t T[threads];
    
    struct thread_data data[threads];

    pthread_mutex_init(&mtx,0);
    for(int i = 0; i < threads; i++)
    {
        data[i].index = i;
        data[i].offset = thread_size;
        if(i+1==threads)
        {
            data[i].size = thread_size+final_block;
        }
        else
        {
            data[i].size = thread_size;
        }

        strcpy(data[i].source,source_file);
        strcpy(data[i].dest,target_file);

        

        pthread_create(&T[i],0,splitter,(void *)&data[i]);
    }
    

   for(int i = 0; i < threads; i++)
    {
        pthread_join(T[i],0);
    }
    pthread_mutex_destroy(&mtx);

    printf("File transfer completed\n");
    
    return 0;
    
}

