/**************************************************************************
*   Copyright (C) 2004-2007 by Michael Medin <michael@medin.name>         *
*                                                                         *
*   This code is part of NSClient++ - http://trac.nakednuns.org/nscp      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#pragma once

#include <boost/tuple/tuple.hpp>

#include <nscapi/nscapi_protobuf.hpp>
#include <client/command_line_parser.hpp>
#include <nscapi/nscapi_targets.hpp>
#include <nscapi/nscapi_protobuf_types.hpp>
#include <socket/client.hpp>

#include <nrpe/packet.hpp>
#include <nrpe/client/nrpe_client_protocol.hpp>

namespace nrpe_client {

	const std::string command_prefix("nrpe");
	const std::string default_command("query");

	struct custom_reader {
		typedef nscapi::targets::target_object object_type;
		typedef nscapi::targets::target_object target_object;

		static void init_default(target_object &target);
		static void add_custom_keys(nscapi::settings_helper::settings_registry &settings, boost::shared_ptr<nscapi::settings_proxy> proxy, object_type &object, bool is_sample);
		static void post_process_target(target_object &target);
	};

	struct connection_data : public socket_helpers::connection_info {
		int buffer_length;

		connection_data(nscapi::protobuf::types::destination_container arguments, nscapi::protobuf::types::destination_container target) {
			arguments.import(target);
			address = arguments.address.host;
			port_ = arguments.address.get_port_string("5666");
			ssl.enabled = arguments.get_bool_data("ssl");
			ssl.certificate = arguments.get_string_data("certificate");
			ssl.certificate_key = arguments.get_string_data("certificate key");
			ssl.certificate_key_format = arguments.get_string_data("certificate format");
			ssl.ca_path = arguments.get_string_data("ca");
			ssl.allowed_ciphers = arguments.get_string_data("allowed ciphers");
			ssl.dh_key = arguments.get_string_data("dh");
			ssl.verify_mode = arguments.get_string_data("verify mode");
			timeout = arguments.get_int_data("timeout", 30);
			retry = arguments.get_int_data("retry", 2);
			buffer_length = arguments.get_int_data("payload length", 1024);

			if (arguments.has_data("no ssl"))
				ssl.enabled = !arguments.get_bool_data("no ssl");
			if (arguments.has_data("use ssl"))
				ssl.enabled = arguments.get_bool_data("use ssl");


		}

		std::string to_string() const {
			std::stringstream ss;
			ss << "host: " << get_endpoint_string();
			ss << ", buffer_length: " << buffer_length;
			ss << ", ssl: " << ssl.to_string();
			return ss.str();
		}
	};

	struct target_handler : public client::target_lookup_interface {
		target_handler(const nscapi::targets::handler<nrpe_client::custom_reader> &targets) : targets_(targets) {}
		nscapi::protobuf::types::destination_container lookup_target(std::string &id) const;
		bool apply(nscapi::protobuf::types::destination_container &dst, const std::string key);
		bool has_object(std::string alias) const;
		const nscapi::targets::handler<nrpe_client::custom_reader> &targets_;
	};
	struct clp_handler_impl : public client::clp_handler {
		boost::shared_ptr<socket_helpers::client::client_handler> client_handler;
		clp_handler_impl(boost::shared_ptr<socket_helpers::client::client_handler> client_handler) : client_handler(client_handler) {}

		int query(client::configuration::data_type data, const Plugin::QueryRequestMessage &request_message, Plugin::QueryResponseMessage &response_message);
		int submit(client::configuration::data_type data, const Plugin::SubmitRequestMessage &request_message, Plugin::SubmitResponseMessage &response_message);
		int exec(client::configuration::data_type data, const Plugin::ExecuteRequestMessage &request_message, Plugin::ExecuteResponseMessage &response_message);

		boost::tuple<int,std::string> send(nrpe_client::connection_data con, const std::string data);

	};


	void setup(client::configuration &config, const ::Plugin::Common_Header& header);

}

