//Licens http://www.boost.org/LICENSE_1_0.txt

//#include <boost/asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/array.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include <sstream>
//#include "serialclass.h"
//
//using std::endl; using std::cout;
//using std::stringstream;
//
//using boost::asio::ip::tcp;
//
//int main(int argc, char **argv) {
//	SerialClass sc;
//
//	try
//	{
//		if (argc != 2){
//			std::cerr << "usage: client <host>" << std::endl;
//			return 1;
//		}
//		boost::asio::io_service io_service;
//		tcp::resolver resolver(io_service);
//		tcp::resolver::query query(argv[1], "31415");
//		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
//
//		tcp::socket socket(io_service);
//		boost::asio::connect(socket, endpoint_iterator);
//
//		stringstream ss;
//		for (;;){
//			boost::array<char, 128> buf;
//			boost::system::error_code error;
//			size_t len = socket.read_some(boost::asio::buffer(buf), error);
//
//			if (error == boost::asio::error::eof){
//				break;
//			}
//			else if(error){
//				throw boost::system::system_error(error);
//			}
//
//			ss.write(&buf.front(), len);
//		}
//
//		boost::archive::binary_iarchive ia(ss);
//		Serialize::serialize(ia, sc, 1.3);
//		cout << "klassen " << endl;
//		cout << "x: " << sc.getX() << " y: " << sc.getY() << endl;
//
//	}
//	catch (std::exception& e){
//		std::cerr << e.what() << std::endl;
//	}
//}


#include <cstdlib>
#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
	chat_client(boost::asio::io_service& io_service,
			tcp::resolver::iterator endpoint_iterator)
: io_service_(io_service),
  socket_(io_service)
{
		boost::asio::async_connect(socket_, endpoint_iterator,
				boost::bind(&chat_client::handle_connect, this,
						boost::asio::placeholders::error));
}

	void write(const chat_message& msg)
	{
		io_service_.post(boost::bind(&chat_client::do_write, this, msg));
	}

	void close()
	{
		io_service_.post(boost::bind(&chat_client::do_close, this));
	}

private:

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
					boost::asio::buffer((char*)&read_msg_.header, sizeof(read_msg_.header)),
					boost::bind(&chat_client::handle_read_header, this,
							boost::asio::placeholders::error));
		}
	}

	void handle_read_header(const boost::system::error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(read_msg_.mess, read_msg_.header.size),
				boost::bind(&chat_client::handle_read_body, this,
						boost::asio::placeholders::error));
		}
		else
		{
			do_close();
		}
	}

	void handle_read_body(const boost::system::error_code& error)
	{
		if (!error)
		{
			if (read_msg_.header.type == chat_message::mtText){
				std::cout.write(read_msg_.data(), read_msg_.header.size);
				std::cout << "\n";
			}
			boost::asio::async_read(socket_,
					boost::asio::buffer((char*)&read_msg_.header, sizeof(read_msg_.header)),
					boost::bind(&chat_client::handle_read_header, this,
							boost::asio::placeholders::error));
		}
		else
		{
			do_close();
		}
	}

	void do_write(chat_message msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			boost::asio::async_write(socket_,
					boost::asio::buffer(write_msgs_.front().begin(),
							write_msgs_.front().length()),
							boost::bind(&chat_client::handle_write, this,
									boost::asio::placeholders::error));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				boost::asio::async_write(socket_,
						boost::asio::buffer(write_msgs_.front().data(),
								write_msgs_.front().length()),
								boost::bind(&chat_client::handle_write, this,
										boost::asio::placeholders::error));
			}
		}
		else
		{
			do_close();
		}
	}

	void do_close()
	{
		socket_.close();
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

int main(int argc, char* argv[])
{
	try
	{		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query("localhost", "31415");
		tcp::resolver::iterator iterator = resolver.resolve(query);

		chat_client c(io_service, iterator);

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

		while (1)
		{

			using namespace std; // For strlen and memcpy.
			chat_message msg;
			msg.setString("apa bepa cepa");
			c.write(msg);
			boost::this_thread::sleep(boost::posix_time::millisec(1000));
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
