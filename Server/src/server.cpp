// server.cpp
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_posix.h>

#include "proto/interface.grpc.pb.h"
#include "proto/interface.pb.h"

#include <iostream>
#include <map>
#include <mutex>
#include <ctime>
#include <memory>
#include <string>

class NumberServiceImpl final : public numbermgmt::NumberManagement::Service 
{
private:
    std::map<uint64_t, time_t> numbers_;  // number -> insertion timestamp
    std::mutex mutex_;

    numbermgmt::Timestamp make_timestamp(time_t t) {
        numbermgmt::Timestamp ts;
        ts.set_unix_seconds(static_cast<int64_t>(t));
        return ts;
    }

public:
    ::grpc::Status Insert(::grpc::ServerContext* context,
                          const ::numbermgmt::InsertRequest* request,
                          ::numbermgmt::OperationResult* response)
    {
        std::cout << "received insert request" << std::endl;
        std::lock_guard<std::mutex> lock(mutex_);

        uint64_t num = request->number();
        if (num == 0) {
            response->set_success(false);
            response->set_message("Only positive integers (≥1) are allowed");
            return grpc::Status::OK;
        }

        auto [it, inserted] = numbers_.try_emplace(num, time(nullptr));
        if (!inserted) {
            response->set_success(false);
            response->set_message("Number " + std::to_string(num) + " already exists");
        } else {
            response->set_success(true);
            response->set_message("Inserted " + std::to_string(num) +
                               " at " + std::to_string(it->second));
            auto* entry = response->mutable_entry();
            entry->set_number(num);
            *entry->mutable_timestamp() = make_timestamp(it->second);
        }
        
        return grpc::Status::OK;
    }


    ::grpc::Status Delete(::grpc::ServerContext* context,
                          const ::numbermgmt::InsertRequest* request,
                          ::numbermgmt::OperationResult* response)    
    {
        std::cout << "recieved delete request" << std::endl;
        std::lock_guard<std::mutex> lock(mutex_);

        uint64_t num = request->number();
        if (numbers_.erase(num)) {
            response->set_success(true);
            response->set_message("Deleted " + std::to_string(num));
        } else {
            response->set_success(false);
            response->set_message("Number " + std::to_string(num) + " not found");
        }
        
        return grpc::Status::OK;
    }

    ::grpc::Status List(::grpc::ServerContext* context,
                        const ::numbermgmt::ListRequest* request,
                        ::numbermgmt::NumberListResponse* response)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        response->set_count(numbers_.size());
        for (const auto& [num, ts] : numbers_) {
            auto* entry = response->add_entries();
            entry->set_number(num);
            *entry->mutable_timestamp() = make_timestamp(ts);
        }
        response->set_message("Current count: " + std::to_string(numbers_.size()));
        
        return grpc::Status::OK;
    }

    ::grpc::Status Clear(::grpc::ServerContext* context,
                         const ::numbermgmt::ClearRequest* request,
                         ::numbermgmt::OperationResult* response)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        size_t count = numbers_.size();
        numbers_.clear();

        response->set_success(true);
        response->set_message("Cleared " + std::to_string(count) + " numbers");
        
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string socket_address = "unix-abstract:numbers-daemon.sock";

    NumberServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(socket_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << socket_address << std::endl;

    // cleanup old socket file
    unlink("/tmp/numbers-daemon.sock");

    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
