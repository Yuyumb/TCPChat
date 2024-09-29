#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <unordered_map>

using namespace std;

#define MESSAGE_LENGTH 1024
#define PORT 57777

struct User {
    string login;
    string password;
    string name;

    User() {}  // Конструктор по умолчанию
    User(const string& login, const string& password, const string& name)
        : login(login), password(password), name(name) {}
};

class Chat {
private:
    unordered_map<string, User> user_map;
    unordered_map<string, bool> logged_in_users;
    vector<string> messages;
public:
    bool registerUser(const string& login, const string& password, const string& name) {
        if (user_map.find(login) != user_map.end()) {
            return false;
        }
        user_map[login] = User(login, password, name);
        return true;
    }

    bool loginUser(const string& login, const string& password) {
        auto it = user_map.find(login);
        if (it != user_map.end() && it->second.password == password) {
            if (logged_in_users[login]) {
                return false;
            }
            logged_in_users[login] = true;
            return true;
        }
        return false;
    }

    void logoutUser(const string& login) {
        logged_in_users[login] = false;
    }

    void sendMessage(const string& sender, const string& message) {
        messages.push_back(sender + ": " + message);
    }

    const vector<string>& getMessages() const {
        return messages;
    }
};

int main() {
    int server_socket, client_socket;
    struct sockaddr_in serveraddress, client;
    socklen_t client_size;
    char message[MESSAGE_LENGTH];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET;

    int bind_status = bind(server_socket, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if (bind_status == -1) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    int connect_status = listen(server_socket, 5);
    if (connect_status == -1) {
        cerr << "Error listening." << endl;
        return 1;
    }
    cout << "Server is listening for new connections..." << endl;

    client_size = sizeof(client);
    client_socket = accept(server_socket, (struct sockaddr*)&client, &client_size);
    if (client_socket == -1) {
        cerr << "Error accepting connection." << endl;
        return 1;
    }

    Chat chat;
    while (true) {
        memset(message, 0, MESSAGE_LENGTH);
        recv(client_socket, message, MESSAGE_LENGTH, 0);

        if (strncmp("end", message, 3) == 0) {
            cout << "Client Exited." << endl;
            break;
        }

        string msg_str(message);
        cout << "Data received: " << msg_str << endl;

        if (msg_str.find("register") == 0) 
        {
            if (msg_str.size() < 10) {
                send(client_socket, "Invalid register command", 24, 0);
                continue;
            }

            size_t first_space = msg_str.find(' ', 9);
            if (first_space == string::npos) {
                send(client_socket, "Invalid register command", 24, 0);
                continue;
            }

            string login = msg_str.substr(9, first_space - 9);

            size_t second_space = msg_str.find(' ', first_space + 1);
            if (second_space == string::npos) {
                send(client_socket, "Invalid register command", 24, 0);
                continue;
            }

            string password = msg_str.substr(first_space + 1, second_space - first_space - 1);
            string name = msg_str.substr(second_space + 1);

            if (chat.registerUser(login, password, name)) {
                send(client_socket, "Registration successful", 22, 0);
            }
            else {
                send(client_socket, "User already exists", 19, 0);
            }
            if (chat.registerUser(login, password, name)) {
                send(client_socket, "Registration successful", 22, 0);
            }
            else {
                send(client_socket, "User already exists", 19, 0);
            }
        }
        else if (msg_str.find("login") == 0) {
            string login = msg_str.substr(6, msg_str.find(' ', 6) - 6);
            string password = msg_str.substr(msg_str.find(' ', 6) + 1);
            if (chat.loginUser(login, password)) {
                send(client_socket, "Login successful", 16, 0);
            }
            else {
                send(client_socket, "User already logged in or invalid credentials", 45, 0);
            }
        }
        else if (msg_str.find("logout") == 0) {
            string login = msg_str.substr(7);
            chat.logoutUser(login);
            send(client_socket, "Logout successful", 17, 0);
        }
        else {
            chat.sendMessage("Client", msg_str);
            const auto& messages = chat.getMessages();
            for (const auto& msg : messages) {
                send(client_socket, msg.c_str(), msg.length(), 0);
            }
        }
    }
    close(client_socket);
    close(server_socket);

    return 0;
}
