#pragma once
#include "json_head.hpp"


namespace Pars
{
    namespace TG
    {   
        struct TelegramRequestes : MainParser
        {

            public:

            [[nodiscard]]
            static json::value
            get_webhook_request
            (
                json::string url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstr ip_address                             = {},
                optint  last_error_date                        = {},
                optstr last_error_message                     = {},
                optint last_synchronization_error_date         = {},
                optint max_connections                         = {},
                std::optional<std::vector<json::string>> allowed_updates = {}  
            )
            {
                json::object ob (ptr_);
                json::object ob1(ptr_);
                json::object ob2(ptr_);


                ob1 = parse_ObjPairs_as_obj
                    (
                        PAIR(url, std::move(url)),
                        PAIR(has_custim_certificate, has_custom_certificate),
                        PAIR(pending_update_count, pending_update_count)
                    );


                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(ip_adress, std::move(ip_address)),
                        MAKE_OP(last_error_date, last_error_date),
                        MAKE_OP(last_error_message, std::move(last_error_message)),
                        MAKE_OP(last_synchronization_error_date, last_synchronization_error_date),
                        MAKE_OP(max_connections, max_connections)
                    );


                json::array arr;
                if (allowed_updates.has_value())
                {
                    auto& vec = allowed_updates.value();
                    arr = parse_all_json_as_array(vec.begin(), vec.end());
                }

                Pars::MainParser::container_move(std::move(ob2), ob1);
                ob2.clear();

                ob2["allowed_updates"] = std::move(arr);

                MainParser::container_move(std::move(ob2), ob1);

                ob ["getwebhookrequest"] = { std::move(ob1) };
                return ob;
            }


        };
    };//namespace TG

};//namespace Pars