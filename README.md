# C-socket-programming

## Project Description:
This is a file downloader application built using socket programming in C language. The file size the application is expected to transfer is around 5-10GB. Inorder to transfer the file at the other end the program splits the the file data into equal size chunks and transfer the data through multiple threads.

<img width="715" alt="c-image" src="https://user-images.githubusercontent.com/46644351/123847858-7a648e00-d930-11eb-9d48-98f334bca883.png">




## Files Description:

1. **peer1.c**: 
This code is a file sender program. It first initiates a socket connection on TCP and first thing it sends is the number of threads to the recievinng program. After this it will calculate the chunk size that each thread will be transferring. Then it generates 15 threads and all the 15 threads start their own socket connection and try to connenct to reciving end. After the connnection is established each thread starts sennding the data. after sending all the the data the threads close the socket connection and the threads are terminated and the program exits.

2. **peer2.c**:
This code is a file reciever program. It starts a socket connection and recieves the number of threads. After which the program blocks in the accept method and waits for the clients to connect. After a connection is established a new Posix thread is generated and the socked id is passed to that thread to pursue communication while the main method is again blocked in accept method waithing for clients. Each thread recieves the data and stores it in a file. After all the threads have recieved and stored the data and returns the threads are terminated after which a new joinable thread is generated to merge all the part file into a single complete file. This threads will read data from all the part files and store them in a single file along with deleting the part file after it is read. After the task of merging is finished the thread exits and the program terminates.
