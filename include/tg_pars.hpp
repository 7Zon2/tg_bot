#pragma once
#include "json_head.hpp"
#include <optional>
#include "entities/concept_entities.hpp"
#include <boost/algorithm/string.hpp>

template<typename T, typename U>
constexpr auto&& forward_like(U&& u) noexcept
{
    constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
    if constexpr (std::is_lvalue_reference_v<T&&>)
    {
        if constexpr (is_const)
            return std::as_const(u);
        else
            return static_cast<U&>(u);
    }
    else
    {
        if constexpr (is_const)
            return std::move(std::as_const(u));
        else
            return std::move(u);
    }
}


namespace Pars
{
    namespace TG
    {   
        using optarray  = std::optional<json::array>;
        using optobj    = std::optional<json::object>;
        using optstrw   = std::optional<json::string_view>;
        using optbool   = std::optional<bool>;
        using optstr    = std::optional<json::string>;
        using optint    = std::optional<int64_t>;
        using optuint   = std::optional<uint64_t>;
        using optdouble = std::optional<double>; 
        using op        = std::pair<json::string,std::optional<json::value>>;
        using p         = std::pair<json::string,json::value>;

        
        #define FIELD_NAME(field)  #field

        #define FIELD_TO_LOWER(field) boost::algorithm::to_lower_copy(json::string{FIELD_NAME(field)})

        #define FIELD_EQUAL(field) FIELD_NAME(field)"="

        #define JS_POINTER(method, field) boost::algorithm::to_lower_copy(json::string{"/"#method"/"#field})

        #define MAKE_PAIR(field) std::make_pair(FIELD_NAME(field), std::ref(field))

        #define MAKE_OP(field)  op{FIELD_NAME(field), field}

        #define PAIR(field)     p{FIELD_NAME(field), field}

        #define URL_USER_INFO(field, value) "@"#field":" value

        #define URL_FIELD(field, value)     #field"=" value

        #define URL_REQUEST(field) "/"#field"?"


        struct TelegramRequestes : MainParser
        {
            protected:

            [[nodiscard]]
            static json::value
            MessageOrigin
            (
                json::string_view type,
                size_t date
            )
            {
                json::object ob(ptr_);
                ob["messageorigin"] = parse_ObjPairs_as_obj(PAIR(type), PAIR(date)); 
                return ob;
            }

            public:

            [[nodiscard]]
            static json::value
            getUpdates
            (
                optint offset = {},
                optint limit  = {},
                optint timeout= {},
                std::optional<std::vector<json::string>> allowed_updates = {}
            )
            {
                json::object ob(ptr_);
                ob = parse_OptPairs_as_obj
                (
                    MAKE_OP(offset),
                    MAKE_OP(limit),
                    MAKE_OP(timeout)        
                );

                json::array arr;
                if(allowed_updates.has_value())
                {
                    for(auto&& i : allowed_updates.value())
                    {
                        arr.push_back(std::move(i));
                    }
                }

                ob.insert_or_assign(FIELD(allowed_updates), std::move(arr));

                json::object ob2(ptr_);
                ob2["getupdates"] = std::move(ob);
                return ob2;
            }


            [[nodiscard]]
            static json::value
            chat
            (
                uint64_t id,
                json::string_view type,
                optstrw title = {},
                optstrw username = {},
                optstrw first_name = {},
                optstrw last_name  = {},
                optbool is_forum   = {}
            )
            {
                json::object ob(ptr_);
                ob = parse_ObjPairs_as_obj
                (
                    PAIR(id),
                    PAIR(type)
                );

                json::object ob2(ptr_);
                ob2 = parse_OptPairs_as_obj
                (
                    MAKE_OP(title),
                    MAKE_OP(username),
                    MAKE_OP(first_name),
                    MAKE_OP(last_name),
                    MAKE_OP(is_forum)
                );

                Pars::obj_move(ob2, ob);

                json::object res(ptr_);
                res[FIELD_TO_LOWER(chat)] = std::move(ob);
                return res;
            }


