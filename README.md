# ABX Exchange Client

This is a C++ client implementation for the ABX Exchange Server. The client connects to the server, receives stock ticker data, and generates a JSON file containing all packets with their sequences.

## Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.10 or higher
- nlohmann_json library
- Node.js 16.17.0 or higher (for running the server)

## Building the Project

1. First, install the required dependencies:

   On macOS (using Homebrew):
   ```bash
   brew install cmake nlohmann-json
   ```

   On Ubuntu/Debian:
   ```bash
   sudo apt-get install cmake nlohmann-json3-dev
   ```

2. Clone this repository:
   ```bash
   git clone <your-repo-url>
   cd abx_exchange_client
   ```

3. Create and enter the build directory:
   ```bash
   mkdir build && cd build
   ```

4. Configure and build the project:
   ```bash
   cmake ..
   make
   ```

## Running the Application

1. First, start the ABX Exchange Server:
   ```bash
   cd abx_exchange_server
   node main.js
   ```

2. In a new terminal, run the client:
   ```bash
   cd build
   ./abx_client
   ```

The client will:
1. Connect to the server (localhost:3000)
2. Request all packets
3. Handle any missing sequences
4. Generate a JSON file named `output.json` containing all packets in sequence

## Output Format

The generated `output.json` file will contain an array of objects, where each object represents a packet with the following structure:

```json
{
    "symbol": "MSFT",
    "buysellindicator": "B",
    "quantity": 50,
    "price": 100,
    "packetSequence": 1
}
```

## Error Handling

The client includes error handling for:
- Connection failures
- Network interruptions
- Missing sequences
- Invalid data formats

## License

This project is licensed under the MIT License - see the LICENSE file for details.
