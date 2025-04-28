#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#define LONGEST_SERVER 253
#define LARGE_MAIL_NUMBER_LENGTH 10
#define BUFFER_SIZE 2048



int logging_on(char* serverName, char* userName, char*password, char* folder, char* numMsg);
void retrieve_raw(int socketfd, int total_char_num);
void read_from_socket(int socketfd, size_t size, char* read_buffer, char* command);
void retrieve(int socketfd, char* numMsg);
void write_to_socket(int socketfd, char* info);
int createSocket_connect(char* serverName);
int read_first_line(int socketfd);
int fetch_last_message(char* inbox_info);
void read_per_line(int socketfd, char* line);
void request_print_information(int socketfd, char* numMsg, char* infoType, char* tag, char* mode);
void parse(int socketfd, char* numMsg);
void mime(int socketfd, char* numMsg);
void getBoundrayValue(char* boundary_value, char* remove_prefix);
void str_to_lower(char* string);
void list(int socketfd, char* numMsg);
void request_sequence_info(char* sequence, int socketfd);
void getFirstLine(char* str, char* firstline);
int is_valid_messageNum(char* numMsg);
void identify_input_error(char* folder, char* password, char* userName, char* numMsg, char* serverName, char* command);


int main(int argc, char** argv){
    char* folder = "INBOX"; 
    char*password = NULL; 
    char*userName = NULL;
    char* numMsg = NULL;
    char* serverName = NULL;
    char* command = NULL;

    if(argc < 6){
        fprintf(stderr, "Not enough of number of argument\n"); 
        exit(1);
    }
    int is_allocated_Msg = 0;
    // retrieve information
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "-u") == 0){
            if (userName == NULL) {
                if ((i + 1) == (argc - 1) || (i + 1) == (argc - 2)) exit(1);
                userName = argv[i+1];
            }
        }
        if (strcmp(argv[i], "-p") == 0){
            if (password == NULL){
                if ((i + 1) == (argc - 1) || (i + 1) == (argc - 2)) exit(1);
                password = argv[i+1];
            }
                
        }
        if (strcmp(argv[i], "-f") == 0){
            if (strcmp(folder, "INBOX") == 0){
                if ((i + 1) == (argc - 1) || (i + 1) == (argc - 2)) exit(1);
                folder = argv[i+1];
            }
                
        }
        if (strcmp(argv[i], "-n") == 0){
            if (numMsg == NULL){
                if ((i + 1) == (argc - 1) || (i + 1) == (argc - 2)) exit(1);
                numMsg = argv[i+1];
            }
                
        }
    }

    command = argv[argc-2];
    serverName = argv[argc-1];

    identify_input_error(folder, password, userName, numMsg, serverName, command);


    if(command == NULL || serverName == NULL) exit(1);

    if(numMsg == NULL) {
        numMsg = malloc(BUFFER_SIZE*sizeof(char));
        is_allocated_Msg = 1;
    }

    // loggin part
    int socketfd = logging_on(serverName, userName, password, folder, numMsg);

    // perform task
    if (strcmp(command, "retrieve") == 0){
        retrieve(socketfd, numMsg);
    }
    if (strcmp(command, "parse") == 0){
        parse(socketfd, numMsg);
    }
    if (strcmp(command, "mime") == 0){
        mime(socketfd, numMsg);
    }
    if (strcmp(command, "list") == 0){
        list(socketfd, numMsg);
    }

    if(is_allocated_Msg) free(numMsg);

    return 0;
}

// utils
/*function for identify errors on input*/
void identify_input_error(char* folder, char* password, char* userName, char* numMsg, char* serverName, char* command){
    if (password == NULL)
    {
        fprintf(stderr, "Lack of password\n");
        exit(1);
    }
    if (userName == NULL)
    {
        fprintf(stderr, "Lack of username\n");
        exit(1);
    } 

    // check serverName
    if (serverName != NULL)
    {
        if(strlen(serverName) > LONGEST_SERVER || strlen(serverName) <= 1){
            fprintf(stderr, "Invalid serverName\n");
            exit(2);
        }
    }

    // check numMsg
    if (numMsg != NULL)
    {
        if (strlen(numMsg) > LARGE_MAIL_NUMBER_LENGTH){
            fprintf(stderr, "Over size of message number\n");
            exit(1);
        }
        if (atoi(numMsg) <= 0){
            fprintf(stderr, "Negative size of message number\n");
            exit(1);
        }
        if (!is_valid_messageNum(numMsg)){
            exit(1);
        }
    }

    // check folder
    if (strcmp(folder, "INBOX") != 0)
    {
        if (strlen(folder) == 0 || strcmp(folder, " ") == 0) {
            fprintf(stderr, "Zero length or blank folder\n");
            exit(3);
        }  
    }
}

