/* fpont 1/00 */
/* pont.net    */
/* tcpServer.c */

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

// defined mutex
pthread_mutex_t mtx;

struct thread_data 
{
    int index;
    int size;
    char source[30];
    int port;
    int oldsd;
};

struct global_data 
{
    int threads;
    char dest[50];
    int sd;
};

/* function readline */
int read_line();
void transfer();
void merger();
int tok();
void get_info();

int main (int argc, char *argv[]) 
{
  
  int sd, newSd, cliLen, rc,*new_sock;

  struct sockaddr_in cliAddr, servAddr;
  char line[MAX_MSG];


  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) 
  {
    perror("cannot open socket ");
    return ERROR;
  }
  
  /* bind server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(SERVER_PORT);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("cannot bind port ");
    return ERROR;
  }

  listen(sd,16);
  

  printf("%s: waiting for data on port TCP %u\n",argv[0],SERVER_PORT);


  cliLen = sizeof(cliAddr);

  //pthread_t T[THREADS];

  int count;
  char dest[50];
  int threads;
  //pthread_t T;
  //struct global_data data;

  newSd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen);
  if(newSd<0) 
  {
    perror("cannot accept connection ");
    return ERROR;
  }

  if(read_line(newSd,dest)!=ERROR)
  {
    //printf("%s\n",dest);
  }

  
  if(read_line(newSd,line)!=ERROR)
  {
    
    tok2(line,&threads);
    //printf("%d\n",threads);
    memset(line,0x0,MAX_MSG);
  }
  strcpy(line,"done");

  rc = send(newSd,line,40,0);

  if(rc<0) 
  {
    perror("cannot send data ");
    close(newSd);
    pthread_exit(0);
  }

  

  close(newSd);

  
  pthread_t T[threads];
  
  //pthread_mutex_init(&mtx,0);
  for(int i=0; i < threads; i++)
  {
    newSd = accept(sd, (struct sockaddr *) &cliAddr, &cliLen);

    if(newSd<0) 
    {
      perror("cannot accept connection ");
      return ERROR;
    }

    new_sock = malloc(1);
    *new_sock = newSd;

    if(pthread_create(&T[i],0,transfer,(void*) new_sock)<0)
    {
      perror("could not create thread");
      return 1;
    }

    count++;
  }

  for(int i=0; i < count; i++)
  {
    pthread_join(T[i],0);
  }

  
  //pthread_mutex_destroy(&mtx);

  pthread_t Tl;

  pthread_create(&Tl,0,merger,(void *)dest);

  pthread_join(Tl,0);

  printf("Files merged\n");
  
    
} 




/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */
/* this function is experimental.. I don't know yet if it works  */
/* correctly or not. Use Steven's readline() function to have    */
/* something robust.                                             */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */

/* rcv_line is my function readline(). Data is read from the socket when */
/* needed, but not byte after bytes. All the received data is read.      */
/* This means only one call to recv(), instead of one call for           */
/* each received byte.                                                   */
/* You can set END_CHAR to whatever means endofline for you. (0x0A is \n)*/
/* read_lin returns the number of bytes returned in line_to_return       */
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

int tok(char line[], char dest[])
{
  char *token;

  token = strtok(line," ");

  int len = strlen(token);
  if((int)token[len-1]==10)
  {
      token[len-1]='\0';
  }

  if(strcmp(token,"thread_accepting")==0)
  {
    token = strtok(NULL, " ");
    strcpy(dest,token);
    //printf("%s\n",dest);
  }
  else
  {
    return -1;
  }
  return 0;
}

int tok2(char line[], int *threads)
{
  char *token;

  token = strtok(line," ");

  int len = strlen(token);
  if((int)token[len-1]==10)
  {
      token[len-1]='\0';
  }

  if(strcmp(token,"threads:")==0)
  {
    token = strtok(NULL, " ");
    *threads = atoi(token);
  }
  else
  {
    return -1;
  }
  return 0;
}


void transfer(void *socket_desc)
{   
    int sd = *(int*)socket_desc;
    int rc;
    char line[MAX_MSG];
    char dest_file[50];
    int s;
    /*
    while (1)
    {
      if(read_line(sd,line)!=ERROR)
      {
        printf("%s\n",line);
        memset(line,0x0,MAX_MSG);
      }
    }
    */
  
    s = recv(sd, line, MAX_MSG, 0); /* wait for data */
    if (s<0) 
    {
      perror(" cannot receive data ");
      pthread_exit(0);
    } 
    else if (s==0) 
    {
      printf(" connection closed by client\n");
      close(sd);
      pthread_exit(0);
    }

    tok(line,dest_file);
    
    /* init line */
    memset(line,0x0,MAX_MSG);

    void *buf;
    buf = malloc(1024);

    int size = 0;
    
    int target = creat(dest_file, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);

    if (target == -1)
    {
        printf("Press any key to exit...\n");
        free(buf);
        pthread_exit(0);
    }
    //printf("%u/%u/%u\n",offset,datax->index,datax->size);

    
    rc = send(sd, "start", 10, 0);
    if(rc<0) 
    {
      perror("cannot send data ");
      close(sd);
      free(buf);
      pthread_exit(0);
    }
    
    int n = recv(sd, buf, 1024, 0);
    while(strcmp("done",buf)!=0)
    {
      write(target,buf,n);
      memset(buf,0x0,1024);

      rc = send(sd,"recieved", 10, 0);

      if(rc<0) 
      {
        perror("cannot send data ");
        close(sd);
        free(buf);
        pthread_exit(0);
      }

      n = recv(sd, buf, 1024, 0);

    }
    /*
    while( (size = read(source,buf,1024)) != 0)
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
            read_line(newSd,line);
      

            rc = send(newSd,buf,size,0);

            if(rc<0) 
            {
              perror("cannot send data ");
              close(sd);
              exit(1);
            }
            
    }
    rc = send(newSd,"done",10,0);
    if(rc<0) 
    {
      perror("cannot send data ");
      close(sd);
      exit(1);
    }
    */
    printf("%s: data recieved successfully.\n",dest_file);
    free(buf);
    close(target);
    
    pthread_exit(0);
    
      
}



void merger(void *data)
{
    char *datax = (char*) data;
    //printf("%s\n",datax);
    char part_file[50];
    int target;

    void *buf;
    buf = malloc(1024);

    target = creat(datax, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);

    if (target == -1)
    {
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    while(1)
    {
        sprintf(part_file,"part%d_%s",count,datax);
        count++;

        int source = open(part_file, O_RDONLY);

        if (source == -1)
        {
            //printf("Press any key to exit...\n");
            close(source);
            close(target);
            pthread_exit(0);
        }

        int size;

        while( (size = read(source,buf,1024)) != 0)
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
                write(target,buf,size);
        }
        close(source);
        remove(part_file);
        
    }
    printf("data merged successfully.\n");
    close(target);
    pthread_exit(0);
}

/*
void get_info(void *data)
{
  char line[100];
  struct global_data *datax = data;
  int newSd = datax->sd;
  if(newSd<0) 
  {
    perror("cannot accept connection ");
    return ERROR;
  }

  if(read_line(newSd,datax->dest)!=ERROR)
  {
    printf("%s\n",datax->dest);
  }

  
  if(read_line(newSd,line)!=ERROR)
  {
    
    tok2(line,&datax->threads);
    printf("%d\n",datax->threads);
    memset(line,0x0,100);
  }
  strcpy(line,"done");

  int rc = send(newSd,line,40,0);

  if(rc<0) 
  {
    perror("cannot send data ");
    close(newSd);
    pthread_exit(0);
  }

  

  close(newSd);
}
*/



  
  
