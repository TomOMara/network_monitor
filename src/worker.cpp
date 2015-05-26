/*
*	Test worker. Pipeline 2/3
*	
*	Worker application: receives a message, sleeps for that number of seconds, and then signals that its finished:
*
*	connects PULL socket to tcp://localhost:5557
*	collects workloads from ventilator via that socket
*	connects PUSH socket to tcp://localhost:5558
*	sends results to sink via that socket
*	Tom O' Mara
*	Help source http://zguide.zeromq.org/cpp:taskwork
*
*   Comile and run: (cd into correct directory)
*		   (compiler) -o (outputfile) (file-to-comile) -I (path of libary include folder) -lzmq
*		%> g++ -o ../bin/worker worker.cpp -I ../lib/zeromq-4.0.5/include -lzmq
*		%> ./../bin/worker
*/

#include "../lib/zeromq-4.0.5/include/zhelpers.hpp"

int main()
{
	zmq::context_t context(1);

	//	Socket to receive messages on
	zmq::socket_t receiver(context, ZMQ_PULL);
	receiver.connect("tcp://127.0.0.1:5557");
	
	//	Socket to send messages to
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.connect("tcp://127.0.0.1:5558");

	//	Process tasks forever

	while(1){
	
		zmq::message_t message; 

		std::string data;
	
		receiver.recv(&message);

		// 	Put message data into variable
		std::istringstream iss(static_cast<char *>(message.data()));
		iss >> data;

		// ** DO SOMETHING WITH THE VARIABLE! **


		// Show contents of message
		std::cout << "Data value:" << data << std::endl;


		//	Send success/failure results to sink
		message.rebuild();
		sender.send(message);

		//	Process indicator for viewer
		std::cout << "." << std::flush;
		
 	
	}	
	return 0;
}


