#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct MessageOrigin : TelegramEntities<MessageOrigin>
        {

            using TelegramEntities::operator=;

            json::string type;
            size_t date_;

            static inline const json::string entity_name{"messageorigin"};
            static constexpr size_t req_fields = 2;
            static constexpr size_t opt_fields = 0;

            public:

            [[nodiscard]]
            json::string 
            get_entity_name() override
            {
                return entity_name;
            }


            MessageOrigin(){}

            MessageOrigin(json::string type, const size_t date)
                : type(std::move(type)), date_(date){}

            virtual ~MessageOrigin() = 0;

            public:

            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value & val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }           
                return TelegramEntities<MessageOrigin>::verify_fields(val);
            }


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                return  TelegramEntities<MessageOrigin>::verify_fields(std::move(val));
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val, json::string_view method = FIELD_NAME(messageorigin))
            {
                json::string meth{"/"};
                meth+=method;
                meth+="/";

                json::string type_{method};
                type_+="type";

                json::string date_{method};
                date_+="date";

                auto map =  MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(type_, json::kind::string),
                    std::make_pair(date_, json::kind::uint64)
                );

                if (req_fields != map.size())
                {
                    return std::nullopt;
                }

                return map;
            }


            template<as_json_value T>
            static
            fields_map
            optional_fields(T&& val)
            {
                return {};
            }


            virtual void 
            fields_from_map
            (fields_map && map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::move(map), MAKE_PAIR(type, type));

                MainParser::field_from_map
                <json::kind::uint64>(std::move(map), std::make_pair("date",std::ref(date_)));
            }


            virtual void 
            fields_from_map
            (fields_map & map)
            {
                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(type, type));

                MainParser::field_from_map
                <json::kind::uint64>(map, MAKE_PAIR(date, date_));
            }


            [[nodiscard]]
            virtual json::value 
            fields_to_value() = 0;


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string type,
                size_t date
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob["messageorigin"] = MainParser::parse_ObjPairs_as_obj
                (
                    PAIR(type, std::move(type)), 
                    PAIR(date, date)
                ); 
                return ob;
            }
        };

        MessageOrigin::~MessageOrigin(){}



        struct MessageOriginUser : MessageOrigin
        {

            using MessageOrigin::operator=;

            User sender_user;

            static constexpr size_t req_fields = 3;
            static constexpr size_t opt_fields = 0;
            static inline const json::string entity_name{"messageoriginuser"}; 

            public:

            [[nodiscard]]
            json::string 
            get_entity_name() override
            {
                return entity_name;
            }


            MessageOriginUser(){}

            MessageOriginUser
            (
                json::string type,
                const size_t date,
                TG::User user
            )
            :
             MessageOrigin(std::move(type), date),
             sender_user(std::move(user))
             {

             }

            public:

            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value & val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(val);
                if (! map.has_value())
                {
                    return {};
                }

                auto map_2 = TelegramEntities<MessageOriginUser>::verify_fields(val);
                if (! map_2.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), map_2.value());
                return map_2;
            }


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(std::move(val));
                if (! map.has_value())
                {
                    return {};
                }

                auto map_2 = TelegramEntities<MessageOriginUser>::verify_fields(std::move(val));
                if (! map_2.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), map_2.value());
                return map_2;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(messageoriginuser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginuser, sender_user), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                MainParser::container_move(std::move(map2), map.value());
                return std::move(map.value());
            }


            template<as_json_value T>
            [[nodiscard]]
            static fields_map
            optional_fields(T&& val)
            {
                return MessageOrigin::optional_fields(std::forward<T>(val));
            }


            void
            fields_from_map
            (fields_map && map) override
            {
                MessageOrigin::fields_from_map(std::move(map));

                MainParser::field_from_map
                <json::kind::object>(std::move(map), MAKE_PAIR(semder_user, sender_user));
            }


            void
            fields_from_map
            (fields_map & map) override
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::object>(map, MAKE_PAIR(sender_user, sender_user));
            }


            [[nodiscard]]
            json::value
            fields_to_value() & override
            {
                return MessageOriginUser::fields_to_value
                (
                    type,
                    date_,
                    sender_user
                );
            }


            [[nodiscard]]
            json::value
            fields_to_value() && override
            {
                return MessageOriginUser::fields_to_value
                (
                    std::move(type),
                    date_,
                    std::move(sender_user)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string type,
                size_t date,
                TG::User user
            )
            {
                auto ob  = MessageOrigin::fields_to_value(std::move(type), date).as_object();
                auto ob2 = std::move(user).fields_to_value().as_object();

                Pars::MainParser::container_move(std::move(ob2), ob);                                                    

                json::object res(MainParser::get_storage_ptr());
                res["messageoriginuser"] = std::move(ob);

                return res;
            }
        };


        struct MessageOriginHiddenUser : MessageOrigin
        {
            using MessageOrigin::operator=;

            json::string sender_user_name;

            static constexpr size_t req_fields = 3;
            static constexpr size_t opt_fields = 0;
            static const inline json::string enity_name{"messageoriginhiddenuser"};

            public:

            [[nodiscard]]
            json::string 
            get_entity_name() override
            {
                return entity_name;
            }


            MessageOriginHiddenUser(){}

            MessageOriginHiddenUser
            (
                json::string type,
                const size_t date,
                json::string sender_user_name
            )
            :
            MessageOrigin(std::move(type), date),
            sender_user_name(std::move(sender_user_name))
            {

            }

            public:


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value & val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(val);
                if (! map.has_value())
                {
                    return {};
                }

                auto map_2 = TelegramEntities<MessageOriginHiddenUser>::verify_fields(val);
                if (! map_2.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), map_2.value());
                return map_2;
            }


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(std::move(val));
                if (! map.has_value())
                {
                    return {};
                }

                auto map_2 = TelegramEntities<MessageOriginHiddenUser>::verify_fields(std::move(val));
                if (! map_2.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), map_2.value());
                return map_2;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginhiddenuser, sender_user_name), json::kind::string)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                MainParser::container_move(std::move(map2), map_);

                return std::move(map_);
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            fields_map
            optional_fields(T&& val)
            {
                return MessageOrigin::optional_fields(std::forward<T>(val));
            }


            void
            fields_from_map
            (fields_map && map) override
            {
                MessageOrigin::fields_from_map(std::move(map));

                MainParser::field_from_map
                <json::kind::string>(std::move(map), MAKE_PAIR(sender_user_name, sender_user_name));
            }


            void
            fields_from_map
            (fields_map & map) override
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(sender_user_name, sender_user_name));
            }


            [[nodiscard]]
            json::value
            fields_to_value() & override
            {
                return MessageOriginHiddenUser::fields_to_value
                (
                    type,
                    date_,
                    sender_user_name
                );
            }


            [[nodiscard]]
            json::value
            fields_to_value() && override
            {
                return MessageOriginHiddenUser::fields_to_value
                (
                    std::move(type),
                    date_,
                    std::move(sender_user_name)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string type,
                size_t date,
                json::string sender_user_name
            )
            {
                auto ob = MessageOrigin::fields_to_value(std::move(type), date).as_object();
                ob.emplace(FIELD_NAME(sender_user_name), std::move(sender_user_name));

                json::object res(MainParser::get_storage_ptr());
                res["messageoriginhiddenuser"] = std::move(ob);
                return res;
            }
        };


        struct MessageOriginChat : MessageOrigin
        {
            using MessageOrigin::operator=;

            TG::Chat sender_chat;
            optstr  author_signature = {};

            static constexpr    size_t req_fields = 3;
            static constexpr    size_t opt_fields = 1;
            static const inline json::string entity_name{"messageoriginchat"};

            public:

            [[nodiscard]]
            json::string 
            get_entity_name() override
            {
                return entity_name;
            }


            MessageOriginChat(){}

            MessageOriginChat
            (
                json::string type,
                const size_t date,
                TG::Chat sender_chat,
                optstr author_signature = {}
            )
            :
            MessageOrigin(std::move(type), date),
            sender_chat(std::move(sender_chat)),
            author_signature(std::move(author_signature))
            {

            }

            public:


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value & val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(val);
                if (! map.has_value())
                {
                    return {};
                }

                auto chat_map = TelegramEntities<MessageOriginChat>::verify_fields(val);
                if (! chat_map.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), chat_map.value());
                return chat_map;
            }


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(std::move(val));
                if (! map.has_value())
                {
                    return {};
                }

                auto chat_map = TelegramEntities<MessageOriginChat>::verify_fields(std::move(val));
                if (! chat_map.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), chat_map.value());
                return chat_map;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(MessageOriginChat, sender_chat), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                MainParser::container_move(std::move(map2), map.value());
                return std::move(map.value());
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
                    std::make_pair(JS_POINTER(messageoriginchat, author_signature), json::kind::string)
                );
            }


            void
            fields_from_map
            (fields_map && map) override
            {
                MessageOrigin::fields_from_map(std::move(map));

                MainParser::field_from_map
                <json::kind::object>(std::move(map), MAKE_PAIR(sender_chat, sender_chat));

                MainParser::field_from_map
                <json::kind::string>(std::move(map), MAKE_PAIR(author_signature, author_signature));
            }


            void
            fields_from_map
            (fields_map & map) override
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::object>(map, MAKE_PAIR(sender_chat, sender_chat));

                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(author_signature, author_signature));
            }


            [[nodiscard]]
            json::value
            fields_to_value() & override
            {
                return MessageOriginChat::fields_to_value
                (
                    type,
                    date_,
                    sender_chat,
                    author_signature
                );
            }


            [[nodiscard]]
            json::value
            fields_to_value() && override
            {
                return MessageOriginChat::fields_to_value
                (
                    std::move(type),
                    date_,
                    std::move(sender_chat),
                    std::move(author_signature)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string type,
                size_t date,
                TG::Chat sender_chat,
                optstr author_signature = {}
            )
            {
                auto ob = MessageOrigin::fields_to_value(std::move(type), date).as_object();
                auto send_ob = std::move(sender_chat).fields_to_value().as_object(); 

                Pars::MainParser::container_move(std::move(send_ob), ob);

                if (author_signature.has_value())
                    ob.emplace(FIELD_NAME(author_signature), author_signature.value());

                json::object res(MainParser::get_storage_ptr());
                res["messageoriginchat"] = std::move(ob); 
                return res;
            }
        };


        struct MessageOriginChannel : MessageOrigin
        {
            using MessageOrigin::operator=;

            public:

            TG::Chat chat;
            size_t message_id;
            optstr author_signature;

            static constexpr  size_t req_fields = 4;
            static constexpr  size_t opt_fields = 1;
            static const inline json::string enity_name{"messageoriginchannel"};

            public:

            [[nodiscard]]
            json::string 
            get_entity_name() override
            {
                return entity_name;
            }


            MessageOriginChannel(){}

            MessageOriginChannel
            (
                json::string type,
                size_t date,
                TG::Chat chat,
                size_t message_id,
                optstr author_signature = {}
            )
            :
                MessageOrigin(std::move(type), date),
                chat(std::move(chat)),
                message_id(message_id),
                author_signature(std::move(author_signature))
                {

                }

            public:

            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value & val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(val);
                if (! map.has_value())
                {
                    return {};
                }

                auto channel_map = TelegramEntities<MessageOriginChannel>::verify_fields(val);
                if (! channel_map.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), channel_map.value());
                return channel_map;
            }


            [[nodiscard]]
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name) override
            {
                if (name != get_entity_name())
                {
                    return {};
                }
                 
                auto map = TelegramEntities<MessageOrigin>::verify_fields(std::move(val));
                if (! map.has_value())
                {
                    return {};
                }

                auto channel_map = TelegramEntities<MessageOriginChannel>::verify_fields(std::move(val));
                if (! channel_map.has_value())
                {
                    return {};
                }

                MainParser::container_move(std::move(map.value()), channel_map.value());
                return channel_map;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(messageoriginchannel));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginchannel, chat), json::kind::object),
                    std::make_pair(JS_POINTER(messageoriginchannel, message_id), json::kind::uint64)
                );

                if(map2.size() != 2)
                {
                    return std::nullopt;
                }

                MainParser::container_move(std::move(map2), map.value());
                return std::move(map.value());
            }


            template<as_json_value T>
            [[nodiscard]]
            static fields_map
            optional_fields(T&& val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginchannel, author_signature), json::kind::string)
                );
            }


            void
            fields_from_map
            (fields_map && map) override
            {
                MessageOrigin::fields_from_map(std::move(map));

                MainParser::field_from_map
                <json::kind::object>(std::move(map), MAKE_PAIR(chat, chat));

                MainParser::field_from_map
                <json::kind::string>(std::move(map), MAKE_PAIR(author_signature, author_signature));
            }


            void
            fields_from_map
            (fields_map & map) override
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::object>(map, MAKE_PAIR(chat, chat));

                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(author_signature, author_signature));
            }


            [[nodiscard]]
            json::value
            fields_to_value() & override
            {
                return MessageOriginChannel::fields_to_value
                (
                    type,
                    date_,
                    chat,
                    message_id,
                    author_signature
                );
            }


            [[nodiscard]]
            json::value
            fields_to_value() && override
            {
                return MessageOriginChannel::fields_to_value
                (
                    std::move(type),
                    date_,
                    std::move(chat),
                    message_id,
                    std::move(author_signature)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string type,
                size_t date,
                TG::Chat chat,
                size_t message_id,
                optstr author_signature = {}
            )
            {
                auto ob = MessageOriginChat::fields_to_value(std::move(type), date, std::move(chat), std::move(author_signature)).as_object();
                ob.emplace("message_id", message_id);

                json::object res(MainParser::get_storage_ptr());
                res["messageoriginchannel"] = std::move(ob);
                return res;
            }
        };


        template<is_fields_map T>
        [[nodiscard]]
        std::unique_ptr<MessageOrigin>
        find_MessageOriginHeirs
        (T&& map)
        {
            std::unique_ptr<MessageOrigin> ptr;
            auto it = map.find(FIELD_NAME(messageoriginchat));
            if (it != map.end())
            {
                ptr = std::make_unique<MessageOriginChat>();
                ptr->fields_from_map(std::forward<T>(map));
                return ptr;
            }

            it = map.find(FIELD_NAME(messageoriginchannel));
            if (it != map.end())
            {
                ptr = std::make_unique<MessageOriginChannel>();
                ptr->fields_from_map(std::forward<T>(map));
                return ptr;
            }

            it = map.find(FIELD_NAME(messageoriginhiddenuser));
            if (it != map.end())
            {
                ptr = std::make_unique<MessageOriginHiddenUser>();
                ptr->fields_from_map(std::forward<T>(map));
                return ptr;
            }

            return ptr;
        }

    }// namespace TG

}//namespace Pars