            template<TG::is_user T>
            [[nodiscard]]
            static json::value
            MessageOriginUser
            (
                json::string_view type,
                size_t date,
                T&& user
            )
            {
                auto ob  = MessageOrigin(type, date).as_object();
                auto ob2 = forward_like<T>(user.fields_to_value().as_object());

                Pars::MainParser::container_move(std::move(ob2), ob);                                                    

                json::object res(ptr_);
                res["messageoriginuser"] = std::move(ob);

                return res;
            }


            [[nodiscard]]
            static json::value
            MessageOriginHiddenUser
            (
                json::string_view type,
                size_t date,
                json::string_view sender_user_name
            )
            {
                auto ob = MessageOrigin(type, date).as_object();
                ob.emplace(FIELD(sender_user_name), sender_user_name);

                json::object res(ptr_);
                res["messageoriginhiddenuser"] = std::move(ob);
                return res;
            }


            template<TG::is_chat T>
            [[nodiscard]]
            static json::value
            MessageOriginChat
            (
                json::string_view type,
                size_t date,
                T &&sender_chat,
                optstrw author_signature = {}
            )
            {
                auto ob = MessageOrigin(type, date).as_object();
                auto send_ob = forward_like<T>(sender_chat.fields_to_value().as_object()); 

                Pars::MainParser::container_move(std::move(send_ob), ob)

                if (author_signature.has_value())
                    ob.emplace(FIELD(author_signature), author_signature.value());

                json::object res(ptr_);
                res["messageoriginchat"] = std::move(ob); 
                return res;
            }


            template<is_chat T>
            [[nodiscard]]
            static json::value
            MessageOriginChannel
            (
                json::string_view type,
                size_t date,
                T&& chat,
                size_t message_id,
                optstrw author_signature = {}

            )
            {
                auto ob = MessageOriginChat(type, date, std::forward<T>(chat), author_signature).as_object();
                ob.emplace("message_id", message_id);

                json::object res(ptr_);
                res["messageoriginchannel"] = std::move(ob);
                return res;
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
                optarray result
            )
            {
                json::object ob(ptr_);
                ob = parse_ObjPairs_as_obj(PAIR(ok));

                json::object ob2(ptr_);
                ob2 = parse_OptPairs_as_obj
                (
                    MAKE_OP(error_code),
                    MAKE_OP(std::move(description))
                );

                Pars::MainParser::container_move(std::move(ob2), ob);

                if (result.has_value())
                {
                    json::value val_arr{std::move(result.value())};
                    ob.emplace(FIELD_NAME(result), std::move(val_arr));
                }
                return ob;
            }


            [[nodiscard]]
            static json::value
            setWebhook
            (
                json::string_view url,
                optstrw certificate = {},
                optstrw ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstrw  secret_token         = {}
            )
            {
                json::object ob(ptr_);
                json::object ob1(ptr_);
                json::object ob2(ptr_);

                ob = parse_ObjPairs_as_obj
                   (
                        PAIR(url)
                   );


                ob1 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(certificate),
                        MAKE_OP(ip_address),
                        MAKE_OP(max_connections),
                        MAKE_OP(std::move(allowed_updates)),
                        MAKE_OP(drop_pending_updates),
                        MAKE_OP(secret_token)
                    ); 

                Pars::MainParser::container_move(std::move(ob1), ob);

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
                        PAIR(url),
                        PAIR(has_custom_certificate),
                        PAIR(pending_update_count)
                    );


                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(ip_address),
                        MAKE_OP(last_error_date),
                        MAKE_OP(last_error_message),
                        MAKE_OP(last_synchronization_error_date),
                        MAKE_OP(max_connections)
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

                Pars::obj_move(std::move(ob2), ob1);

                ob ["getwebhookrequest"] = { std::move(ob1) };
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
                            PAIR(id),
                            PAIR(is_bot),
                            PAIR(first_name)
                        );

                ob_2 =  parse_OptPairs_as_obj
                        (
                            MAKE_OP(last_name),
                            MAKE_OP(username),
                            MAKE_OP(language_code),
                            MAKE_OP(is_premium),
                            MAKE_OP(added_to_attachment_menu),
                            MAKE_OP(can_join_groups),
                            MAKE_OP(can_read_all_group_messages),
                            MAKE_OP(supports_inline_queries),
                            MAKE_OP(can_connect_to_business)    
                        );

