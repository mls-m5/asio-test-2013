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
#include <boost/archive/binary_oarchive.hpp>
#include "serialclass.h"

using boost::asio::ip::tcp;
using std::stringstream;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
	SerialClass sc;
	cout << "testar" << endl;

	boost::archive::binary_oarchive oa(cout);
	Serialize::serialize(oa, sc, 1.3);
	cout << endl;

	try{
		boost::asio::io_service io_service;
		tcp::acceptor acceptor (io_service, tcp::endpoint(tcp::v4(), 31415));

		for(;;){
			tcp::socket socket(io_service);
			acceptor.accept(socket);

			boost::system::error_code ignored_error;

			stringstream ss;
			boost::archive::binary_oarchive oa(ss);
			Serialize::serialize(oa, sc, 1.3);
			sc.setX(sc.getX() + 1);
			boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);
		}
	}
	catch (std::exception& e) {
	    std::cerr << e.what() << std::endl;
	}
}
