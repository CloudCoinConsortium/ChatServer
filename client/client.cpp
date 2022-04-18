// tcp chatroom client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include "pg1lib.h"

// macros
#define MAX_LINE 2048

using namespace std;

// globals
int sockfd;
bool PSEND;
vector<string> onlineUsers;
string USERNAME;
bool ABORT_DIR_MSG;
string PUB_KEY_FOR_MSG;

// proototypes
void *handle_messages(void*);
void handle_login(string);
int recvWithCheck(string&);
void send_string(string);
void promptUser();
void publicMessage();
void directMessage();


void displayPrompt(){
    cout << "Please enter a command: P (Public message), D (Direct message), Q (Quit)" << endl;
    cout << "> ";
}

void promptUser(){
    bool quit = false;
    string usrInput;
    cout << "\nSuccessfully Joined Chat" << endl;
    while (!quit){
        displayPrompt();
        cin >> usrInput;
        if (usrInput == "P"){
            publicMessage();
        }
        else if (usrInput == "D"){
            directMessage();
        }
        else if (usrInput == "Q"){
            send_string(usrInput+USERNAME);
            quit = true;
        }
        else {
            cout << "Sorry, we do not understand your slection. Please choose: 'P', 'D', or 'Q'\n" << endl;
        }
    
    }
}
void directMessage(){
    send_string("dirMsg");
    string usrToMsg, msg;
    // prompt comes from other thread
    ABORT_DIR_MSG = false;
    cin >> usrToMsg;
    send_string(usrToMsg);
    if (ABORT_DIR_MSG){
        return;
    }
    cout << "Message: ";
    cin >> msg;
    char *buf = new char[MAX_LINE];
    char * pubKeyCStr = new char[PUB_KEY_FOR_MSG.length() + 1];
    char * msgToSendCStr = new char[msg.length()+1];
    strcpy(pubKeyCStr, PUB_KEY_FOR_MSG.c_str());
    strcpy(msgToSendCStr, msg.c_str());
    buf = encrypt(msgToSendCStr, pubKeyCStr);
    msg = buf;            
    delete [] pubKeyCStr;
    delete [] msgToSendCStr;
    delete [] buf;
    send_string(msg);
    send_string(USERNAME);

}

void publicMessage(){
    send_string("pubMsg");
    // no error: this is the ack from server
    // in other words we are good to transmit if no error from send_string("pubMsg")
    string msg;
    cout << "Enter the public message: ";
    cin >> msg;
    send_string(msg);
    PSEND = false;
    while(!PSEND){
        // spin until we get confirmation 
    }
}


void *handle_messages(void*){
    string message;
    int numBytesRec;
    
    while(1) {
        string message;
        int numBytesRec;
        numBytesRec = recvWithCheck(message);
        if (message.substr(0,2) == "DE"){ // encrypted private
            string frmUser;
            recvWithCheck(frmUser);
            message = message.substr(2);
            char * buf;
            char * msg;
            msg = new char[MAX_LINE];
            buf = new char[MAX_LINE];
            strcpy(msg, message.c_str());
            buf = decrypt(msg);
            message = buf; 
            cout << "\n*** Incoming private message from " << frmUser << " ***: ";
            cout << message << endl;
            cout << "> ";
            fflush(stdout);
        }
        else if (message[0] == 'D'){
            message = message.substr(1);
            cout << "\n*** Incoming public message ***: ";
            cout << message << endl;
            cout << "> ";
            fflush(stdout);
        }

        else if (message[0] == 'C'){  // command message
            if (message.substr(1) == "P:OK"){
                PSEND = true;
            }
            else if (message.substr(1,4) == "usrs"){
                string names = message.substr(5);
                stringstream ss(names);
                char c;
                string name = "";
                while (ss >> c){
                    name = name + c;
                    if (ss.peek() == ';'){
                        onlineUsers.push_back(name);
                        name = "";
                        ss.ignore();
                    }
                }
                cout << "Peers online: " << endl;
                for (auto it = onlineUsers.begin(); it != onlineUsers.end(); it++){
                    if (*it == USERNAME){
                        continue;
                    }
                    cout << " " << *it << endl;;
                }
                cout << endl;
                cout << "Peer to message: ";
                fflush(stdout);
            }
            else if (message.substr(1,3) == "key"){
                //cout << "WE GOT THE KEY" << endl;
                PUB_KEY_FOR_MSG = message.substr(4);
            }
            else if (message.substr(1,3) == "DNE") {
                cout << "User does not exist: Abort" << endl; 
                ABORT_DIR_MSG = true;
            }
            else if (message.substr(1, 2) == "NO"){
                cout << "User is not online, message not sent" << endl;
            }
            else if (message.substr(1,4) == "SENT"){
                // cout << "MESSAGE SENT" << endl;
            }
        }
        else {
            cout << "DONT RECOGNIZE THIS: " << message << endl;
        }                   
    }
}

