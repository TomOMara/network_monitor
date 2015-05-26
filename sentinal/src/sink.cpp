/*
*	Test sink. Pipeline 3/3
*	
*	Sink application: collects all tasks from workers and displays results. 
*	Binds pull socket to tcp://localhost:5558
*	Collects results from workers via that socket
*
*	Tom O' Mara
*	Help source http://zguide.zeromq.org/cpp:tasksink
*
*   Comile and run: (cd into correct directory)
*		   (compiler) -o (outputfile) (file-to-comile) -I (path of libary include folder) -L (path to c++ bindings) -lzmq
*		%> g++ -o ../bin/sink sink.cpp -I ../lib/zeromq-4.0.5/include -lzmq
*		%>  ./../bin/sink
*/

#include "../lib/zeromq-4.0.5/include/zmq.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	//	Prepare context and socket
	zmq::context_t context(1);
	zmq::socket_t receiver(context, ZMQ_PULL);
	receiver.bind("tcp://127.0.0.1:5558");
	
	//	Wait for start of batch
	zmq::message_t message;
	receiver.recv(&message);

	int task_nbr;
	
	//	Display success/failure results
	//for (task_nbr = 0; task_nbr < 12; task_nbr++){
	while(1){

		if(receiver.recv(&message)){ 
		


		std::string(static_cast<char*>(message.data()));

		if ((task_nbr / 10) * 10 == task_nbr)
			std::cout << ":" << std::flush;
		else
			std::cout << "." << std::flush;
	
	
	}else std::cout << "msg not recieved" << std::endl;

	}
	return 0;
	
}
