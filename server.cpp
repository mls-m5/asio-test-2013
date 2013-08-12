/*
 * server.cpp
 *
 *  Created on: 11 aug 2013
 *      Author: mattias
 */

//
//#include <ctime>
//#include <iostream>
//#include <string>
//#include <boost/asio.hpp>
//#include <sstream>
//#include <boost/archive/binary_oarchive.hpp>
//#include "serialclass.h"
//
//using boost::asio::ip::tcp;
//using std::stringstream;
//using std::cout;
//using std::endl;
//
//int main(int argc, char **argv) {
//	SerialClass sc;
//	cout << "testar" << endl;
//
//	boost::archive::binary_oarchive oa(cout);
//	Serialize::serialize(oa, sc, 1.3);
//	cout << endl;
//
//	try{
//		boost::asio::io_service io_service;
//		tcp::acceptor acceptor (io_service, tcp::endpoint(tcp::v4(), 31415));
//
//		for(;;){
//			tcp::socket socket(io_service);
//			acceptor.accept(socket);
//
//			boost::system::error_code ignored_error;
//
//			stringstream ss;
//			boost::archive::binary_oarchive oa(ss);
//			Serialize::serialize(oa, sc, 1.3);
//			sc.setX(sc.getX() + 1);
//			boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);
//		}
//	}
//	catch (std::exception& e) {
//	    std::cerr << e.what() << std::endl;
//	}
//}

//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;
//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_session
		: public boost::enable_shared_from_this<chat_session>
{
public:
	chat_session(boost::asio::io_service& io_service)
: socket_(io_service)
{
}

	tcp::socket& socket();

	void start();

	void deliver(const chat_message& msg);
	void handle_read_header(const boost::system::error_code& error);
	void handle_read_body(const boost::system::error_code& error);

	void handle_write(const boost::system::error_code& error);

private:
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

typedef boost::shared_ptr<chat_session> chat_session_ptr;

namespace room{
std::set<chat_session_ptr> participants_;
enum { max_recent_msgs = 100 };
chat_message_queue recent_msgs_;

void join(chat_session_ptr participant);

void leave(chat_session_ptr participant);

void deliver(const chat_message& msg);
}
using namespace room;


//----------------------------------------------------------------------

tcp::socket& chat_session::socket()
{
	return socket_;
}

void chat_session::start()
{
	join(shared_from_this());
	boost::asio::async_read(socket_,
			boost::asio::buffer((char*)&read_msg_.header, chat_message::header_length),
			boost::bind(
					&chat_session::handle_read_header, shared_from_this(),
					boost::asio::placeholders::error));
}

void chat_session::deliver(const chat_message& msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		auto& msg = write_msgs_.front();
		boost::asio::async_write(socket_,
				boost::asio::buffer((char*)&msg.header,
						msg.length()),
						boost::bind(&chat_session::handle_write, shared_from_this(),
								boost::asio::placeholders::error));
	}
}

void chat_session::handle_read_header(const boost::system::error_code& error)
{
	if (!error)
	{
		boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.mess, read_msg_.header.size),
				boost::bind(&chat_session::handle_read_body, shared_from_this(),
						boost::asio::placeholders::error));
	}
	else
	{
		leave(shared_from_this());
	}
}

void chat_session::handle_read_body(const boost::system::error_code& error)
{
	if (!error)
	{
		room::deliver(read_msg_);
		boost::asio::async_read(socket_,
				boost::asio::buffer((char*)&read_msg_.header, chat_message::header_length),
				boost::bind(&chat_session::handle_read_header, shared_from_this(),
						boost::asio::placeholders::error));
	}
	else
	{
		leave(shared_from_this());
	}
}

void chat_session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		write_msgs_.pop_front();
		if (!write_msgs_.empty())
		{
			chat_message &msg = write_msgs_.front();
			boost::asio::async_write(socket_,
					boost::asio::buffer((char*)&msg.header,
							msg.length()),
							boost::bind(&chat_session::handle_write, shared_from_this(),
									boost::asio::placeholders::error));
		}
	}
	else
	{
		leave(shared_from_this());
	}
}


//----------------------------------------------------------------------


void room::join(chat_session_ptr participant)
{
	participants_.insert(participant);
	std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
			boost::bind(&chat_session::deliver, participant, _1));
}

void room::leave(chat_session_ptr participant)
{
	participants_.erase(participant);
}

void room::deliver(const chat_message& msg)
{
	recent_msgs_.push_back(msg);
	while (recent_msgs_.size() > max_recent_msgs)
		recent_msgs_.pop_front();

	std::for_each(participants_.begin(), participants_.end(),
			boost::bind(&chat_session::deliver, _1, boost::ref(msg)));
}

using namespace room;
//----------------------------------------------------------------------

class chat_server
{
public:
	chat_server(boost::asio::io_service& io_service,
			const tcp::endpoint& endpoint)
: io_service_(io_service),
  acceptor_(io_service, endpoint)
{
		start_accept();
}

	void start_accept()
	{
		chat_session_ptr new_session(new chat_session(io_service_));
		acceptor_.async_accept(new_session->socket(),
				boost::bind(&chat_server::handle_accept, this, new_session,
						boost::asio::placeholders::error));
	}

	void handle_accept(chat_session_ptr session,
			const boost::system::error_code& error)
	{
		if (!error)
		{
			session->start();
		}

		start_accept();
	}

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
	try
	{
		std::cout << "startar server port 31415" << std::endl;

		boost::asio::io_service io_service;

		using namespace std; // For atoi.
		tcp::endpoint endpoint(tcp::v4(), atoi("31415"));
		chat_server_ptr server(new chat_server(io_service, endpoint));

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