void handle_login(string username){
    send_string(username);
    string response, password;
    recvWithCheck(response);
    //cout << "server res: " << response << endl;
    if (response == "existUser"){
        cout << "Password: ";
        cin >> password;
        send_string(password);
        recvWithCheck(response);
        while (response != "OK"){
            password = "";
            cout << "Wrong password, please try again" << endl;
            cout << "pass: ";
            cin >> password;
            send_string(password);
            recvWithCheck(response);
        }
    }
    else { // new user
        cout << "Creating new user" << endl;
        cout << "Enter password: ";
        cin >> password;
        send_string(password);
        recvWithCheck(response);
        while (response != "OK"){
            cout << "Sorry, there seemed to be an error. Please try again" << endl;
            cout << "Enter pass: ";
            cin >> password;
            send_string(password);
            recvWithCheck(response);
        }    
    }
    // gen public key and send over
    char *pubKey = getPubKey();
    string pubKeyStr(pubKey);
    send_string(pubKey);
}

int recvWithCheck(string &outMsg) {
    int numBytesRec;
    char buf[MAX_LINE];
    int len = sizeof(buf);
    int flags = 0;

    memset(buf, 0, len);
 
    if ((numBytesRec=recv(sockfd, buf, len, flags)) == -1){
        perror("receive error");
        close(sockfd);
        exit(1);
    }
    if (numBytesRec == 0){
        cout << "recvWithCheck: zero bytes received" << endl;
        close(sockfd);
        exit(1);
    }
    outMsg = buf;
    return numBytesRec;
}

void send_string(string toSend) {
    int len = toSend.length();
    if (send(sockfd, toSend.c_str(), len, 0) == -1) {
        perror("Client send error!\n");
        exit(1);
    }
}

void send_short(short int val) {
	char toSend[MAX_LINE];
	sprintf(toSend, "%d", val);
	send_string(toSend);
}

void send_long(long int val) {
	char toSend[MAX_LINE];
	sprintf(toSend, "%ld", val);
	send_string(toSend);
}

int rec_int() {
	int i;
	char buf[MAX_LINE];	

	if ((i = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
		perror("did not receive int value");
		exit(1);
	}
	
	int value;
	sscanf(buf, "%d", &value); // convert value to an int

	return value;
}

int main(int argc, char * argv[]) {
    char buf[MAX_LINE];
    struct sockaddr_in serv_addr;
    string ip_address, username;
    int port_no;


    cout << "Enter the Ip Address: ";
    cin >> ip_address;

    cout << "Enter the Port no.: ";
    cin >> port_no;

    cout << "Enter the Username:";
    cin >> username;

    //cout << "Ip Address: " << ip_address << " Port: " <<port_no <<endl;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(" Socket creation failed");
        exit(EXIT_FAILURE);
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address.c_str(), &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    cout << "Connecting to " << ip_address << " on port " << port_no << endl;
    if (connect(sockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0) {
        perror("Socket connection failed");
        exit(EXIT_FAILURE);
    }

    cout << "Conected to: " << ip_address << endl; 

    // handle login
    handle_login(username);

	pthread_t msgThread;
    if (pthread_create(&msgThread, NULL, handle_messages, NULL)){
        perror("create handle_messages thread");
        exit(1);
    }
    promptUser(); 
    close(sockfd);
}



