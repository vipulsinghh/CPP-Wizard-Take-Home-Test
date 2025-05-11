#include "abx_client.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

ABXClient::ABXClient(const std::string& host, int port)
    : host_(host), port_(port), sockfd_(-1), connected_(false), max_sequence_(0) {}

ABXClient::~ABXClient() {
    if (connected_) {
        close(sockfd_);
    }
}

void ABXClient::connect() {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        throw std::runtime_error("Error creating socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address");
    }

    if (::connect(sockfd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Connection failed");
    }

    connected_ = true;
}

void ABXClient::sendRequest(uint8_t callType, uint8_t resendSeq) {
    if (!connected_) {
        throw std::runtime_error("Not connected to server");
    }

    uint8_t request[2] = {callType, resendSeq};
    if (send(sockfd_, request, 2, 0) != 2) {
        throw std::runtime_error("Failed to send request");
    }
}

ABXClient::Packet ABXClient::readPacket() {
    if (!connected_) {
        throw std::runtime_error("Not connected to server");
    }

    const size_t PACKET_SIZE = 17; // 4 + 1 + 4 + 4 + 4 bytes
    uint8_t buffer[PACKET_SIZE];
    
    size_t total_read = 0;
    while (total_read < PACKET_SIZE) {
        ssize_t n = recv(sockfd_, buffer + total_read, PACKET_SIZE - total_read, 0);
        if (n <= 0) {
            throw std::runtime_error("Connection closed by server");
        }
        total_read += n;
    }

    Packet packet;
    size_t offset = 0;

    // Read symbol (4 bytes)
    packet.symbol = readString(buffer + offset, 4);
    offset += 4;

    // Read buy/sell indicator (1 byte)
    packet.buysellindicator = buffer[offset++];

    // Read quantity (4 bytes)
    packet.quantity = readInt32BE(buffer + offset);
    offset += 4;

    // Read price (4 bytes)
    packet.price = readInt32BE(buffer + offset);
    offset += 4;

    // Read sequence (4 bytes)
    packet.sequence = readInt32BE(buffer + offset);

    return packet;
}

void ABXClient::requestAllPackets() {
    sendRequest(1); // Call type 1: Stream All Packets
    
    while (true) {
        try {
            Packet packet = readPacket();
            received_packets_[packet.sequence] = packet;
            max_sequence_ = std::max(max_sequence_, packet.sequence);
        } catch (const std::runtime_error& e) {
            if (std::string(e.what()) == "Connection closed by server") {
                break;
            }
            throw;
        }
    }

    handleMissingSequences();
}

ABXClient::Packet ABXClient::requestPacket(int sequence) {
    connect(); // Reconnect for each request
    sendRequest(2, sequence); // Call type 2: Resend Packet
    Packet packet = readPacket();
    close(sockfd_);
    connected_ = false;
    return packet;
}

void ABXClient::handleMissingSequences() {
    for (int32_t seq = 1; seq <= max_sequence_; ++seq) {
        if (received_packets_.find(seq) == received_packets_.end()) {
            std::cout << "Requesting missing sequence: " << seq << std::endl;
            try {
                Packet packet = requestPacket(seq);
                received_packets_[seq] = packet;
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay between requests
            } catch (const std::exception& e) {
                std::cerr << "Error requesting sequence " << seq << ": " << e.what() << std::endl;
            }
        }
    }
}

std::vector<ABXClient::Packet> ABXClient::getAllPackets() {
    std::vector<Packet> packets;
    packets.reserve(received_packets_.size());
    
    for (const auto& pair : received_packets_) {
        packets.push_back(pair.second);
    }
    
    std::sort(packets.begin(), packets.end(),
              [](const Packet& a, const Packet& b) {
                  return a.sequence < b.sequence;
              });
    
    return packets;
}

void ABXClient::saveToJson(const std::string& filename) {
    json j = json::array();
    
    for (const auto& packet : getAllPackets()) {
        json packet_json = {
            {"symbol", packet.symbol},
            {"buysellindicator", std::string(1, packet.buysellindicator)},
            {"quantity", packet.quantity},
            {"price", packet.price},
            {"packetSequence", packet.sequence}
        };
        j.push_back(packet_json);
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    
    file << j.dump(4);
}

int32_t ABXClient::readInt32BE(const uint8_t* buffer) {
    return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

std::string ABXClient::readString(const uint8_t* buffer, size_t length) {
    return std::string(reinterpret_cast<const char*>(buffer), length);
}

int main() {
    try {
        ABXClient client;
        std::cout << "Connecting to server..." << std::endl;
        client.connect();
        
        std::cout << "Requesting all packets..." << std::endl;
        client.requestAllPackets();
        
        std::cout << "Saving packets to output.json..." << std::endl;
        client.saveToJson();
        
        std::cout << "Done! Check output.json for results." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
