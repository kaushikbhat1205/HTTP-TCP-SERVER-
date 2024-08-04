#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<thread>
#include<sstream>
#include<fstream>
//#include<stringstream>
//#include "Server.h"
//#include "ServerData.h"
std::string respond404(){
    return "HTTP/1.1 404 Not Found\r\n\r\n";
}
void handle_http(int client_fd,struct sockaddr_in client_addr,int server_fd,std::string dir){
    std::string client_messg;
    ssize_t bytes_recvd = recv(client_fd, &client_messg[0], client_messg.max_size(), 0);
    if (bytes_recvd < 0) {
        std::cerr << "Error receiving the message from client" << std::endl;
        close(client_fd);
        close(server_fd);
        exit(0) ;
    }
    std::cout<<"received mssg"<<client_messg<<" "<<std::endl;
    std::string response = "";
//    std::string response = client_messg.starts_with("GET / HTTP/1.1\r\n") ? "HTTP/1.1 200 OK\r\n\r\n"
//                                                                          : "HTTP/1.1 404 Not Found\r\n\r\n";
//   accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    if (client_messg.starts_with("GET / HTTP/1.1\r\n")) {
        response = "HTTP/1.1 200 OK\r\n\r\n";
    } else if (client_messg.starts_with("GET /echo/")) {
        int start = -1;
        int end = -1;
        std::string path;

        int space1 = client_messg.find(' ');
//       int space2=-1;
        if (space1 != std::string::npos) {
            start = space1 + 1;
            int space2 = client_messg.find(' ', start);
            if (space2 != std::string::npos) {
                end = space2;
            }
        }
        path = client_messg.substr(start, end - start);
//        std::cout<<path<<std::endl;
        std::string response_body = path.substr(6);
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
                   std::to_string(response_body.size()) + "\r\n\r\n" + response_body;
    } else if (client_messg.starts_with("GET /user-agent")) {
        int header_start = client_messg.find("\r\n") + 4;
        if (header_start != std::string::npos) {
            int user_agent = client_messg.find("User-Agent: ", header_start) + 12;
            int user_agent_end = client_messg.find("\r\n", user_agent);
            std::string response_body = client_messg.substr(user_agent, user_agent_end - user_agent);
            if (user_agent != std::string::npos && user_agent_end != std::string::npos) {
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
                           std::to_string(response_body.size()) + "\r\n\r\n" + response_body;
            } else
                response = respond404();
        } else {
            response = respond404();
        }
    }
    else if(client_messg.starts_with("GET /files/")){
        std::cout<<"mssg: "<<client_messg<<std::endl;
        int pos = 11;
        std::cout<<" position start"<<pos<<std::endl;
        int end = client_messg.find(" ",11);
        std::cout<<"ending at "<<end<<std::endl;
        std::string file_name = client_messg.substr(pos,end-pos);
        std::cout<<"file name:"<<file_name<<std::endl;
        std::cout<<dir<<std::endl;
        std::ifstream files_data(dir + file_name);
        if (files_data.good()) {
            std::stringstream content;
            content << files_data.rdbuf();
//            std::stringstream respond("");
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + std::to_string(content.str().length()) + "\r\n\r\n" + content.str() + "\r\n";
        } else {
            response = respond404();
        }
//        std::cout<<"data found in the file "<<data<<std::endl;
//        response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + std::to_string(data.size()) + "\r\n\r\n"+data;
    }else if(client_messg.starts_with("POST /files/")){
        int start = 12;
        int end=client_messg.find(" ",12);
        std::string new_file_name = client_messg.substr(start,end-start);
        start = (client_messg.find("\r\n\r\n"))+4;
        end= client_messg.size()-2;
        std::string new_file_messg = client_messg.substr(start,end-start);
        std::cout<<"mssg from client "<<client_messg<<std::endl;
        std::cout<<"end of client message"<<std::endl;
        std::cout<<"mssg in file "<<new_file_messg<<std::endl;
        std::ofstream outfile(dir + new_file_name);
        outfile.write(new_file_messg.c_str(),new_file_messg.size());
        outfile.close();

        response = "HTTP/1.1 201 Created\r\n\r\n";
    }
    else {
        response = respond404();
    }
    std::cout << response << std::endl;
    send(client_fd, response.c_str(), response.size(), 0);
    std::cout << response << "sent as response from server to client" << std::endl;
}
int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
//  std::cout << std::unitbuf;
//  std::cerr << std::unitbuf;

  std::string dir;
  if(argc==3 && std::strcmp(argv[1],"--directory")==0){
    dir = argv[2];
  }

  // You can use print statements as follows for debugging, they'll be visible when running tests.
//  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
   int server_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
   }
  //
  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
   int reuse = 1;
   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
     std::cerr << "setsockopt failed\n";
     return 1;
   }

   struct sockaddr_in server_addr;
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = INADDR_ANY;
   server_addr.sin_port = htons(4221);

   if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
     std::cerr << "Failed to bind to port 4221\n";
     return 1;
   }

   int connection_backlog = 5;
   if (listen(server_fd, connection_backlog) != 0) {
     std::cerr << "listen failed\n";
     return 1;
   }

   struct sockaddr_in client_addr;
   int client_addr_len = sizeof(client_addr);

   std::cout << "Waiting for a client to connect...\n";
while(true) {
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    if (client_fd < 0) {
        std::cerr << "Error handling client connection" << std::endl;
        close(server_fd);
        return 1;
    }
    std::cout << "Client connected\n";
    std::thread th(handle_http,client_fd,client_addr,server_fd,dir);
    th.detach();
    }
   close(server_fd);

  return 0;
}