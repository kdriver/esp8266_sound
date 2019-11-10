#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "UDPLogger.h"


WiFiUDP Udp;
byte tx_buffer[1000];
unsigned int len;
byte tx[] = {'H','E','L','L','O','\n'};
UDPLogger::UDPLogger(const char *ip,unsigned short int port)
{
    strcpy(remote,ip);
    dest_port = port;
}
void UDPLogger::init(void)
{
    Udp.begin(8787);
    Udp.beginPacket("192.168.1.161",dest_port);
    Udp.write(tx,sizeof(tx));
    Udp.endPacket();
}

void UDPLogger::send(String s )
{
   
    Udp.beginPacket("192.168.1.161",dest_port);
    Udp.beginPacket(remote,dest_port);
    s.getBytes(tx_buffer,sizeof(tx_buffer));
    Udp.write(tx_buffer,s.length());
    Udp.endPacket();
}