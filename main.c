#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

typedef enum {
    POWER_MAINS = 0,
    POWER_BATTERY = 1
} device_power_t;

typedef struct {
    time_t timestamp;
    uint8_t id;
    int16_t temperature;
    device_power_t status;
    uint32_t checksum;
} device_data_t;

FILE* log_file = NULL;

int sockfd;
struct sockaddr_in servaddr;

char* server_ip;
int server_port;

int transmission_id = 0;

const char* status_str[] = {"MAINS", "BATTERY"};

int send_interval_s;
int log_interval_s;

int16_t generate_temperature() {
    return (rand() % 1001) + 200;
}

device_power_t generate_status() {
    return rand() % 2;
}

void fill_checksum(device_data_t* data) {
    data->checksum = 0;
    
    uint32_t sum = 0;
    const uint8_t* bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < sizeof(device_data_t); i++) {
        sum += bytes[i];
    }
    
    data->checksum = sum;
}

void log_to_file(const device_data_t* data) {
    char time_buf[26];
    strftime(time_buf, 26, "%Y-%m-%d %H:%M:%S", localtime(&data->timestamp));

    fprintf(log_file, "Log:\n");
    fprintf(log_file, "Timestamp: %s\n", time_buf);
    fprintf(log_file, "Temperature: %.1f°\n", data->temperature / 10.0f);
    fprintf(log_file, "Power source: %s\n", status_str[data->status]);
    fprintf(log_file, "Identifier: %d\n", data->id);
    fprintf(log_file, "Checksum: %u\n", data->checksum);
    fprintf(log_file, "------------------------------\n\n");

    fflush(log_file);
}

void initialize_log_file(char* filename) {
    log_file = fopen(filename, "a");

    if (log_file == NULL) {
        perror("Log file open failure");
        exit(EXIT_FAILURE);
    }
}

void initialize_socket() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Socket failure");
        fclose(log_file);
        exit(EXIT_FAILURE);
    }
}

void configure_address() {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("Address is invalid");
        close(sockfd);
        fclose(log_file);
        exit(EXIT_FAILURE);
    }
}

device_data_t fill_data() {
    device_data_t data;

    data.timestamp = time(NULL);
    data.id = ++transmission_id;
    data.temperature = generate_temperature();
    data.status = generate_status();

    fill_checksum(&data);

    return data;
}

void start_udp_loop() {
    int transmission_elapsed_s = 0;
    int log_elapsed_s = 0;

    device_data_t data;

    while (transmission_id < UINT8_MAX) {

        if (transmission_elapsed_s >= send_interval_s) {
            data = fill_data();

            ssize_t sent_bytes = sendto(sockfd, &data, sizeof(data), 0,
                                    (const struct sockaddr *)&servaddr, sizeof(servaddr));
            if (sent_bytes < 0) {
                perror("Send failed");
                break;
            }

            printf("Sent %zd bytes to %s:%d\n", sent_bytes, server_ip, server_port);

            transmission_elapsed_s = 0;
        }

        if (log_elapsed_s >= log_interval_s) {
            printf("Logged\n");
            log_to_file(&data);
            log_elapsed_s = 0;
        }

        transmission_elapsed_s++;
        log_elapsed_s++;

        sleep(1);
    }
}

void dealloc_resources() {
    close(sockfd);
    fclose(log_file);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <server_ip> <server_port> <transmission_cycle_s> <log_cycle_s> <log_file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server_ip = argv[1];
    server_port = atoi(argv[2]);
    send_interval_s = atoi(argv[3]);
    log_interval_s = atoi(argv[4]);
    char* filename = argv[5];

    initialize_log_file(filename);

    initialize_socket();
    configure_address();

    start_udp_loop();
    
    dealloc_resources();
    
    return 0;
}