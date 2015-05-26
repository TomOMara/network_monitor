//	Tom O' Mara
//	Threaded ARP ping in C++(**Linux only**)  
//	Description: 
//		This program is Version 2 of arp. 
//		This program is intended to pass the results into a message queue. A module to save msgs to database and send msgs to remote server 
//		recieve messages from queue and run in a background thread. 
//  Note:
//	https://code.google.com/p/libcrafter/ <-- Required Library for compilation
// 	
//  31/03/2015
//  %> g++ -o ../bin/arp2 arp2.cpp -l crafter -lzmq
//  %> ./../bin/arp
//  To compile and run exe use this!!


#include <iostream>
#include <string> 
#include <fstream>
using std::ofstream;

#include "../lib/crafter-0.3/crafter.h" 			// <-- Header file required for high level packet creation commands
#include "../lib/zeromq-4.0.5/include/zmq.hpp"		// 	<--	Message Queue Library
//#include "../lib/zeromq-4.0.5/include/zhelpers.hpp"
using zmq::context_t;
using zmq::socket_t;
using zmq::message_t;


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
using std::cout;
using std::endl;
using std::string;
using std::vector;



/* Collapse namespaces */ 
using namespace std;
using namespace Crafter;



int main()
{
	// Set interface // (remember to change the interface name to eth0 for LAN cable or wlan0 for wireless),
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
/*************************************************
*	CONSTRUCTION AREA A START
*
*
*
*
***********************************************/
	// Create a connection to output channel
	zmq::context_t context(1); 

	//Socket to send messages on
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.bind("tcp://127.0.0.1:5557");	

	// Send '0' to sink to signal start of incoming batch
	zmq::socket_t sink(context, ZMQ_PUSH);
	sink.connect("tcp://127.0.0.1:5558");
	zmq::message_t message(35);			 //<-- Message size
	memcpy(message.data(), "0", 1);		 //<-- put 0 into a message
	sink.send(message);					 //<-- send message				



/*************************************************
*	CONSTRUCTION AREA A END
*
*
*
*
***********************************************/
	

	// Iterate over every reply within replies_packet container 
	vector<Packet*>::iterator it_pck; 
	int counter = 0; 
	for ( it_pck = replies_packet.begin() ; it_pck < replies_packet.end() ; it_pck++ )
	{
		Packet* reply_packet = (*it_pck); // create reference to individual packet.
		
		if(reply_packet)
		{
			cout <<"got one.."<<endl;
			ARP* arp_layer = reply_packet->GetLayer<ARP>();					    //<-- Get ARP layer of reply packet

/*************************************************
*	CONSTRUCTION AREA B START
*
*
*
*
***********************************************/
			// pass  to message queue 
			message.rebuild(35);
			sprintf ((char *) message.data(), "%s,%s" , arp_layer->GetSenderMAC().c_str(), arp_layer->GetSenderIP().c_str());	
		

			sender.send(message);				



/*************************************************
*	CONSTRUCTION AREA B END
*
*
*
*
***********************************************/
	

			cout << "[@] Host: " << arp_layer->GetSenderIP() << " is up with MAC Address: "     //<-- Print source IP 
				 << arp_layer->GetSenderMAC() << endl;  
			outfile << "[@] Host: " << arp_layer->GetSenderIP() << " is up with MAC Address: "  //<-- Write to file
				<< arp_layer->GetSenderMAC() << endl; 			

			counter++;
		}
		else
			cout << '.';
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
	
	cout <<"finished" << endl;
	return 0; 
}
	
