#pragma once
#include "json_head.hpp"
#include <optional>

namespace Pars
{
    namespace TG
    {
        using optstrw = std::optional<json::string_view>;
        using optbool = std::optional<bool>;
        using optstr  = std::optional<json::string>;
        using optint  = std::optional<int64_t>;
        using optuint = std::optional<uint64_t>;
        using op      = std::pair<json::string,std::optional<json::value>>;
        using p       = std::pair<json::string,json::value>;



        struct TelegramRequestes : MainParser
        {

            [[nodiscard]]
            static json::value
            setWebhook
            (
                json::string_view url,
                optstr certificate = {},
                optstr ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstr  secret_token         = {}
            )
            {
                json::object ob(ptr_);
                json::object ob1(ptr_);
                json::object ob2(ptr_);

                ob = parse_ObjPairs_as_obj
                   (
                        p {"url", url}
                   );


                ob1 = parse_OptPairs_to_obj
                    (
                        op{"certificate", certificate},
                        op{"ip_address",  ip_address},
                        op{"max_connections", max_connections},
                        op{"allowed_updates", allowed_updates},
                        op{"drop_pending_updates", drop_pending_updates},
                        op{"secret_token", secret_token}
                    ); 

                ob.insert(ob1.begin(), ob1.end());

                ob2["setWebhook"] = { std::move(ob) };

                return ob2;
            }


            [[nodiscard]]
            static json::value
            get_webhook_request
            (
                json::string_view url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstrw ip_address                             = {},
                optint  last_error_date                        = {},
                optstrw last_error_message                     = {},
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
                        p {"url",url},
                        p {"has_custom_certificate",has_custom_certificate},
                        p {"pending_update_count", pending_update_count}
                    );


                ob2 = parse_OptPairs_to_obj
                    (
                        op {"ip_address",ip_address},
                        op {"last_error_date",last_error_date},
                        op {"last_error_message", last_error_message},
                        op {"last_synchronization_error_date",last_synchronization_error_date},
                        op {"max_connections", max_connections}
                    );


                json::array arr;
                if (allowed_updates.has_value())
                {
                    auto& vec = allowed_updates.value();
                    arr = parse_all_json_as_array(vec.begin(), vec.end());
                }

                ob1.insert(ob2.begin(), ob2.end());
                
                ob2.clear();
                ob2["allowed_updates"] = std::move(arr);

                ob1.insert(ob2.begin(), ob2.end());

                ob ["get_webhook_request"] = { std::move(ob1) };

                return ob;
            }


            [[nodiscard]]
            static json::value
            get_user_request
            (
                uint64_t id,
                bool is_bot,
                json::string_view first_name,
                optstrw last_name                   = {},
                optstrw username                    = {},
                optstrw language_code               = {},
                optbool is_premium                  = {},
                optbool added_to_attachment_menu    = {},
                optbool can_join_groups             = {},
                optbool can_read_all_group_messages = {},
                optbool supports_inline_queries     = {},
                optbool can_connect_to_business     = {}
            )
            {
                json::object ob   {ptr_};
                json::object ob_1 {ptr_};
                json::object ob_2 {ptr_};

                ob_1 =  parse_ObjPairs_as_obj
                        (
                            p{"id",id},
                            p{"is_bot",is_bot},
                            p{"first_name",  first_name}
                        );

                ob_2 =  parse_OptPairs_to_obj
                        (
                            op {"last_name", last_name},
                            op {"username",  username},
                            op {"language_code", language_code},
                            op {"is_premium", is_premium},
                            op {"added_to_attachment_menu",added_to_attachment_menu},
                            op {"can_join_groups",can_join_groups},
                            op {"can_read_all_group_messages", can_read_all_group_messages},
                            op {"supports_inline_queries", supports_inline_queries},
                            op {"can_connect_to_business", can_connect_to_business}    
                        );


                ob_1.insert(ob_2.begin(),ob_2.end());

                ob["user"] = { ob_1 };

                return ob;
            }

        };


        template<typename Derived>
        struct TelegramEntities
        {
            public:

            [[nodiscard]]
            std::optional<std::unordered_map<json::string, json::value>>  
            requested_fields(const json::value& val)
            {
                return static_cast<Derived&>(*this).requested_fields(val); 
            }

            [[nodiscard]]
            std::unordered_map<json::string, json::value> 
            optional_fields(const json::value& val)
            {
                return static_cast<Derived&>(*this).optional_fields(val);
            }

