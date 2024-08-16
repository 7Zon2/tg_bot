#pragma once
#include "json_head.hpp"
#include <optional>

namespace Pars
{
    namespace TG
    {
        using optobj  = std::optional<json::object>;
        using optstrw = std::optional<json::string_view>;
        using optbool = std::optional<bool>;
        using optstr  = std::optional<json::string>;
        using optint  = std::optional<int64_t>;
        using optuint = std::optional<uint64_t>;
        using op      = std::pair<json::string,std::optional<json::value>>;
        using p       = std::pair<json::string,json::value>;


        #define FIELD_NAME(field) #field

        #define JS_POINTER(method, field) "/"#method"/"#field

        #define MAKE_PAIR(field) std::make_pair(FIELD_NAME(field), std::ref(field))

        #define MAKE_OP(field) op{FIELD_NAME(field), field}



        struct TelegramRequestes : MainParser
        {

            [[nodiscard]]
            static json::value
            getUpdates
            (
                optint offset = {},
                optint limit  = {},
                optint timeout= {},
                std::optional<json::array> allowed_updates = {}
            )
            {
                json::object ob(ptr_);
                ob = parse_OptPairs_as_obj
                (
                    MAKE_OP(offset),
                    MAKE_OP(limit),
                    MAKE_OP(timeout),
                    MAKE_OP(allowed_updates)           
                );

                json::object ob2(ptr_);
                ob2["getipdates"] = std::move(ob);
                
                return ob2;
            }


            [[nodiscard]]
            static json::value
            deletewebhook
            (
                bool drop_pending_updates
            )
            {
                json::object ob(ptr_);
                ob["drop_pending_updates"] = drop_pending_updates;
                return ob;
            }


            [[nodiscard]]
            static json::value
            TelegramResponse
            (
                bool ok,
                optint error_code,
                optstrw description,
                optobj result
            )
            {
                json::object ob(ptr_);
                ob = parse_ObjPairs_as_obj(p{"ok", ok});

                json::object ob2(ptr_);
                ob2 = parse_OptPairs_as_obj
                (
                    op("error_code",  error_code),
                    op("description", description)
                );

                ob.insert(ob2.begin(), ob2.end());

                if (result.has_value())
                {
                    json::object res = std::move(result.has_value());
                    ob.insert(res.begin(), res.end());
                }
                return ob;
            }


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


                ob1 = parse_OptPairs_as_obj
                    (
                        op{"certificate", certificate},
                        op{"ip_address",  ip_address},
                        op{"max_connections", max_connections},
                        op{"allowed_updates", allowed_updates},
                        op{"drop_pending_updates", drop_pending_updates},
                        op{"secret_token", secret_token}
                    ); 

                ob.insert(ob1.begin(), ob1.end());

                ob2["setwebhook"] = { std::move(ob) };

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


                ob2 = parse_OptPairs_as_obj
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

                ob_2 =  parse_OptPairs_as_obj
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
    };//namespace TG

};//namespace Pars