//	Tom O' Mara
//	Threaded ARP ping in C++(**Linux only**)  
//	Description: 
//		This program 
//  Note:
//	Windows: You should setup a cygwin environment and compile libcrafter with the instructions for linux on the webpage.
//	https://www.cygwin.com/ <-- Cygwin site, http://www.voxforge.org/home/docs/cygwin-cheat-sheet <-- Cygwin help
//	https://code.google.com/p/libcrafter/ <-- Required Library for compilation
// 	
//  31/03/2015
//  %> g++ -o ../bin/arp arp.cpp -l crafter 
//  %> ./../bin/arp

//To compile and run exe use this!!

#include <iostream>
#include <string> 
#include <fstream>
#include "../lib/crafter-0.3/crafter.h" // <-- Header file required for high level packet creation commands

/* Collapse namespaces */ 
using namespace std;
using namespace Crafter;



int main()
{
	// Set interface // (remember to change the interface name "wlan0" by yours),
	string iface = "eth0";
	
	// Get the IP address associated to the interface //
	string myIP = GetMyIP(iface);	
	// Get the MAC Address associated to the interface //
	string myMAC = GetMyMAC(iface);
	
	// Print information // 
	cout << "[@] My MAC Address is: " << myMAC << endl;
	cout << "[@] My IP Address is: "  << myIP  << endl;
	
	// ------ Common data to all headers ------ //
	
	Ethernet ether_header; 
	
	ether_header.SetSourceMAC(myMAC);						// <-- Set our MAC as source //
	ether_header.SetDestinationMAC("ff:ff:ff:ff:ff:ff"); 	// <-- Set broadcast address // 
	
	ARP arp_header; 
	
	arp_header.SetOperation(ARP::Request); 					// <-- Set Operation (ARP Request) 
	arp_header.SetSenderIP(myIP);							// <-- Set our network data 
	arp_header.SetSenderMAC(myMAC);							// <-- Set our MAC as sender
	
	// -------------------------------------------- //
	
	// Define network to scan // my pc = 139.184.50.155, other = 139.184.50.154 router = 139.184.50.1
	
	vector<string> net = GetIPs("10.0.2.*"); 			// <-- Create container of IP Addresses
	vector<string>::iterator ip_addr; 						// <-- Iterator
	
	// Create a container of packet pointers to hold all ARP Requests 
	vector<Packet*> request_packets;
	
	// Iterate to access each string that defines an IP address 
	for( ip_addr = net.begin() ; ip_addr != net.end() ; ip_addr++ )
	{
		arp_header.SetTargetIP(*ip_addr); 					// <-- Set a destination IP address on the ARP header
		
		// Create packet on the heap 
		Packet* packet = new Packet; 
		
		// Push the layers 
		packet->PushLayer(ether_header); 
		packet->PushLayer(arp_header); 
		
		// Push the packet into the container 
		request_packets.push_back(packet);
		
	}

	// Now we have all the packets in the request_packets container, now we can send them
	//  48  ( n-threads )  -> Number of threads for distributing the packets tunnel-able (run tests on pi to find best value.          
  	//  0.1 ( time-out )   -> Time-out in seconds for waiting an answer (100 ms)
    	//  2   ( retry )      -> Number of times we send a packet until a response is received
	
	cout << "[@] Sending ARP Requests. Please Wait..." << endl; 
	
	// Create container that's similar to request_packets to hold the responses 	
	vector<Packet*> replies_packet(request_packets.size());
	
	// Send packets
	SendRecv(request_packets.begin(), request_packets.end(), replies_packet.begin(), iface, 0.1, 2, 48);
	
	cout << "[@] Packets sent " << endl; 
	
	// Create an output file
	ofstream outfile;
	outfile.open( "results.txt" ); 

	// Iterate over every reply within replies_packet container 
	vector<Packet*>::iterator it_pck; 
	int counter = 0; 
	for ( it_pck = replies_packet.begin() ; it_pck < replies_packet.end() ; it_pck++ )
	{
		Packet* reply_packet = (*it_pck); // create reference to individual packet.
		
		if(reply_packet)
		{
			ARP* arp_layer = reply_packet->GetLayer<ARP>();					    //<-- Get ARP layer of reply packet
			cout << "[@] Host: " << arp_layer->GetSenderIP() << " is up with MAC Address: "     //<-- Print source IP 
				 << arp_layer->GetSenderMAC() << endl;  
			outfile << "[@] Host: " << arp_layer->GetSenderIP() << " is up with MAC Address: "  //<-- Write to file
				<< arp_layer->GetSenderMAC() << endl; 			

			counter++;
		}
	}
	
	outfile.close(); 		// <-- Close our file 

	cout << "[@] " << counter << "hosts up. " << endl; 
	
	// Delete the container with the ARP requests. 
	for(it_pck = request_packets.begin() ; it_pck < request_packets.end() ; it_pck++)
	{
		delete (*it_pck);
	} 
	
	
	// Delete the container with the ARP responses. 
	for(it_pck = replies_packet.begin() ; it_pck < replies_packet.end() ; it_pck++ )
	{
		delete (*it_pck); 
	}	
	
	return 0; 
}
	
	

	
