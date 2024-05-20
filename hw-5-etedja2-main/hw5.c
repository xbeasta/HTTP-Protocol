#include "hw5-pthread.h"
#include "hw5.h"

#define BMAX 1024
#define HMAX 1024
#define RMAX 4096

static const char OK200[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n";
static const char ping_request[] = "GET /ping HTTP/1.1\r\n\r\n";
static const char ping_body[] = "pong";
static const char message[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n";
static const char err404_response[30] = "HTTP/1.1 404 Not Found\r\n\r\n";
static const char err400_response[32] = "HTTP/1.1 400 Bad Request\r\n\r\n";
static char shared_saved_message[RMAX] = "<empty>";
static int contentLength = 7;

sem_t sem;
sem_t sem_nrequests;
sem_t sem_nheaders;
sem_t sem_nbodys;
sem_t sem_nerrors;
sem_t sem_nerror_bytes;
sem_t sem_shared_message;
sem_t sem_contentLength;

static int NREQUESTS = 0;
static int NHEADERS = 0;
static int NBODYS = 0;
static int NERRORS = 0;
static int NERROR_BYTES = 0;

int Socket(int namespace, int style, int protocol);
void Bind(int sockfd, struct sockaddr * server, socklen_t length);
void Listen(int sockfd, int qlen);
int Accept(int sockfd, struct sockaddr* addr, socklen_t * length_ptr);

static void Fstat(int fd, struct stat * buf) {
    int ret = fstat(fd, buf);
    if (ret < 0) {
        perror("fstat");
        exit(1);
    }
}

static off_t get_size(int fd) {
    if (fd == -1) {
        return -1;
    }

    struct stat buf;
    Fstat(fd, &buf);
    return buf.st_size;
}

int open_listenfd(int port) {
    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    static struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));

    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    Bind(listenfd, (struct sockaddr*) &server, sizeof(server) );
    Listen(listenfd, 10);

    return listenfd;
}

int create_server_socket(int port, int threads)
{   
    // Configure listening socket

    // Create a pool of threads (for tests 09 and 10)
    // Parameter threads is only using for test 09 and 10
    // Initialize sync variables
    int listenfd = open_listenfd(port);

    sem_init(&sem, 0, 1);
    sem_init(&sem_nrequests, 0, 1);
    sem_init(&sem_nheaders, 0, 1);
    sem_init(&sem_nbodys, 0, 1);
    sem_init(&sem_nerrors, 0, 1);
    sem_init(&sem_nerror_bytes, 0, 1);
    sem_init(&sem_shared_message, 0, 1);
    sem_init(&sem_contentLength, 0, 1);

    // Return a listening fd
    // assert(0);
    return listenfd;
}

static void send_response(int connfd, int hsize, int bsize, char* header, char* body) {

    sem_wait(&sem_nheaders);
    NHEADERS += hsize;
    sem_post(&sem_nheaders);

    sem_wait(&sem_nbodys);
    NBODYS += bsize;
    sem_post(&sem_nbodys);

    send_fully(connfd, header, hsize, 0);
    send_fully(connfd, body, bsize, 0);

    sem_wait(&sem_nrequests);
    NREQUESTS += 1;
    sem_post(&sem_nrequests);
}

static void handle_ping(int connfd) {
    int HSIZE = 0;
    char header[HMAX + 1];

    int BSIZE = 0;
    char body[BMAX + 1];

    HSIZE = snprintf(header, HMAX, OK200, 4);
    BSIZE = strlen(ping_body);

    memcpy(body, ping_body, BSIZE);
    send_response(connfd, HSIZE, BSIZE, header, body);
    close(connfd);
}

