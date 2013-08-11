/*
 * server.cpp
 *
 *  Created on: 11 aug 2013
 *      Author: mattias
 */


#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <sstream>
#include "serialclass.h"

using boost::asio::ip::tcp;
using std::stringstream;
using std::cout;
using std::endl;

//std::string make_daytime_string(){
//	using namespace std;
//	time_t now = time(0);
//	return ctime(&now);
//}

int main(int argc, char **argv) {
	SerialClass sc;
	cout << "testar" << endl;

	Serialize::oarchive oa(cout);
	Serialize::serialize(oa, sc, 1.3);
	cout << endl;

	try{
		boost::asio::io_service io_service;
		tcp::acceptor acceptor (io_service, tcp::endpoint(tcp::v4(), 31415));

		for(;;){
			tcp::socket socket(io_service);
			acceptor.accept(socket);

//			std::string message = make_daytime_string();

			boost::system::error_code ignored_error;
//			boost::asio::write(socket, boost::asio::buffer(message), ignored_error);


			stringstream ss;
			Serialize::oarchive oa(ss);
			Serialize::serialize(oa, sc, 1.3);
			sc.setX(sc.getX() + 1);
			boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);
		}
	}
	catch (std::exception& e) {
	    std::cerr << e.what() << std::endl;
	}
}
