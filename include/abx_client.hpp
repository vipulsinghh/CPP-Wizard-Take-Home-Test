#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

class ABXClient {
public:
    struct Packet {
        std::string symbol;
        char buysellindicator;
        int32_t quantity;
        int32_t price;
        int32_t sequence;
    };

    ABXClient(const std::string& host = "localhost", int port = 3000);
    ~ABXClient();

    // Connect to the server
    void connect();
    
    // Request all packets
    void requestAllPackets();
    
    // Request a specific packet by sequence
    Packet requestPacket(int sequence);
    
    // Get all packets in sequence
    std::vector<Packet> getAllPackets();
    
    // Save packets to JSON file
    void saveToJson(const std::string& filename = "output.json");

private:
    std::string host_;
    int port_;
    int sockfd_;
    bool connected_;
    std::map<int32_t, Packet> received_packets_;
    int32_t max_sequence_;

    // Helper methods
    void sendRequest(uint8_t callType, uint8_t resendSeq = 0);
    Packet readPacket();
    void handleMissingSequences();
    static int32_t readInt32BE(const uint8_t* buffer);
    static std::string readString(const uint8_t* buffer, size_t length);
};
