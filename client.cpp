/*
    Author: Osvaldo 

    This a client program that will wait connect with server and sent it requests for
    current low and high prices for cars of a particular make and model

    The program ends when user enters "quit" or nothing

    Date completed: 7/19/2019
*/

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

using namespace std;

//Function prototypes
void error(char *msg);

const int customrBufferSize = 250;
const string specialCharacter= "&"; // Separates low price and high price in char transfer message from server
const char EXITLOOPKEYWORD[] = "quit";

int main(){
    int sockfd, portno, n;
    sockaddr_in serv_addr;
    hostent *server;
    char transferMessage[customrBufferSize];
    char myMessage[customrBufferSize];
    string inputString;
    char hostInput[customrBufferSize]; // stores hotst name


    char *errorMsg; //stores the message to be thrown
    errorMsg = new char[customrBufferSize];
    cout << "Enter port number: ";
    cin >> portno;
    cout << endl;

    while (cin.fail()){
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Enter VALID port number: ";
        cin >> portno;
        cout << endl;
    }

    cout << "Enter host: ";
    cin >> hostInput;
    cout << endl;
    cin.ignore();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        strcpy(errorMsg,"ERROR opening socket");
        error(errorMsg);
    }
    
    server = gethostbyname(hostInput);
    if (server == NULL){
        strcpy(errorMsg,"ERROR, no such host");
        error(errorMsg);
    }

    memset((char*) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char*) server->h_addr_list, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
        strcpy(errorMsg,"ERROR connecting");
        error(errorMsg);
    }

    cout <<"Enter a car name and model: ";
    getline(cin, inputString);
    cout << endl;
    strcpy(myMessage, inputString.c_str());
    //cout << "Message to be sent-" << myMessage << "." << endl;
    //cout << "Length of stirng: " << strlen(myMessage) << endl;

    while(strcmp(myMessage, EXITLOOPKEYWORD) != 0 && strlen(myMessage) > 0){
        n = write(sockfd, myMessage, sizeof(myMessage));
        if (n < 0){
            strcpy(errorMsg,"ERROR writing to socket");
            error(errorMsg);
        }
        memset(transferMessage, '\0', customrBufferSize);
        n = read(sockfd, transferMessage, customrBufferSize);
        if (n < 0){
            strcpy(errorMsg,"ERROR reading from socket");
            error(errorMsg);
        }

        if (transferMessage[0] == '-'){
            cout <<"That make and model is not in the database." << endl << endl;
        }
        else{
            string reply = transferMessage;
            int index = reply.find_first_of("&");
            string reply2 = reply.substr(index + 1, 1000); // extract the high price from message
            reply.erase(index);
            cout << "The low price for that car is $" << reply<< "." << endl;
            cout <<"The High price for taht car is $" << reply2 << "." << endl << endl;
        }
      
        //std::cout << transferMessage << std::endl;
        memset(myMessage, '\0', customrBufferSize);

        std::cout <<"Enter a car name and model: ";
        getline(cin, inputString);
        cout << endl;
        strcpy(myMessage, inputString.c_str());
        //cout << "Message to be sent-" << myMessage << "." << endl;
        //cout << "Length of string: " << strlen(myMessage) << endl;


        // reconnect to server 
        if (strcmp(myMessage, EXITLOOPKEYWORD) != 0 && strlen(myMessage) > 0){
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0){
                strcpy(errorMsg,"ERROR opening socket");
                error(errorMsg);
            }
    
            server = gethostbyname(hostInput);
            if (server == NULL){
                strcpy(errorMsg,"ERROR, no such host");
                error(errorMsg);
            }

            memset((char*) &serv_addr, '\0', sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            memcpy((char*) server->h_addr_list, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(portno);

            if((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
                strcpy(errorMsg,"ERROR connecting");
                error(errorMsg);
            }
        }

    }

    return 0;
}

void error(char *msg){
    perror(msg);
    exit(0);
}