            [[nodiscard]]
            Derived
            fields_from_map(const std::unordered_map<json::string, json::value>& map)
            {
                return static_cast<Derived&>(*this).fields_from_map(map);
            }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static std::optional<Derived>
            get_request (T && val) noexcept
            {
                using namespace boost;

                system::error_code er;

                if( val.is_object() == false)
                {
                    return std::nullopt;
                }

                json::object obj = val.as_object();
                auto it = obj.find("user");

                if(it == obj.end())
                {
                    return std::nullopt;
                }

                return Derived{};
            }


            [[nodiscard]]
            json::value
            entity_to_value()
            {
                return static_cast<Derived&>(*this).fields_to_value();
            }


            [[nodiscard]]
            static
            std::optional<std::unordered_map<json::string, json::value>>
            verify_fields(const json::value& val)
            {
                Derived der{};

                auto req_map = der.requested_fields(val);
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = der.optional_fields(val);

                auto map = std::move(req_map.value());

                for(auto && i : opt_map)
                {
                    map.insert_or_assign(std::move(i.first), std::move(i.second));
                }

                return map; 
            }


            virtual ~TelegramEntities() = 0;
        };

        template<typename Derived>
        TelegramEntities<Derived>::~TelegramEntities(){}



        struct SetWebHook : TelegramEntities<SetWebHook>
        {
            public:

            json::string url;
            optstr certificate = {};
            optstr ip_address  = {};
            optint max_connections = {};
            std::optional<json::array> allowed_updates = {};
            optbool drop_pending_updates = {};
            optstr  secret_token         = {};

            public:

            SetWebHook(){}

            SetWebHook
            (
                json::string_view url,
                optstr certificate = {},
                optstr ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstr  secret_token         = {}
            )
            :
              url(url),
              certificate(certificate),
              ip_address(ip_address),
              max_connections(max_connections),
              allowed_updates(allowed_updates),
              drop_pending_updates(drop_pending_updates),
              secret_token(secret_token)
            {
                
            }

            public:

            [[nodiscard]]
            std::optional<std::unordered_map<json::string, json::value>> 
            requested_fields(const json::value& val) 
            {
                const size_t sz = 1;

                auto opt = MainParser::check_pointer_validation(val, std::make_pair("/setWebhook", json::kind::object));
                if(opt.has_value() == false)
                {
                    return std::nullopt;
                }
                
                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/setWebhook/url", json::kind::uint64)
                );

                if(map.size() < sz)
                {
                    return std::nullopt;
                }

                return map;
            }