static void handle_request(int connfd) {
    char request[RMAX];
    int RSIZE = 0;

    RSIZE = recv_http_request(connfd, request, RMAX-1, 0);
    request[RSIZE] = '\0';

    char file_contents[1024];
    char serverResponse[1024];
    char serverResponse2[1024];
    char* getFileRequest;
    char savedMessage[RMAX] = "<empty>";
    int getLength = 7;

    char* statistics = "Requests: %d\nHeader bytes: %d\nBody bytes: %d\nErrors: %d\nError bytes: %d";

    int HSIZE = 0;
    int BSIZE = 0;

    if (RSIZE == 0) {
        return;
    }

    if ( strstr(request, "GET") != NULL ) {

        if ( !strncmp(request, ping_request, strlen(ping_request)) ) {
            handle_ping(connfd);
            return;
        } else if ( strstr(request, "/echo") != NULL ) {
            // If client requests /echo, send back their message (variable length);
            char* startOfBody = strstr(request, "\r\n"); // This will bring me to the "\r" after "HTTP/1.1"
            startOfBody += 2; // startOfBody will now locate to the start of the message.
            
            char* endOfBody = strstr(startOfBody, "\r\n\r\n");
            if ( endOfBody != NULL ) {
                *endOfBody = '\0';
            }
            
            getLength = strlen(startOfBody);
            if (getLength > 1024) {
                startOfBody[1025] = '\0';
            }
            getLength = strlen(startOfBody);

            int mSize;
            if ( startOfBody[getLength - 2] == '\n' ) {
                mSize = snprintf(serverResponse, 1024, message, getLength - 1);
            } else {
                mSize = snprintf(serverResponse, 1024, message, getLength);
            }
            sem_wait(&sem_nheaders);
            NHEADERS += mSize;
            sem_post(&sem_nheaders);
            HSIZE = mSize;
            send_fully(connfd, serverResponse, mSize, 0);

            if ( startOfBody[getLength - 1] == '\n' ) {
                sem_wait(&sem_nbodys);
                NBODYS += getLength - 1;
                sem_post(&sem_nbodys);

                BSIZE = getLength - 1;
                send_fully(connfd, startOfBody, getLength - 1, 0);
            } else {
                sem_wait(&sem_nbodys);
                NBODYS += getLength;
                sem_post(&sem_nbodys);

                BSIZE = getLength;
                send_fully(connfd, startOfBody, getLength, 0);
            }

            // Add 1 to num_requests if request ran successfully
            sem_wait(&sem_nrequests);
            NREQUESTS++;
            sem_post(&sem_nrequests);
        } else if (strstr(request, "/read") != NULL) {

            int mSize;
            if ( strcmp(shared_saved_message, "<empty>") == 0 ) {
                // If savedMessage is empty
                mSize = snprintf(serverResponse, 1024, message, 7);
                
                sem_wait(&sem_nheaders);
                NHEADERS += mSize;
                sem_post(&sem_nheaders);

                send_fully(connfd, serverResponse, mSize, 0); // Send header

                sem_wait(&sem_nbodys);
                NBODYS += 7;
                sem_post(&sem_nbodys);

                send_fully(connfd, savedMessage, 7, 0); // Send body
            } else {
                // If savedMessage is filled with a string
                mSize = snprintf(serverResponse, 1024, message, contentLength);

                sem_wait(&sem_nheaders);
                NHEADERS += mSize;
                sem_post(&sem_nheaders);

                send_fully(connfd, serverResponse, mSize, 0); // Send header
                
                sem_wait(&sem_nbodys);
                NBODYS += contentLength;
                sem_post(&sem_nbodys);

                send_fully(connfd, shared_saved_message, contentLength, 0); // Send body
            }

            // Add 1 to num_requests if request ran successfully
            sem_wait(&sem_nrequests);
            NREQUESTS++;
            sem_post(&sem_nrequests);

        } else if (strstr(request, "/stats") != NULL) {
            // Send variable 'statistics' over to the client
            // Data for statistics is in the order of: requests, header bytes, body bytes, errors, and error bytes.

            // Make the body first so we can get its size, then make the header
            int bodySize = snprintf(serverResponse2, 1024, statistics, NREQUESTS, NHEADERS, NBODYS, NERRORS, NERROR_BYTES);
            getLength = bodySize;
            int headerSize = snprintf(serverResponse, 1024, message, getLength);

            send_fully(connfd, serverResponse, headerSize, 0); // Send off the header first
            send_fully(connfd, serverResponse2, bodySize, 0); // Send off the body next

            sem_wait(&sem_nheaders);
            NHEADERS += headerSize;
            sem_post(&sem_nheaders);
            
            sem_wait(&sem_nbodys);
            NBODYS += bodySize;
            sem_post(&sem_nbodys);

            // Add 1 to num_requests if request ran successfully
            sem_wait(&sem_nrequests);
            NREQUESTS++;
            sem_post(&sem_nrequests);

        } else if (strstr(request, "/") != NULL) {

            // Request file on disk
            // Get the file request from the string
            char* startOfString = strstr(request, "/");
            char* endOfString = strstr(startOfString, " ");
            *endOfString = '\0';
            getFileRequest = startOfString + 1;

            // Open the string
            int html_fd = open(getFileRequest, O_RDONLY, 0);

            // Check if file exists,
            if (html_fd != -1) {
                // File is found

                // Read in the string and store in the buffer
                int file_content_size = get_size(html_fd);

                int mSize = snprintf(serverResponse, 1024, message, file_content_size);  // Send header

                sem_wait(&sem_nheaders);
                NHEADERS += mSize;
                sem_post(&sem_nheaders);

                sem_wait(&sem_nbodys);
                NBODYS += file_content_size;
                sem_post(&sem_nbodys);

                send_fully(connfd, serverResponse, mSize, 0);
                
                int fileLength = read(html_fd, file_contents, 1024);
                while ( fileLength > 0 ) {  // Send the body
                    send_fully(connfd, file_contents, fileLength, 0);
                    fileLength = read(html_fd, file_contents, 1024);
                }
                
                close(html_fd);

                // Add 1 to num_requests if request ran successfully
                sem_wait(&sem_nrequests);
                NREQUESTS++;
                sem_post(&sem_nrequests);
            } else {
                // File is not found
                sem_wait(&sem_nerrors);
                NERRORS++;
                sem_post(&sem_nerrors);

                sem_wait(&sem_nerror_bytes);
                NERROR_BYTES += strlen(err404_response);
                sem_post(&sem_nerror_bytes);

                send_fully(connfd, err404_response, strlen(err404_response), 0);
            }

        } else {
             // Bad request
             sem_wait(&sem_nerrors);
             NERRORS++;
             sem_post(&sem_nerrors);
             
            sem_wait(&sem_nerror_bytes);
            NERROR_BYTES += strlen(err404_response);
            sem_post(&sem_nerror_bytes);
            send_fully(connfd, err400_response, strlen(err400_response), 0);
        }

    } else if ( strstr(request, "POST") != NULL ) {
        // POST shoulve have /write

        char* formatTest = strstr(request, "POST");
        formatTest += 4;
        if ( *formatTest != ' ' ) {
            send_fully(connfd, err404_response, strlen(err404_response), 0);
            return;
        }

        if (strstr(request, "/write") != NULL) {

            // Process of getting the content length
            char* getContentLength = strstr(request, "Content-Length");
            getContentLength += 16;
            char* endOfContentLength = strstr(getContentLength, "\r\n");
            *endOfContentLength = '\0';
            getLength = atoi(getContentLength);  // We now have the Content-Length value
            *endOfContentLength = '\r'; // Remove the null character so we can continue traversing through the string


            // Now we create the body of the response
            char* getMessage = strstr(request, "\r\n\r\n");
            getMessage += 4; // We are at the start of the actual message

            if ( getLength > 1024 ) {
                getLength = 1024;
            }
            
            memset(savedMessage, 0, RMAX);
            memcpy(savedMessage, getMessage, getLength);

            sem_wait(&sem);
            memset(shared_saved_message, 0, RMAX);
            memcpy(shared_saved_message, savedMessage, getLength);
            sem_post(&sem);

            sem_wait(&sem_contentLength);
            contentLength = getLength;
            sem_post(&sem_contentLength);

            int mSize = snprintf(serverResponse, 1024, message, getLength);

            sem_wait(&sem_nheaders);
            NHEADERS += mSize;
            sem_post(&sem_nheaders);

            send_fully(connfd, serverResponse, mSize, 0); // Send off the header first

            sem_wait(&sem_nbodys);
            NBODYS += getLength;
            sem_post(&sem_nbodys);

            send_fully(connfd, savedMessage, getLength, 0); // Send off the body

            // Add 1 to num_requests if request ran successfully
            sem_wait(&sem_nrequests);
            NREQUESTS++;
            sem_post(&sem_nrequests);
            
        } else {
            // Bad request
            sem_wait(&sem_nerrors);
            NERRORS++;
            sem_post(&sem_nerrors);
            
            sem_wait(&sem_nerror_bytes);
            NERROR_BYTES += strlen(err404_response);
            sem_post(&sem_nerror_bytes);
            
            send_fully(connfd, err400_response, strlen(err400_response), 0);
        }
    }

    close(connfd); // DO NOT DELETE THIS
}

