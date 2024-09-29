#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define MESSAGE_LENGTH 1024
#define PORT 57777

int main() {
    int client_socket;
    struct sockaddr_in serveraddress;
    char message[MESSAGE_LENGTH];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET;

    if (connect(client_socket, (struct sockaddr*)&serveraddress, sizeof(serveraddress)) < 0) 
    {
        cerr << "Error connecting to server." << endl;
        return 1;
    }

    while (true) 
    {
        cout << "Enter command (register/login/message/end): ";
        cin.getline(message, MESSAGE_LENGTH);

        send(client_socket, message, strlen(message), 0);

        if (strncmp("end", message, 3) == 0) 
        {
            break;
        }

        memset(message, 0, MESSAGE_LENGTH);
        recv(client_socket, message, MESSAGE_LENGTH, 0);
        cout << "Response from server: " << message << endl;
    }

    close(client_socket);

    return 0;
}