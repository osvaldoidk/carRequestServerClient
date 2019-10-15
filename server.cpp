/*
    Author: Osvaldo

    This a server program that will wait for connection requests from client, read a car make
    and model and reply with the current low and high prices for that car

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
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h> 
#include <string>

using namespace std;

struct carInformation{
    string name;
    string lowPrice;
    string highPrice;
};

//Function prototypes
void error(char *msg);
void extractPrice(string &lineInput, string &price);
void initializeCarList(vector<carInformation> &carList);
void displayCarList(vector<carInformation> &carList);
void deleteTrailingLeadingSpaces(string& inputS);
int searchCarPrices(vector<carInformation> &carList, char userCarNameInput[], int size);
bool isNumber(string s);

// global variables
const int BUFFERSIZE = 250;

int main(){
    int sockfd, newsockfd, portno,n;
    socklen_t cliclen;
    char *errorMsg; //stores the message to be thrown
    errorMsg = new char[BUFFERSIZE];

    char serverMessage[BUFFERSIZE];
    char clientMessage[BUFFERSIZE]; 
    vector<carInformation> carList;
    int index; // index location of car found in search

    // Recieve port number
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
    
    sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        strcpy(errorMsg,"ERROR opening socket");
        error(errorMsg);
    }
    memset((char*) &serv_addr, '\0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if((bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
        strcpy(errorMsg,"ERROR on binding");
        error(errorMsg);
    }

    initializeCarList(carList);
    //displayCarList(carList);
    
    while(1){
        
        listen(sockfd, 5);
        cliclen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cliclen);
        if (newsockfd < 0){
            strcpy(errorMsg,"ERROR on accept");
            error(errorMsg);
        } // end if
       
        memset(serverMessage, '\0', BUFFERSIZE);
      
        n = read(newsockfd, clientMessage, sizeof(clientMessage));
        if (n < 0){
            strcpy(errorMsg,"ERROR reading from socket");
            error(errorMsg);
        }

        index = searchCarPrices(carList, clientMessage, BUFFERSIZE);

        // car found
        if (index > -1){

            string reply = carList[index].lowPrice;
            reply.append(1, '&');
            reply.append(carList[index].highPrice);
            strcpy(serverMessage, reply.c_str());
            
            n=write(newsockfd, serverMessage, BUFFERSIZE);
            if (n < 0){
                strcpy(errorMsg,"ERROR writing to socket" );
                error(errorMsg);
            } 
        }
        // car not foudnd
        else{
            string reply = "-1";
            reply.append(1, '&');
            reply.append("1");
            strcpy(serverMessage, reply.c_str());
            n=write(newsockfd, serverMessage, BUFFERSIZE);
            if (n < 0){
                strcpy(errorMsg,"ERROR writing to socket" );
                error(errorMsg);
            }
        } // end if

    } // end while loop

    return 0;
}

void error(char *msg){
    perror(msg);
    exit(1);
}

void initializeCarList(vector<carInformation> &carList){
    fstream infile;
    char filename[100];
    do{
    cout << "Enter the input file name: ";
    cin >> filename;
    //cout << endl << filename << endl;
    infile.open(filename);

    if (!infile.is_open()){
        cout << endl <<"UNABLE to open file, try different file path!" << endl << endl;
    }
    }while(!infile.is_open());
   
    while(!infile.eof()){
        string lineInput;
        carInformation carRecord;  
        getline(infile, lineInput); 
        //cout << endl << "LINE: "<< lineInput << endl;
        // Reverse str[beign..end] 
        reverse(lineInput.begin(), lineInput.end()); 
    
        // Extracting prices
        extractPrice(lineInput, carRecord.highPrice);
        extractPrice(lineInput, carRecord.lowPrice);
        carRecord.name = lineInput;

        reverse(carRecord.name.begin(), carRecord.name.end());
        //cout << endl << "CAR NAME: " << carRecord.name << endl;

        reverse(carRecord.highPrice.begin(), carRecord.highPrice.end());
         //cout << "CAR L PRICE: " << carRecord.lowPrice << endl;
        reverse(carRecord.lowPrice.begin(), carRecord.lowPrice.end());
         //cout << endl << "CAR H PRICE: " << carRecord.highPrice << endl;

        carList.push_back(carRecord);
        //cout << "--------------" << endl << endl;
    }
    infile.close();
}

void displayCarList(vector<carInformation> &carList){
    cout << "CAR LIST" << endl << endl;
    for (int i = 0; i < carList.size(); i++){
        cout << carList[i].name << "---" << carList[i].lowPrice << "---" << carList[i].highPrice << endl;
    }
}

void extractPrice(string &lineInput, string &price){
    // extract price
    int i = 0;
    char c = lineInput[i];
    while (!(isspace(lineInput[i]))){
        price.append(1, lineInput[i]);
        i++;
    }
    // delete extracted price from line
    lineInput.erase(0, price.length());
    i = 0;

    //delete trailing spaces from line
    while(isspace(lineInput[0])){
        lineInput.erase(0, 1);
    }
}

int searchCarPrices(vector<carInformation> &carList, char userCarNameInput[], int size){
    int carFound;
    string inputString = userCarNameInput;
    deleteTrailingLeadingSpaces(inputString);
    strcpy(userCarNameInput, inputString.c_str());
    char carInDatabase[100]; // stores car make and model from database converted from strng to c-array

    for (int count = 0; count < carList.size(); count++){
        inputString = carList[count].name;
        strcpy(carInDatabase, inputString.c_str());
        carFound = strcmp(userCarNameInput, carInDatabase);
        //cout << endl << "Name Buffer value---" << userCarNameInput << " | Databasecar---" << carInDatabase << endl << endl;
        if (carFound == 0){
            return count;
            break;
        } // end if
    } //end for loop

    if (carFound != 0){
       return -1;
    } // end if

}

void deleteTrailingLeadingSpaces(string& inputS){
    // delete trailing spaces
    while(isspace(inputS[0])){
        inputS.erase(0, 1);
    }

    // delete leading spaces
    while(isspace(inputS[inputS.length() - 1])){
        inputS.erase(inputS.length() - 1, 1);
    }
}

bool isNumber(string s) 
{ 
    for (int i = 0; i < s.length(); i++) 
        if (isdigit(s[i]) == false) 
            return false; 
  
    return true; 
} 