void * handle_request_helper(void * arg) {
    long temp = (long) arg;
    int connfd = temp;

    handle_request(connfd);

    return NULL;
}

void accept_client(int server_socket)
{
    // This will accept & handle client.
    // Parameter server socket accepts the listening fd from create_server_socket()

    // Accept the client
    static struct sockaddr_in client;
    static socklen_t client_size;

    memset(&client, 0, sizeof(client));
    memset(&client_size, 0, sizeof(client_size));

    int connfd = Accept(server_socket, (struct sockaddr*)&client, &client_size);

    pthread_t thread;

    if ( connfd != -1 ) {
        long temp = (long) connfd;
        hw5_pthread_create(&thread, NULL, handle_request_helper, (void *) temp);
    }  
}

int Socket(int namespace, int style, int protocol) {
    int sockfd = socket(namespace, style, protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    return sockfd;
}

void Bind(int sockfd, struct sockaddr * server, socklen_t length) {
    if ( bind(sockfd, server, length) < 0 ) {
        perror("listen");
        exit(1);
    }
}

void Listen(int sockfd, int qlen) {
    if ( listen(sockfd, qlen) < 0 ) {
        perror("listen");
        exit(1);
    }
}

int Accept(int sockfd, struct sockaddr * addr, socklen_t * length_ptr) {
    int newfd = accept(sockfd, addr, length_ptr);
    if (newfd < 0) {
        perror("accept");
        exit(1);
    }

    return newfd;
}