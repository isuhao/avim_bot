#pragma once

#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "packet.pb.h"
#include "avproto/serialization.hpp"
#include "avproto_wrapper.hpp"

#include "bot_service.hpp"

namespace bot_avim {

	typedef std::vector<std::string> member_ptr;

	typedef enum
	{
		GROUP_STATUS_OFFLINE  = 0,
		GROUP_STATUS_ONLINE   = 1,
	}group_status;

	// CMD START FROM  0X100
	const int CMD_GROUP_TEST = 0x200;

	class bot_group
		: public bot_service
	{
	public:
		explicit bot_group(boost::asio::io_service& io_service, std::shared_ptr<RSA> key, std::shared_ptr<X509> crt);
		~bot_group();

	public:
		bool add_member(const std::string& name);
		bool del_member(const std::string& name);

		bool handle_message(const std::string& sender, const std::string& content);
		bool handle_message(int type, std::string &sender, std::shared_ptr<google::protobuf::Message> msg_ptr);
		bool status_changed(int status);
		bool notify(int cmd, int ext1, int ext2);

		void dump_status();

	private:
		void notify(boost::system::error_code ec);

	private:
		member_ptr m_group_member_list;
		group_status m_status;
		boost::shared_ptr<avproto_wrapper> m_avproto;
	};

}
