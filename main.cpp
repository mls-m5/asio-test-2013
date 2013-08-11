//Licens http://www.boost.org/LICENSE_1_0.txt

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>


using std::endl; using std::cout;

//Tutorial 1-4
//int main(int argc, char **argv) {
//	boost::asio::io_service io;
//
//	boost::asio::deadline_timer  t(io, boost::posix_time::seconds(5));
//
////	t.wait();
//	t.async_wait([](const boost::system::error_code&){
//		cout << "har väntat" << endl;
//	}
//	);
//	std::cout << "gör det här först..." << std::endl;
//
//	io.run();
//	return 0;
//}

using boost::asio::ip::tcp;

int main(int argc, char **argv) {
	try
	{
		if (argc != 2){
			std::cerr << "usage: client <host>" << std::endl;
			return 1;
		}
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], "31415");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		for (;;){
			boost::array<char, 128> buf;
			boost::system::error_code error;
			size_t len = socket.read_some(boost::asio::buffer(buf), error);


			if (error == boost::asio::error::eof){
				break;
			}
			else if(error){
				throw boost::system::system_error(error);
			}

			std::cout.write(buf.data(), len);
		}
	}
	catch (std::exception& e){
		std::cerr << e.what() << std::endl;
	}
}
