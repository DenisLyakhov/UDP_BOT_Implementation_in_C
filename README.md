# UDP_BOT_Implementation_in_C

A lightweight C application that simulates IoT device data and transmits it to a UDP server with configurable intervals and local logging.

## Features
- **Data Generation**:
  - Timestamp (Unix time)                           - 8 bytes
  - Auto-incrementing packet ID (1-255)             - 1 byte
  - Random temperature readings (20.0°C to 120.0°C) - 2 bytes
  - Power source option (MAINS/BATTERY)             - 1 byte
  - Checksum validation                             - 4 bytes
- **Configurable Intervals**:
  - Separate options for UDP transmission and local logging intervals
- **Network & Storage**:
  - UDP socket communication
  - Persistent logging to text files

## Build Instructions

### Prerequisites
- GCC compiler
- Linux/Unix-like OS (for socket headers)

### Compilation
```bash
gcc udp_bot.c -o udp_bot
```

## Usage

### Command

``` bash
./udp_bot <server_ip> <server_port> <send_interval> <log_interval> <logfile_path>
```

### Example

``` bash
./udp_bot 192.168.1.100 8080 1 5 device_log.txt
```

## Verification

The application was tested using Wireshark
