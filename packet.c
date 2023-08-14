#include <stdio.h>
#include <pcap.h>

#define SNAPLEN 65535  // Maximum number of bytes to capture
#define PROMISCUOUS_MODE 1
#define TIMEOUT_MS 1000

void packetHandler(u_char *userData, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    printf("Received a packet of size %d\n", pkthdr->len);
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *captureHandle;
    pcap_if_t *alldevs, *device;

    printf("Before pcap_findalldevs\n");
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        return 1;
    }
    printf("After pcap_findalldevs\n");

    device = alldevs;

    // Open the network interface for capturing
    captureHandle = pcap_open_live(device->name, SNAPLEN, PROMISCUOUS_MODE, TIMEOUT_MS, errbuf);
    if (captureHandle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", device->name, errbuf);
        pcap_freealldevs(alldevs);
        return 1;
    }

    // Start capturing packets
    pcap_loop(captureHandle, 0, packetHandler, NULL);

    // Close the packet capture handle
    pcap_close(captureHandle);
    pcap_freealldevs(alldevs);
    system("pause");
    return 0;
}
