/*
*	Test ventilator. To be later integrated into arp2.cpp. Pipeline 1/3
*	Binds PUSH socket to tcp://localhost:5557
*	Sends batch of tasks to workers via that socket
*
*	Tom O' Mara
*	Help source http://zguide.zeromq.org/cpp:taskvent
*
*   Comile and run: (cd into correct directory)
*		   (compiler) -o (outputfile) (file-to-comile) -I (path of libary include folder) -L (path to c++ bindings) -lzmq
*		%> g++ -o ../bin/ventilator ventilator.cpp -I ../zeromq-4.0.5/include -lzmq
*		%> ./../bin/ventilator
*/

#include "../lib/zeromq-4.0.5/include/zmq.hpp"
using zmq::context_t;
using zmq::socket_t;
using zmq::message_t;

#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
using std::cout;
using std::endl;




int main(int argc, char *argv[])
{
	zmq::context_t context(1); 

	//Socket to send messages on
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.bind("tcp://127.0.0.1:5557");	

	cout << "Press enter when the workers are ready: " << endl;
	getchar();
	cout << "Sending tasks to workers...\n" << endl;

	// Send '0' to sink to signal start of incoming batch
	zmq::socket_t sink(context, ZMQ_PUSH);
	sink.connect("tcp://127.0.0.1:5558");
	zmq::message_t message(20);			 //<-- Message size
	memcpy(message.data(), "0", 1);		 //<-- put 0 into a message
	sink.send(message);					 //<-- send message

	// Send 100 tasks
	int task_nbr;

	for(task_nbr = 0; task_nbr < 12; task_nbr++) {
		
		message.rebuild(10);
		sprintf ((char *) message.data(), "%d", task_nbr);
		sender.send(message);
	}

	sleep(1); // Give message time to deliver. 

	return 0;
}





