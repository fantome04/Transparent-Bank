#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>


#include "bank.h"
#include "config.h"

sem_t* sem;
Bank* ptr;

int request_counter = 0;
bool is_shutting = false;

std::string logic(std::string input);
void server_handler();

void client_handler(int client_socket);

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("socket");
        exit(errno);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(hostshort);

    if(bind(server_socket, (struct sockaddr*)& server_address, sizeof(server_address)) < 0){
        perror("bind");
        exit(errno);
    }

    if(listen(server_socket, 1024) < 0){
        perror("listen");
        exit(errno);
    }

    std::cout << "waiting for connection\n";

    sem = sem_open(sem_name,  O_CREAT, 0666, 1);
    if(sem == SEM_FAILED) {
        std::cerr << "open" << std::endl;
        exit(errno);
    }


    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if(shm_fd == -1)
    {
        std::cerr << "shm_open client" <<std::endl;
        exit(errno);
    }

    std::size_t size = sizeof(Bank) + n * sizeof(BankCell);

    ptr = (Bank*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED)
    {
        std::cerr << "init mmap" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> client_threads;
    std::thread server_thread(server_handler);

    while(true) {
        int client_socket;
        struct sockaddr_in client_address;
        unsigned int client_addr_len = sizeof(client_address);

        if((client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_addr_len)) < 0){
            perror("accept");
            exit(errno);
        }

        std::cout << "Connected client with address: " << inet_ntoa(client_address.sin_addr) << "\n";

        client_threads.push_back(std::thread(client_handler, client_socket));
    }

    for (auto &thread : client_threads) {
        thread.join();
    }

    server_thread.join();

    if(close(server_socket) < 0){
        perror("close");
        exit(errno);
    }

    if(sem_close(sem) < 0) {
        std::cerr << "close" << std::endl;
        exit(errno);
    }

    if(munmap(ptr, size) == -1)
    {
        std::cerr << "init munmap" <<std::endl;
        exit(EXIT_FAILURE);
    }


    if(close(shm_fd) == -1) {
        std::cerr << "init close" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}

void server_handler() {
    bool has_printed = false;
    while(true) {
        if(request_counter % 5 == 0 && !has_printed){
            std::cout << "The count of total request is: " << request_counter << std::endl;
            has_printed = true;
        }
        if(request_counter % 5 == 1) {
            has_printed = false;
        }
    }
}

void client_handler(int client_socket) {
    char buffer[3001];
    int rs;

    while((rs = recv(client_socket, buffer, 3000, 0)) > 0){
        if(sem_wait(sem) < 0) {
            std::cerr << "wait" << std::endl;
            return;
        }

        buffer[rs] = '\0';

        if(std::string(buffer) == "shut down") {
            is_shutting = true;
        }

        if(is_shutting) {
            std::string mssg = std::string(buffer);
            int sent = send(client_socket, mssg.c_str(), mssg.size(), 0);
            if(sent == -1){
                return;
            }

            if(sem_post(sem) < 0) {
                std::cerr << "post" << std::endl;
                return;
            }

            if(close(client_socket) < 0){
                perror("close");
                return;
            }

            exit(EXIT_SUCCESS);
        }

        std::string mssg =  logic(buffer);
        ++request_counter;

        int sent = send(client_socket, mssg.c_str(), mssg.size(), 0);
        if(sent == -1){
            return;
        }

        if(sem_post(sem) < 0) {
            std::cerr << "post" << std::endl;
            return;
        }
    }

    if (rs == -1) {
        perror("receive");
        return;
    }

    if(close(client_socket) < 0){
        perror("close");
        return;
    }
}

std::string logic(std::string input)
{
    std::stringstream string(input);
    std::string temp;
    std::vector<std::string> in;
    while(string >> temp) {
        in.push_back(temp);
    }
    std::string str;

    if(in.size() == 2)
    {
        if(in[0] == "freeze")
        {
            str = ptr->freeze_cell(std::stoi(in[1]) - 1);
        }
        else if(in[0] == "unfreeze")
        {
            str = ptr->unfreeze_cell(std::stoi(in[1]) - 1);
        }
        else if(in[0] == "addall")
        {
            str = ptr->add_to_all(std::stoi(in[1]));
        }
        else if(in[0] == "suball")
        {
            str = ptr->sub_from_all(std::stoi(in[1]));
        }
        else if(in[0] == "getinfo") {
            str = ptr->get_info(std::stoi(in[1]) - 1);
        }
        else
        {
            str = "invalid input";
        }
    }
    else if(in.size() == 3)
    {
        if(in[0] == "setmin")
        {
            str = ptr->set_cell_min_amount(std::stoi(in[1]) - 1, std::stoi(in[2]));
        }
        else if(in[0] == "setmax")
        {
            str = ptr->set_cell_max_amount(std::stoi(in[1]) - 1, std::stoi(in[2]));
        }
        else
        {
            str = "invalid input";
        }
    }
    else if(in.size() == 4)
    {
        if(in[0] == "transfer")
        {
            str = ptr->transfer(std::stoi(in[1]) - 1, std::stoi(in[2]) - 1, std::stoi(in[3]));
        }
        else
        {
            str = "invalid input";
        }
    }
    else {
        str = "invalid input";
    }
    return str;
}
