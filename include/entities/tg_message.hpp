#pragma once 
#include "TelegramEntities.hpp"
#include "User.hpp"
#include "Chat.hpp"
#include "Document.hpp"
#include "PhotoSize.hpp"
#include "MessageOrigin.hpp"
#include "entities/PhotoSize.hpp"


namespace Pars
{
    namespace TG
    {
        struct message : TelegramEntities<message>
        {
            using TelegramEntities::operator =; 

            using optuser = std::optional<User>;
            using optchat = std::optional<Chat>;
            using optphoto = std::optional<std::pmr::vector<PhotoSize>>;

            static inline const json::string entity_name{"message"};
            static constexpr  size_t req_fields = 3;
            static constexpr  size_t opt_fields = 17;

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            uint64_t message_id;
            uint64_t date;
            Chat chat;

            optint  message_thread_id;
            optuser from;
            optchat sender_chat;
            optint  sender_boost_count;
            optuser sender_business_bot;
            optstr  business_connection_id;
            std::optional<std::shared_ptr<MessageOrigin>> forward_origin;
            optbool is_topic_message;
            optbool is_automatic_forward;
            std::optional<std::shared_ptr<message>> reply_to_message;
            ///fields
            ///
            ///....
            std::optional<Document> document;
            optphoto photo;
            optuser via_bot;
            optuint edit_date;
            optbool has_protected_content;
            optbool is_from_offline;
            optstr media_group_id;
            optstr author_signature;
            optstr text;
            optstr caption;
             

            public:

            message() noexcept {}


            template<is_all_json_entities T>
            message(T&& val)
            {
                create(std::forward<T>(val));
            }


            message
            (
                uint64_t message_id,
                uint64_t date,
                Chat chat,
                optint  message_thread_id = {},
                optuser from = {},
                optchat sender_chat = {},
                optint  sender_boost_count = {},
                optuser sender_business_bot = {},
                optstr  business_connection_id = {},
                std::optional<std::shared_ptr<MessageOrigin>> forward_origin = {},
                optbool is_topic_message = {},
                optbool is_automatic_forward = {},
                std::optional<std::shared_ptr<message>> reply_to_message = {},
                optuser via_bot = {},
                optuint edit_date = {},
                optbool has_protected_content = {},
                optbool is_from_offline = {},
                optstr media_group_id = {},
                optstr author_signature = {},
                optstr text = {},
                std::optional<Document> document = {},
                optphoto photo = {},
                optstr caption = {}
            )
            noexcept: 
                message_id(message_id),
                date(date),
                chat(std::move(chat)),
                message_thread_id(message_thread_id),
                from(std::move(from)),
                sender_chat(std::move(sender_chat)),
                sender_boost_count(sender_boost_count),
                sender_business_bot(std::move(sender_business_bot)),
                business_connection_id(std::move(business_connection_id)),
                forward_origin(std::move(forward_origin)),
                is_topic_message(is_topic_message),
                is_automatic_forward(is_automatic_forward),
                reply_to_message(std::move(reply_to_message)),
                document(std::move(document)),
                photo(std::move(photo)),
                via_bot(std::move(via_bot)),
                edit_date(edit_date),
                has_protected_content(has_protected_content),
                is_from_offline(is_from_offline),
                media_group_id(std::move(media_group_id)),
                author_signature(std::move(author_signature)),
                text(std::move(text)),
                caption(std::move(caption))
                {

                }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                auto message_map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(message, message_id), json::kind::int64),
                    std::make_pair(JS_POINTER(message, date), json::kind::int64),
                    std::make_pair(JS_POINTER(message, chat), json::kind::object)
                );

                if (message_map.size() != req_fields)
                {
                    return std::nullopt;
                }

