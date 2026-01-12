// client.cpp
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>

#include "proto/interface.grpc.pb.h"

class NumberClient {
public:
    NumberClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(numbermgmt::NumberManagement::NewStub(channel)) {}

    void Insert(uint64_t number) {
        numbermgmt::InsertRequest request;
        request.set_number(number);

        numbermgmt::OperationResult response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Insert(&context, request, &response);

        if (status.ok()) {
            if (response.success()) {
                std::cout << "Success: " << response.message() << "\n";
                if (response.has_entry()) {
                    std::cout << "  number: " << response.entry().number()
                              << "  inserted: " << response.entry().timestamp().unix_seconds() << "\n";
                }
            } else {
                std::cout << "Failed: " << response.message() << "\n";
            }
        } else {
            std::cout << "RPC failed: " << status.error_message() << "\n";
        }
    }

    void Delete(uint64_t number) {
        numbermgmt::DeleteRequest request;
        request.set_number(number);

        numbermgmt::OperationResult response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Delete(&context, request, &response);

        if (status.ok()) {
            std::cout << response.message() << "\n";
        } else {
            std::cout << "RPC failed: " << status.error_message() << "\n";
        }
    }

    void List() {
        numbermgmt::ListRequest request;
        numbermgmt::NumberListResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->List(&context, request, &response);

        if (status.ok()) {
            std::cout << response.message() << "\n";
            for (const auto& entry : response.entries()) {
                std::cout << entry.number()
                          << "  (" << entry.timestamp().unix_seconds() << ")\n";
            }
        } else {
            std::cout << "RPC failed: " << status.error_message() << "\n";
        }
    }

    void Clear() {
        numbermgmt::ClearRequest request;
        numbermgmt::OperationResult response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Clear(&context, request, &response);

        if (status.ok()) {
            std::cout << response.message() << "\n";
        } else {
            std::cout << "RPC failed: " << status.error_message() << "\n";
        }
    }

private:
    std::unique_ptr<numbermgmt::NumberManagement::Stub> stub_;
};

unsigned int countWordsAlg(const std::string& str) {
    std::istringstream stream(str);
    // Use std::distance to count elements between the beginning and end iterators
    return std::distance(std::istream_iterator<std::string>(stream), std::istream_iterator<std::string>());
}

int main() 
{
    auto channel = grpc::CreateChannel("unix-abstract:numbers-daemon.sock",
                                       grpc::InsecureChannelCredentials());

    NumberClient client(channel);

    std::cout << "gRPC Number Manager CLI\n"
                 "Usage:\n"
                 "    command [options]\n\n"
                 "Available Commands\n"
                 "    insert <positive integer>     Sends a number from the CLI to the backend server instance\n"
                 "    delete <positive integer>     Deletes the specified number from the backend server\n"
                 "    list                          Prints all numbers to console, organized smallest to largest, with their timestamp\n"
                 "    clear                         Deletes all stored entries\n\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "insert") {
            uint64_t num;
            char potential_extra_char;

            // Verify correct number of args
            if(countWordsAlg(iss.str()) > 2){
                std::cout << "Too many arguments were input\n";
            }
            else{
                // verify a positive integer was received.
                if (!(iss >> num)) std::cout << "Usage: insert <positive integer>\n";
                else if (iss >> potential_extra_char) std::cout << "Extra characters found in command\n";
                else{
                    if (num <= 1) std::cout << "number must be a positive integer\n";
                    else client.Insert(num);
                }
            }
        }
        else if (cmd == "delete") {
            uint64_t num;
            char potential_extra_char;

            // Verify correct number of args
            if(countWordsAlg(iss.str()) > 2){
                std::cout << "Too many arguments were input\n";
            }
            else{
                // verify a positive integer was received.
                if (!(iss >> num)) std::cout << "Usage: insert <positive integer>\n";
                else if (iss >> potential_extra_char) std::cout << "Extra characters found in command\n";
                else{
                    if (num <= 1) std::cout << "number must be a positive integer\n";
                    else client.Delete(num);
                }
            }
        }
        else if (cmd == "list") {
            // Verify correct number of args
            if(countWordsAlg(iss.str()) > 1){
                std::cout << "Too many arguments were input\n";
            }
            else{
                client.List();
            }
        }
        else if (cmd == "clear") {
            // Verify correct number of args
            if(countWordsAlg(iss.str()) > 1){
                std::cout << "Too many arguments were input\n";
            }
            else{
                client.Clear();
            }
        }
        else {
            std::cout << "Unknown command\n";
        }
        std::cout << "\n";
    }

    return 0;
}