/*function for test whether it is valid message number*/
int is_valid_messageNum(char* numMsg){
    for(int i = 0; i < strlen(numMsg); i++){
        if(!isdigit(numMsg[i])) return 0;
    }
    return 1;
}

/* function for read the first line from server*/
int read_first_line(int socketfd){
    char first_line [1024];
    char c;
    int first_line_num = 0;
    int char_num;

    // read by char
    while(1){
        read(socketfd, &c, sizeof(char));
        first_line[first_line_num] = c;
        first_line_num++;
        
        // meet \n, break
        if (c == '\n') {
            first_line[first_line_num] = '\0';
            break;
        }  
    }

    // retrieve how many chars in total
    int result = sscanf(first_line, "%*[^{]{%d}", &char_num);
    
    if (result != 1){
        // check for Foler number error
        if (strstr(first_line, "No mailbox selected") != NULL)
        {
            printf("Folder not found\n");
            exit(3);
        }
        // check for login error
        else if (strstr(first_line, "Error in IMAP command received by server") != NULL)
        {
            printf("Login failure\n"); 
            exit(3);
        }
        // check for message error
        else if (strstr(first_line, "Invalid messageset"))
        {   
            printf("Message not found\n");
            exit(3);
        }
    } 

    return char_num;  
}

/* function read information from server*/
void read_from_socket(int socketfd, size_t size, char* read_buffer, char* command){
    ssize_t result = read(socketfd, read_buffer, size-1);
    if (result == 0) exit(3);
    read_buffer[result] = '\0';
}

/* function send information to server*/
void write_to_socket(int socketfd, char* info){
    ssize_t result = write(socketfd, info, strlen(info));
    if (result == -1) {
        printf("error in writen to server\n");
        exit(3);
    }
}

