// client.cpp
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>

#include "proto/interface.grpc.pb.h"
#include "proto/interface.pb.h"

/**
 * @class NumberClient
 * @brief Client for interacting with a gRPC-based number management service.
 *
 * @details This class provides a simple interface to perform CRUD-like operations
 *          (insert, delete, list, clear) on a remote number management service
 *          via gRPC.
 *
 * @note All operations are synchronous (blocking).
 * @note The class assumes the existence of the generated gRPC code from
 *       the `numbermgmt` proto definition (NumberManagement service).
 */
class NumberClient {
public:
    /**
     * @brief Constructs a NumberClient with the given gRPC channel.
     *
     * @param channel Shared pointer to a gRPC channel
     */
    NumberClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(numbermgmt::NumberManagement::NewStub(channel)) {}

    /**
     * @brief Inserts a number into the remote storage.
     *
     * @details Sends an InsertRequest to the server and prints the result.
     *          On success, prints the inserted entry with its timestamp.
     *
     * @param number The 64-bit unsigned integer to insert
     */
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
            std::cout << "RPC failed:\n"
                      << "  code    = " << status.error_code() << "\n"
                      << "  message = " << status.error_message() << "\n"
                      << "  details = " << status.error_details() << "\n";
        }
    }

    /**
     * @brief Deletes a specific number from the remote storage.
     *
     * @details Sends a DeleteRequest to the server and prints the result message.
     *
     * @param number The 64-bit unsigned integer to delete
     */
    void Delete(uint64_t number) {
        numbermgmt::DeleteRequest request;
        request.set_number(number);

        numbermgmt::OperationResult response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Delete(&context, request, &response);

        if (status.ok()) {
            std::cout << response.message() << "\n";
        } else {
            std::cout << "RPC failed:\n"
                      << "  code    = " << status.error_code() << "\n"
                      << "  message = " << status.error_message() << "\n"
                      << "  details = " << status.error_details() << "\n";
        }
    }

    /**
     * @brief Retrieves and prints all stored numbers with their insertion timestamps.
     *
     * @details Sends a ListRequest and prints the server response message followed
     * by all stored entries in the format:
     * number (unix_timestamp)
     *
     * @note The list is printed in the order returned by the server.
     */
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
            std::cout << "RPC failed:\n"
                      << "  code    = " << status.error_code() << "\n"
                      << "  message = " << status.error_message() << "\n"
                      << "  details = " << status.error_details() << "\n";
        }
    }

    /**
     * @brief Removes all numbers from the remote storage (clear operation).
     *
     * @details Sends a ClearRequest and prints the confirmation or error message.
     */
    void Clear() {
        numbermgmt::ClearRequest request;
        numbermgmt::OperationResult response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Clear(&context, request, &response);

        if (status.ok()) {
            std::cout << response.message() << "\n";
        } else {
            std::cout << "RPC failed:\n"
                      << "  code    = " << status.error_code() << "\n"
                      << "  message = " << status.error_message() << "\n"
                      << "  details = " << status.error_details() << "\n";
        }
    }

private:
    std::unique_ptr<numbermgmt::NumberManagement::Stub> stub_;
};

/**
 * @brief Counts whitespace-separated words in a string
 * @param str Input string
 * @return Number of words (0 for empty or whitespace-only input)
 */
unsigned int countWordsAlg(const std::string& str) {
    std::istringstream stream(str);
    // Use std::distance to count elements between the beginning and end iterators
    return std::distance(std::istream_iterator<std::string>(stream), std::istream_iterator<std::string>());
}

/**
 * @brief Prints the help/usage message for the gRPC Number Manager CLI.
 *
 * @details Displays a formatted, human-readable summary of available commands,
 *          syntax examples, and basic rules.
 */
void print_help() {
    std::cout << R"(
    gRPC Number Manager CLI
    ══════════════════════
    Commands:
    insert <number>     Add a positive integer           e.g. insert 2025
    delete <number>     Remove a number if it exists     e.g. delete 100
    list                Show all numbers (sorted) with timestamps
    clear               Delete everything
    help                Show this help message
    exit                Exit the program

    Rules:
      - Numbers must be positive integers greater than or equal to 2
      - Commands are case-sensitive
    )" << std::endl;
} 

int main() 
{
    auto channel = grpc::CreateChannel("unix-abstract:numbers-daemon.sock",
                                       grpc::InsecureChannelCredentials());

    NumberClient client(channel);

    print_help();

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
        else if (cmd == "help") {
            print_help();
        }
        else {
            std::cout << "Unknown command\n";
        }
        std::cout << "\n";
    }

    return 0;
}