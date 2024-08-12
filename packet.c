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



[Unit]
Description=iodine DNS tunneling service
After=network.target

[Service]
Type=simple
User=root
Group=root
ExecStart=/usr/sbin/iodined -F -P 'Lahaja40' 10.0.0.1 ns.safaricom.pro
Restart=always

[Install]
WantedBy=multi-user.target



dig @10.0.0.1 ns.safaricom.pro

journalctl -u iodine

sudo /usr/sbin/iodined -f -P 'Lahaja40' 172.105.40.146 ns.safaricom.pro
tring all wesaddgfgttyreerddfssssgghy

Terming it expecting and not so lovely ghh i see one nation anf
god
gibyt uhhh helleluhya

getchar


gfdse

But they seem to

hujghyhgg

okay s thet said 

I am okay

fuctiomn well98777887

give them ..........tyre4567890-=12............. @#$%^&(*)
This code will handle the comma 
I said that it will happen actuall
...........,,,,,///

Testing 1234567890
Testing .....

n the code you provided, it looks like the issue with multiple window name logs appearing 

In that case well i don't know 

yes i mande che

but they said i am 