/*create socket and connect*/
int createSocket_connect(char* serverName){
    char* port = "143";
    int connfd, s;
    struct addrinfo hints, *serverContent, *rp;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    // firstly try to set up ipv6
    s = getaddrinfo(serverName, port, &hints, &serverContent);

    if (s != 0){
        // fall back to ipv4
        hints.ai_family = AF_INET;
        s = getaddrinfo(serverName, port, &hints, &serverContent);
    }

    if (s != 0){
        fprintf(stderr, "error in connect server\n");
        exit(2);
    }

    for (rp = serverContent; rp!=NULL; rp = rp->ai_next){
        connfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connfd == -1) continue;
        if (connect(connfd, rp->ai_addr, rp->ai_addrlen) != 1) break;

        close(connfd);
    }

    if (rp == NULL){
        fprintf(stderr, "failed connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(serverContent);
    return connfd;

}

/*function for read per line from server*/
void read_per_line(int socketfd, char* line){
    char c;
    int first_line_num = 0;
    while(1){
        read(socketfd, &c, sizeof(char));
        line[first_line_num] = c;
        first_line_num++;
        if (c == '\n') {
            line[first_line_num] = '\0';
            break;
        }  
    }
}

// part loggin on
int logging_on(char* serverName, char* userName, char*password, char* folder, char* numMsg){
    char buffer_read[BUFFER_SIZE];
    // create and connect
    int socketfd = createSocket_connect(serverName);
    // reading from socket
    read_from_socket(socketfd, BUFFER_SIZE, buffer_read, NULL);

    char temp_write[BUFFER_SIZE];
    snprintf(temp_write, BUFFER_SIZE, "TST1 LOGIN %s %s\r\n", userName, password);
    write_to_socket(socketfd, temp_write);
    // check login, read response from server
    read_from_socket(socketfd, BUFFER_SIZE, buffer_read, NULL);

    // tell the server to select folder
    snprintf(temp_write, BUFFER_SIZE, "TST2 SELECT \"%s\"\r\n", folder);
    write_to_socket(socketfd, temp_write);

    // get response from server
    read_from_socket(socketfd, BUFFER_SIZE, buffer_read, NULL);

    // if the number of message has not been specified
    if (strlen(numMsg) == 0)
    {   
        int last_message = fetch_last_message(buffer_read);
        sprintf(numMsg, "%d", last_message);
    }

    // printf("%s\n", buffer_read);
    return socketfd;
}

// part retrieve
int fetch_last_message(char* inbox_info){
    char* line;
    int last_numMsg;

    // get the line of existing message number
    line = strtok(inbox_info, "\n");
    line = strtok(NULL, "\n");
    line = strtok(NULL, "\n");
    // printf("line = %s\n", line);

    if (line != NULL) 
    {   
        
        // match the string
        if (sscanf(line, "* %d EXISTS", &last_numMsg) == 1)
        {   
            if(last_numMsg == 0){
                printf("Currently no messages in mailbox\n");
                exit(5);
            }
            return last_numMsg;
        }
        else
        {
            printf("find last message error\n");
            printf("info = %s\n", line);
        }
    }
    return 0;
}

/*function for retrieve all information from server, given a total number of char*/
void retrieve_raw(int socketfd, int total_char_num){
    char c;
    while(total_char_num > 0){
        read(socketfd, &c, sizeof(char));
        printf("%c", c);
        total_char_num -= 1;
    }
}

/*task retrieve*/
void retrieve(int socketfd, char* numMsg){
    char temp_write[BUFFER_SIZE];

    snprintf(temp_write, BUFFER_SIZE, "TST3 FETCH %s BODY.PEEK[]\r\n", numMsg);

    // write to server
    write_to_socket(socketfd, temp_write);

    // read response from server
    // read first line to retrieve the total char number in body
    int total_char_num = read_first_line(socketfd);

    retrieve_raw(socketfd, total_char_num);

    exit(0);
}

// part parse
/*function for request header fileds and print those information*/
void request_print_information(int socketfd, char* numMsg, char* infoType, char* tag, char* mode){
    char temp_write [BUFFER_SIZE];
    char buffer_read [BUFFER_SIZE];
    char firstLine [BUFFER_SIZE];
    int char_num;
    if (strcmp(infoType, "FROM") == 0) infoType = "From";
    if (strcmp(infoType, "TO") == 0) infoType = "To";
    
    // write command to server
    snprintf(temp_write, BUFFER_SIZE, "%s FETCH %s BODY.PEEK[HEADER.FIELDS (%s)]\r\n",tag, numMsg, infoType);
    write_to_socket(socketfd, temp_write);
    read_from_socket(socketfd, BUFFER_SIZE, buffer_read, NULL);

    // find first line
    char buffer_copy [BUFFER_SIZE];
    strcpy(buffer_copy, buffer_read);
    getFirstLine(buffer_read, firstLine);

    // find char number
    sscanf(firstLine, "%*[^{]{%d}", &char_num);

    // print the information type for different mode
    if (strcmp(mode, "parse") == 0) printf("%s", infoType);
    if (strcmp(mode, "list") == 0) printf("%s", numMsg);

    // if there has information
    if (char_num > 2){

        for (int i = strlen(firstLine) + strlen(infoType) + 1; i < strlen(firstLine) + char_num - 3; i++){
            if(buffer_copy[i] == '\r') continue;

            else if(buffer_copy[i] == '\n') continue;

            else printf("%c", buffer_copy[i]);
        }
        printf("\n");
    }else{
        // no information
        if(strcmp(infoType, "Subject") == 0) printf(": <No subject>\n");
        else printf(":\n");
    }
}



/*task parse*/
void parse(int socketfd, char* numMsg){
    // request for FROM
    request_print_information(socketfd, numMsg, "FROM", "TST5", "parse");
    // request for TO
    request_print_information(socketfd, numMsg, "TO", "TST6", "parse");
    // // // request for Date
    request_print_information(socketfd, numMsg, "Date", "TST7", "parse");
    // request for Subject
    request_print_information(socketfd, numMsg, "Subject", "TST8", "parse");
}


// part mime
/*function for get boundary value*/
void getBoundrayValue(char* boundary_value, char* remove_prefix){

    // copy boundary value to char array
    int double_quotes_count = 0;
    int boundary_num = 2;
    for (int i = 0; i < strlen(remove_prefix); i++){
        // check whether escaped from double quotes or \n
        if(double_quotes_count == 2 || remove_prefix[i] == '\r' || remove_prefix[i] == '\n') break;
        
        // normally copy char to the string
        if(remove_prefix[i] != '\"' && remove_prefix[i] != '\''){
            boundary_value[boundary_num] = remove_prefix[i];
            boundary_num++;   
        } else{
            if (remove_prefix[i] == '"') double_quotes_count ++;
        }
    }
    boundary_value[boundary_num] = '\0';
}

/*task mime*/
void mime(int socketfd, char* numMsg){
    char temp_write[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    char* boundary_prefix = " boundary=";
    char boundary_value [BUFFER_SIZE] = {0};
    boundary_value[0] = '-';
    boundary_value[1] = '-';
    char end_indicator [BUFFER_SIZE] = {0};
    char* contextType_prefix = "content-type: text/plain";
    char* charset = "charset=utf-8";
    char* encoding1 = "content-transfer-encoding: quoted-printable";
    char* encoding2 = "content-transfer-encoding: 7bit";
    char* encoding3 = "content-transfer-encoding: 8bit";
    int counts = 0;
    char prev_line [BUFFER_SIZE];
    int ignore_prev = 0;


    // write message to server
    snprintf(temp_write, BUFFER_SIZE, "TST9 FETCH %s BODY.PEEK[]\r\n", numMsg);
    write_to_socket(socketfd, temp_write);

    // get total number of char
    int total_num = read_first_line(socketfd);

    // indicator to check whether body is found
    int bodyFound = 0;
    while(1){
        // message to the end but still did not match information, exit
        if (counts == total_num && !bodyFound){
            printf("No matched information\n");
            exit(4);
        }
        counts++;

        // read information per line
        read_per_line(socketfd, line);
        
        // check whether reach the end of body
        if(strncasecmp(line, boundary_value, strlen(boundary_value)) == 0 && bodyFound == 1) break;
        
        if(strncasecmp(line, end_indicator, strlen(end_indicator)) == 0 && bodyFound == 1) break;

        // first time get boundary value
        if (strncasecmp(line, boundary_prefix, strlen(boundary_prefix)) == 0){
            // remove prefix
            char* remove_prefix = line + strlen(boundary_prefix);
            getBoundrayValue(boundary_value, remove_prefix);
        }

        // find boundary value header, prepare to printing body
        if (strncasecmp(line, boundary_value, strlen(boundary_value)) == 0){
            int contentTypeFound = 0;
            int charsetFound= 0;
            int contentTransFound = 0;
            int content_count = 3;

            while(content_count > 0){
                if (counts == total_num){
                    printf("No matched information\n");
                    exit(4);
                }
                counts++;
                read_per_line(socketfd, line);

                // find contextType
                if (strncasecmp(line, contextType_prefix, strlen(contextType_prefix)) == 0) 
                {   
                    contentTypeFound = 1;
                    content_count --;
                    // skip ; for con text type
                    char* remove_type = line + strlen(contextType_prefix) + 2;

                    // check for charset
                    if (strncasecmp(remove_type, charset, strlen(charset)) == 0)
                    {   
                        charsetFound = 1;
                        content_count --;
                    }
                }
                
                // whether charset is folded
                if(!charsetFound){
                    // convert string to lower case
                    str_to_lower(line);
                    if (strstr(line, charset) == NULL)
                        {
                            charsetFound = 1;
                            content_count --;
                        }
                }

                // find content encoding
                if (strncasecmp(line, encoding1, strlen(encoding1)) == 0 || strncasecmp(line, encoding2, strlen(encoding2)) == 0 
                || strncasecmp(line, encoding3, strlen(encoding3)) == 0)
                {   
                    contentTransFound = 1;
                    content_count --;
                }
            }

            // if 3 header were found, prepare to print body
            if (contentTypeFound && charsetFound && contentTransFound){
                bodyFound = 1;
                ignore_prev = 1;
                // modified boundary value to the indicator of end
                int currentLen = strlen(boundary_value);
                strcpy(end_indicator, boundary_value);
                end_indicator[currentLen] = '-';
                end_indicator[currentLen + 1] = '-';
                end_indicator[currentLen + 2] = '\r';
                end_indicator[currentLen + 3] = '\n';
                end_indicator[currentLen + 4] = '\0';
                // skip one line
                read_per_line(socketfd, line);
                continue;
            } else{
                continue;
            }

        }
        
        // body found, print body, but ignore the last line
        if (bodyFound && !ignore_prev) {printf("%s", prev_line);}
        strcpy(prev_line, line);
        ignore_prev = 0;
        
    }   
}

/*function for convert string to lower*/
void str_to_lower(char* string){
    if(string == NULL) return;

    while(*string){
        *string = tolower((unsigned char) *string);
        string++;
    }    
}

// list part
/*task list*/
void list(int socketfd, char* numMsg){
    char buffer_write [BUFFER_SIZE];
    char buffer_read [BUFFER_SIZE];
    char sequence [BUFFER_SIZE];

    // write and read from server
    snprintf(buffer_write, BUFFER_SIZE, "TST10 SEARCH ALL\r\n");
    write_to_socket(socketfd, buffer_write);
    read_per_line(socketfd, buffer_read);

    // extract sequence from message
    sscanf(buffer_read, "* SEARCH %[^\r\n]", sequence);

    read_per_line(socketfd, buffer_read);

    // start to request Subject information from server
    request_sequence_info(sequence, socketfd);
}

/*function for request subject information from server given a sequence */
void request_sequence_info(char* sequence, int socketfd){
    char tag[BUFFER_SIZE];
    
    // seperate by " "
    char* token = strtok(sequence, " ");

    while(token != NULL){
        // produce new tag
        sprintf(tag, "L%s", token);

        // request subject information
        request_print_information(socketfd, token, "Subject", tag, "list");

        token = strtok(NULL, " ");
    }
}

/*function for get the first line from a string*/
void getFirstLine(char* str, char* firstline){
    int firstnum = 0;
    for(int i = 0; i < strlen(str); i++){
        if (str[i] == '\n') break;
        firstline[firstnum] = str[i];
        firstnum++;
    }
    firstline[firstnum] = '\0';
}
