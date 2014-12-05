#include "logging.hpp"
#include "avproto_wrapper.hpp"
#include "bot_group.hpp"

#include "avproto.hpp"
#include "avproto/avjackif.hpp"
#include "avproto/message.hpp"
#include "bot_service.hpp"

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

namespace bot_avim {

	avproto_wrapper::avproto_wrapper(boost::asio::io_service& io_service, std::string key, std::string crt)
	: bot_avproto(io_service, key, crt)
	, m_avkernel(io_service)
	{
		LOG_DBG << "avproto constructor";
	}

	avproto_wrapper::~avproto_wrapper()
	{}
	
	bool avproto_wrapper::register_service(bot_service *service)
	{
		m_service.reset(service);
	}
	
	bool avproto_wrapper::start()
	{
		boost::asio::spawn(m_io_service, std::bind(&avproto_wrapper::connect_coroutine, this, std::placeholders::_1));
	}
	
	void avproto_wrapper::connect_coroutine(boost::asio::yield_context yield_context)
	{
		boost::system::error_code ec;
		using namespace boost::asio::ip;
#if 0
		tcp::resolver resolver(m_io_service);

		auto _debug_host = getenv("AVIM_HOST");
		tcp::resolver::query query(_debug_host?_debug_host:"avim.avplayer.org", "24950");

		auto endpoint_iterator = resolver.async_resolve(query, yield_context[ec]);

		if (ec || endpoint_iterator == tcp::resolver::iterator())
		{
			std::cout << "server not found" << std::endl;
			return;
		}

		m_socket.reset(new tcp::socket(m_io_service));

		boost::asio::async_connect(*m_socket, endpoint_iterator, yield_context[ec]);

		if (ec)
		{
			std::cout << "connection failed, msg: " << ec.message() << std::endl;
			return;
		}
		std::cout << "connection established " << std::endl;
		//m_avif.reset(new avjackif(m_socket));
#endif
		m_avif.reset(new avjackif(m_io_service));
		boost::asio::spawn(m_io_service, std::bind(&avproto_wrapper::login_coroutine, this, std::placeholders::_1));
	}
	
	bool avproto_wrapper::login_coroutine(boost::asio::yield_context yield_context)
	{
		boost::shared_ptr<BIO> keyfile(BIO_new_mem_buf(&m_key[0], m_key.length()), BIO_free);
		boost::shared_ptr<BIO> certfile(BIO_new_mem_buf(&m_crt[0], m_crt.length()), BIO_free);

		std::shared_ptr<RSA> m_rsa_key;
		std::shared_ptr<X509> m_x509_cert;
		m_rsa_key.reset(
		PEM_read_bio_RSAPrivateKey(keyfile.get(), 0, 0, 0), //(pem_password_cb*)pass_cb,(void*) key.c_str()),
		RSA_free
		);

		m_x509_cert.reset(PEM_read_bio_X509(certfile.get(), 0, 0, 0), X509_free);
		m_avif->set_pki(m_rsa_key, m_x509_cert);
		auto _debug_host = getenv("AVIM_HOST");
		bool ret = m_avif->async_connect(_debug_host?_debug_host:"avim.avplayer.org", "24950", yield_context);
		std::cout << "connect ok" << std::endl;
		//bool ret = m_avif->async_connect("127.0.0.1", "24950", yield_context);
		if (m_avif->async_handshake(yield_context))
		{
			std::cout << "login success " << std::endl;
		}
		
		m_avkernel.add_interface(m_avif);
		std::string me_addr = av_address_to_string(*m_avif->if_address());
		m_avkernel.add_route(".+@.+", me_addr, m_avif->get_ifname(), 100);
		
		// start message_receiver
		boost::asio::spawn(m_io_service, std::bind(&avproto_wrapper::handle_message, this, std::placeholders::_1));
		
		m_service.get()->notify(0, 1, 0);
		return true;
	}
	
	bool avproto_wrapper::handle_message(boost::asio::yield_context yield_context)
	{
		for(;;)
		{
			std::string target,data;
			m_avkernel.async_recvfrom(target, data, yield_context);
			m_service.get()->handle_message(0,target, decode_control_message(data));
		}
		return true;
	}
	
	bool avproto_wrapper::write_packet(std::string target, std::string &pkt)
	{
		m_avkernel.async_sendto(target, pkt, [](boost::system::error_code ec){
			if(ec)
				std::cout << "send failed, msg: " << ec.message() << std::endl;
			else
				std::cout << "send ok" << std::endl;
		});
	}
	
}
