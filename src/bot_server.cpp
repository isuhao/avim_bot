#include "logging.hpp"
#include "bot_server.hpp"

namespace bot_avim {

	bot_server::bot_server(boost::shared_ptr<RSA> rsa, boost::shared_ptr<X509> x509_cert, bot_role role)
	: m_ca(rsa, x509_cert)
	, m_role(role)
	{
		LOG_DBG << "bot server constructor";
		m_ios.reset(new boost::asio::io_service);
		
	}

	bot_server::~bot_server()
	{}
	
	void bot_server::start()
	{
		//TODO
		//Bind local port
		//Login
		m_ios->run();
	}

	void bot_server::stop()
	{
		m_ios->stop();
	}

	bool bot_server::write_packet()
	{
	}
	
	bool bot_server::handle_packet()
	{
	}

	
	bool bot_server::add_bot(const std::string& name, bot_role type)
	{
		return true;
	}

	bool bot_server::del_bot(const std::string& name)
	{
		return true;
	}

	void bot_server::continue_timer()
	{
	}
	
	void bot_server::dump_status()
	{
	}
	
}