            [[nodiscard]]
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/setWebhook/certificate", json::kind::string),
                    std::make_pair("/setWebhook/ip_address", json::kind::string),
                    std::make_pair("/setWebhook/max_connections", json::kind::int64),
                    std::make_pair("/setWebhook/allowed_updates", json::kind::array),
                    std::make_pair("/setWebhook/drop_pending_updates", json::kind::bool_),
                    std::make_pair("/setWebhook/secret_token", json::kind::string)
                );

                return map;
            } 
            

            [[nodiscard]]
            void fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("url", std::ref(url)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("certificate", std::ref(certificate)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("ip_address", std::ref(ip_address)));

                MainParser::field_from_map
                <json::kind::int64>(map, std::make_pair("max_connections", std::ref(max_connections)));

                MainParser::field_from_map
                <json::kind::array>(map, std::make_pair("allowed_updates", std::ref(allowed_updates)));

                MainParser::field_from_map
                <json::kind::bool_>(map, std::make_pair("drop_pending_updates", std::ref(drop_pending_updates)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("secret_token", std::ref(secret_token)));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::setWebhook
                (
                    url,
                    certificate,
                    ip_address,
                    max_connections,
                    allowed_updates,
                    drop_pending_updates,
                    secret_token
                );
            }

        };



        struct User : TelegramEntities<User>
        {
            public:

            uint64_t id;
            bool is_bot;
            json::string first_name;
            optstr  last_name     = {};
            optstr  username      = {};
            optstr  language_code = {};
            optbool is_premium    = {};
            optbool added_to_attachment_menu     = {};
            optbool can_join_groups              = {};
            optbool can_read_all_group_messages  = {};
            optbool supports_inline_queries      = {};
            optbool can_connect_to_business      = {};

            public:

            User(){}

            User
            (
                uint64_t id,
                bool is_bot,
                json::string_view first_name,
                optstrw last_name     = {},
                optstrw username      = {},
                optstrw language_code = {},
                optbool is_premium    = {},
                optbool added_to_attachment_menu        = {},
                optbool can_join_groups                 = {},
                optbool can_read_all_group_messages     = {},
                optbool supports_inline_queries         = {},
                optbool can_connect_to_business         = {}
            )
            :
                id(id),
                is_bot(is_bot),
                first_name(first_name),
                last_name(last_name),
                username (username),
                language_code(language_code),
                is_premium(is_premium),
                added_to_attachment_menu(added_to_attachment_menu),
                can_join_groups(can_join_groups),
                can_read_all_group_messages(can_read_all_group_messages),
                supports_inline_queries(supports_inline_queries),
                can_connect_to_business(can_connect_to_business)
            {

            } 


            public:


            [[nodiscard]]
            std::optional<std::unordered_map<json::string, json::value>> 
            requested_fields(const json::value& val) 
            {
                const size_t sz = 3;

                auto opt = MainParser::check_pointer_validation(val, std::make_pair("/user", json::kind::object));
                if(opt.has_value() == false)
                {
                    return std::nullopt;
                }
                

                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/user/id", json::kind::uint64),
                    std::make_pair("/user/is_bot", json::kind::bool_),
                    std::make_pair("/user/first_name", json::kind::string)
                );

                if(map.size() < sz)
                {
                    return std::nullopt;
                }

                return map;
            }


            [[nodiscard]]
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/user/last_name", json::kind::string),
                    std::make_pair("/user/username",  json::kind::string),
                    std::make_pair("/user/language_code", json::kind::string),
                    std::make_pair("/user/is_premium", json::kind::bool_),
                    std::make_pair("/user/added_to_attachment_menu", json::kind::bool_),
                    std::make_pair("/user/can_join_groups", json::kind::bool_),
                    std::make_pair("/user/can_read_all_group_messages", json::kind::bool_),
                    std::make_pair("/user/supports_inline_queries", json::kind::bool_),
                    std::make_pair("/user/can_connect_to_business", json::kind::bool_)
                );

                return map;
            }


            void fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::uint64>(  map,  std::make_pair("id", std::ref(id)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("is_bot", std::ref(is_bot)));

                MainParser::field_from_map
                <json::kind::string>(  map,  std::make_pair("first_name", std::ref(first_name)));

                MainParser::field_from_map
                <json::kind::string>(  map,  std::make_pair("last_name", std::ref(last_name)));

                MainParser::field_from_map
                <json::kind::string>(  map,  std::make_pair("username",  std::ref(username)));

                MainParser::field_from_map
                <json::kind::string>(  map,  std::make_pair("language_code", std::ref(language_code)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("is_premium", std::ref(is_premium)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("added_to_attachment_menu", std::ref(added_to_attachment_menu)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("can_join_groups", std::ref(can_join_groups)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("can_read_all_group_messages", std::ref(can_read_all_group_messages)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("supports_inline_queries", std::ref(supports_inline_queries)));

                MainParser::field_from_map
                <json::kind::bool_>(   map,  std::make_pair("can_connect_to_business", std::ref(can_connect_to_business)));
            }

            public:

            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::get_user_request
                (
                    id,
                    is_bot,
                    first_name,
                    last_name,
                    username,
                    language_code,
                    is_premium,
                    added_to_attachment_menu,
                    can_join_groups,
                    can_read_all_group_messages,
                    supports_inline_queries,
                    can_connect_to_business
                );
            }

        };



        struct Webhook : TelegramEntities<Webhook>
        {
            public:

            json::string url;
            bool has_custom_certificate;
            uint64_t pending_update_count;
            optstrw ip_address         = {};
            optint  last_error_date    = {};
            optstrw last_error_message = {};
            optint last_synchronization_error_date         = {};         
            optint max_connections                         = {};
            std::optional<std::vector<json::string>> allowed_updates = {}; 

            public:

            Webhook() {}

            Webhook
            (
                json::string_view url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstrw ip_address         = {},
                optint  last_error_date    = {},
                optstrw last_error_message = {},
                optint last_synchronization_error_date         = {},
                optint max_connections                         = {},
                std::optional<std::vector<json::string>> allowed_updates = {}  
            )
            :
                url(url),
                has_custom_certificate(has_custom_certificate),
                pending_update_count(pending_update_count),
                ip_address(ip_address),
                last_error_date(last_error_date),
                last_error_message(last_error_message),
                last_synchronization_error_date(last_synchronization_error_date),
                max_connections(max_connections),
                allowed_updates(std::move(allowed_updates))
            {

            }

            private:

            [[nodiscard]]
            json::value
            fields_to_value() 
            {
                return TelegramRequestes::get_webhook_request
                (
                    url,
                    has_custom_certificate,
                    pending_update_count,
                    ip_address,
                    last_error_date,
                    last_error_message,
                    last_synchronization_error_date,
                    max_connections,
                    allowed_updates
                );
            }

        };

    };//namespace TG

};//namespace Pars