                Pars::MainParser::container_move(std::move(ob_2), ob_1);

                ob["user"] = { std::move(ob_1) };
                return ob;
            }


            [[nodiscard]]
            static json::value
            forwardMessage
            (
                json::string_view chat_id,
                json::string_view from_chat_id,
                int message_id,
                optint message_thread_id,
                optbool disable_notification,
                optbool protect
            )
            {
                json::object ob{ptr_};
                json::object ob_1(ptr_);

                ob_1 =  parse_ObjPairs_as_obj
                    (
                        PAIR(chat_id),
                        PAIR(from_chat_id),
                        PAIR(message_id)
                    );

                json::object ob_2{ptr_};
                ob_2 = parse_OptPairs_as_obj
                      (
                        MAKE_OP(message_thread_id),
                        MAKE_OP(disable_notification),
                        MAKE_OP(protect)
                      );

                
                Pars::MainParser::container_move(std::move(ob_2), ob_1);

                ob["forwardmessage"] = { std::move(ob_1) };
                return ob;
            }


            [[nodiscard]]
            json::value
            linkPreviewOptions
            (
                optbool is_disabled = {},
                optstrw url = {},
                optbool prefer_small_media = {},
                optbool prefer_large_media = {},
                optbool show_above_text = {}
            )
            {
                json::object ob{ptr_};
                ob = parse_OptPairs_as_obj
                    (
                        MAKE_OP(is_disabled),
                        MAKE_OP(url),
                        MAKE_OP(prefer_small_media),
                        MAKE_OP(prefer_large_media),
                        MAKE_OP(show_above_text)
                    );

               json::object ob_2(ptr_);
               ob_2["linkpreviewoptions"] = std::move(ob);
               return ob_2;
            }


            [[nodiscard]]
            json::value
            PhotoSize
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double width,
                double height,
                optdouble file_size = {}
            )
            {
                json::object ob{ptr_};
                ob = parse_ObjPairs_as_obj
                    (
                        PAIR(file_id),
                        PAIR(file_unique_id),
                        PAIR(width),
                        PAIR(height)
                    );

                json::object ob2{ptr_};
                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(file_size)
                    );

                Pars::MainParser::container_move(std::move(ob2), ob);

                json::object res_(ptr_);
                res["photosize"] = std::move(ob);
                return res;
            }


            [[nodiscard]]
            json::value
            Animation
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double width,
                double height,
                double duration,
                std::optional<PhotoSize> thumbnail = {},
                optstrw file_name =  {},
                optstrw mime_type =  {},
                optdouble file_size = {}
            )
            {
                json::object ob{ptr_};
                ob = parse_ObjPairs_as_obj
                    (
                        PAIR(file_id),
                        PAIR(file_unique_id),
                        PAIR(width),
                        PAIR(height),
                        PAIR(duration)
                    );

                json::object ob2(ptr_);
                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(file_name),
                        MAKE_OP(mime_type),
                        MAKE_OP(file_size)
                    );

                if (thumbnail.has_value())
                {
                    json::object ob2(ptr_);
                    auto && ref_thumbnail = std::move(thumbnail.value());
                    ob2 = std::move(ref_thumbnail.fields_to_value()).as_object();

                    Pars::obj_move(ob2, ob);
                }

                Pars::MainParser::container_move(std::move(ob), photo_ob);

                json::object res(ptr_);
                res["animation"] = std::move(photo_ob);
                return res;
            }


            
            [[nodiscard]]
            json::value
            Audio
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double duration,
                optstrw performer = {},
                optstrw title = {},
                optstrw file_name = {},
                optstrw mime_type = {},
                std::optional<PhotoSize> thumbnail = {},
                optdouble file_size = {}

            )
            {
                json::object ob = Animation
                (
                    file_id,
                    file_unique_id,
                    0,
                    0,
                    duration,
                    thumbnail,
                    file_name,
                    mime_type,
                    file_size
                );

                json::object ob2(ptr_);
                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(title),
                        MAKE_OP(performer)
                    );

                MainParser::container_move(std::move(ob2), ob);
                json::object res(ptr_);
                res["audio"] = std::move(ob);
                return res;
            }

        };
    };//namespace TG

};//namespace Pars