                return message_map; 
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            fields_map
            optional_fields(T&& val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(message, message_thread_id), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, from), json::kind::object),
                    std::make_pair(JS_POINTER(message, sender_chat), json::kind::object),
                    std::make_pair(JS_POINTER(message, sender_boost_count), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, sender_business_bot), json::kind::object),
                    std::make_pair(JS_POINTER(message, forward_origin), json::kind::object),
                    std::make_pair(JS_POINTER(message, is_topic_message), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, is_automatic_forward), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, reply_to_message), json::kind::object),
                    std::make_pair(JS_POINTER(message, via_bot), json::kind::object),
                    std::make_pair(JS_POINTER(message, edit_date), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, has_protected_content), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, is_from_offline), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, media_group_id), json::kind::string),
                    std::make_pair(JS_POINTER(message, author_signature), json::kind::string),
                    std::make_pair(JS_POINTER(message, text), json::kind::string),
                    std::make_pair(JS_POINTER(message, document), json::kind::object),
                    std::make_pair(JS_POINTER(message, photo), json::kind::array),
                    std::make_pair(JS_POINTER(message, caption), json::kind::string)
                );
            };



            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {

               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(message_id, message_id));

               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(date, date));
     
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(chat, chat));
                   
               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(message_thread_id, message_thread_id));
                            
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(from, from));
                           
               MainParser::field_from_map
               <json::kind::object>(map, MAKE_PAIR(sender_chat, sender_chat));
                              
               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(sender_boost_count, sender_boost_count));
                                          
               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(sender_boost_count, sender_boost_count));
                                          
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(sender_business_bot, sender_business_bot));


                forward_origin = Pars::TG::find_MessageOriginHeirs(map);
                if (forward_origin)
                {
                    auto & ptr = forward_origin.value();
                    *ptr = map;
                }
                                                                            
               MainParser::field_from_map
               <json::kind::bool_>(map, MAKE_PAIR(is_topic_message, is_topic_message));
                                                                          
               MainParser::field_from_map
               <json::kind::bool_>(map, MAKE_PAIR(is_automatic_forward, is_automatic_forward));
                                                                                         
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(reply_to_message, reply_to_message));

               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(via_bot, via_bot));

               MainParser::field_from_map
               <json::kind::int64>(map, MAKE_PAIR(edit_date, edit_date));

               MainParser::field_from_map
               <json::kind::bool_>(map, MAKE_PAIR(is_from_offline, is_from_offline));

               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(media_group_id, media_group_id));
               
               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(author_signature, author_signature));

               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(text, text));

               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(document, document));


               json::array photo_sizes;
               MainParser::field_from_map
               <json::kind::array>(std::forward<T>(map), MAKE_PAIR(photo, photo_sizes));

               print("\n\nPhoto Sizes array:", photo_sizes.size(),"\n\n");
               if( ! photo_sizes.empty())
               {
                  photo = std::pmr::vector<PhotoSize>{};
               }
               else
               {
                 photo = std::nullopt;
               }

               for(auto&& ph : photo_sizes)
               {
                 PhotoSize temp{};
                 auto map = PhotoSize::verify_fields(std::move(ph));
                 if(map)
                 {
                   temp.fields_from_map(std::move(map).value());
                   auto& vec = photo.value();
                   vec.push_back(std::move(temp));
                 }
               }

               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(caption, caption));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return message::fields_to_value
                (
                    self.message_id,
                    self.date,
                    Utils::forward_like<Self>(self.chat),
                    self.message_thread_id,
                    Utils::forward_like<Self>(self.from),
                    Utils::forward_like<Self>(self.sender_chat),
                    self.sender_boost_count,
                    Utils::forward_like<Self>(self.sender_business_bot),
                    Utils::forward_like<Self>(self.business_connection_id),
                    Utils::forward_like<Self>(self.forward_origin),
                    self.is_topic_message,
                    self.is_automatic_forward,
                    Utils::forward_like<Self>(self.reply_to_message),
                    Utils::forward_like<Self>(self.via_bot),
                    self.edit_date,
                    self.has_protected_content,
                    self.is_from_offline,
                    Utils::forward_like<Self>(self.media_group_id),
                    Utils::forward_like<Self>(self.author_signature),
                    Utils::forward_like<Self>(self.text),
                    Utils::forward_like<Self>(self.document),
                    Utils::forward_like<Self>(self.photo),
                    Utils::forward_like<Self>(self.caption)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                uint64_t message_id,
                uint64_t date,
                TG::Chat chat, 
                optint  message_thread_id = {},
                std::optional<User> from = {},
                std::optional<Chat> sender_chat = {},
                optint  sender_boost_count = {},
                std::optional<User> sender_business_bot = {},
                optstr  business_connection_id = {},
                std::optional<std::shared_ptr<TG::MessageOrigin>> forward_origin = {},
                optbool is_topic_message = {},
                optbool is_automatic_forward = {},
                std::optional<std::shared_ptr<message>> reply_to_message = {},
                std::optional<TG::User> via_bot = {},
                optuint edit_date = {},
                optbool has_protected_content = {},
                optbool is_from_offline = {},
                optstr media_group_id = {},
                optstr author_signature = {},
                optstr text = {},
                std::optional<Document> document = {},
                optphoto photo = {},
                optstr caption = {}
            )
            {

                json::object req_ob(MainParser::get_storage_ptr());
                req_ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(message_id, message_id),
                        PAIR(date, date)
                    );
                

                std::optional<std::reference_wrapper<MessageOrigin>> origin_ = {};
                std::optional<message> mes_ = {};

                bool is_exist = false;
                bool is_move  = false;

                if (forward_origin.has_value())
                {
                  if (forward_origin.value() != nullptr)
                  {
                    is_exist = true;
                    origin_ = *forward_origin.value();
                  }

                  if (forward_origin.value().use_count() == 1)
                    is_move = true;
                }

                if (reply_to_message.has_value())
                {
                    if (reply_to_message.value().use_count() == 1)
                    {
                        mes_ = std::move(*reply_to_message.value());
                    }
                    else
                    {
                        mes_ = *reply_to_message.value();
                    }
                }

                json::object objects = parse_tg_entenies_to_obj
                (
                    std::move(chat),
                    std::move(from),
                    std::move(sender_chat),
                    std::move(sender_business_bot),
                    (is_exist) ? (is_move ? std::move(origin_) : origin_) 
                    : std::optional<std::reference_wrapper<MessageOrigin>>{},
                    std::move(mes_),
                    std::move(via_bot),
                    std::move(document),
                    std::move(photo)
                );

                MainParser::container_move(std::move(req_ob), objects);

                json::object opt_ob{MainParser::get_storage_ptr()};
                opt_ob = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(message_thread_id, message_thread_id),
                        MAKE_OP(sender_boost_count, sender_boost_count),
                        MAKE_OP(business_connection_id, std::move(business_connection_id)),
                        MAKE_OP(is_topic_message, is_topic_message),
                        MAKE_OP(is_automatic_forward, is_automatic_forward),
                        MAKE_OP(edit_date, edit_date),
                        MAKE_OP(has_protected_content, has_protected_content),
                        MAKE_OP(is_from_offline, is_from_offline),
                        MAKE_OP(media_group_id, std::move(media_group_id)),
                        MAKE_OP(author_signature, std::move(author_signature)),
                        MAKE_OP(text, std::move(text)),
                        MAKE_OP(caption, std::move(caption))
                    );  

                MainParser::container_move(std::move(opt_ob), objects);

                json::object res(MainParser::get_storage_ptr());
                res [FIELD_NAME(message)] = std::move(objects);
                return res;
            }
        };

    }//namespace TG   

}//namespace Pars
