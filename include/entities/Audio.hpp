#pragma once
#include "TelegramEntities.hpp"
#include "Animation.hpp"


namespace Pars
{
    namespace TG
    {
        struct Audio : protected Animation
        {
            using Animation::Animation;
            using Animation::operator = ;

            optstr performer;
            optstr title;

            public:

            static const inline json::string entity_name{"audio"};
            static const constexpr size_t req_fields = 3;
            static const constexpr size_t opt_fields = 6;

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            Audio(){}

            Audio(Animation anim)
                :Animation(std::move(anim)){}

            Audio
            (
                json::string file_id,
                json::string file_unique_id,
                double duration,
                optstr performer = {},
                optstr title = {},
                std::optional<PhotoSize> thumbnail = {},
                optstr file_name =  {},
                optstr mime_type =  {},
                optuint file_size = {}
            )
            :
                Animation
                (
                    std::move(file_id),
                    std::move(file_unique_id),
                    0,
                    0,
                    duration,
                    std::move(thumbnail),
                    std::move(file_name),
                    std::move(mime_type),
                    file_size
                ),
                performer(std::move(performer)),
                title(std::move(title))
            {

            }

            public:

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                auto audio_map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(audio, file_id), json::kind::string),
                    std::make_pair(JS_POINTER(audio, file_unique_id), json::kind::string),
                    std::make_pair(JS_POINTER(audio, duration), json::kind::double_)
                );

                if (audio_map.size() != req_fields)
                {
                    return std::nullopt;
                }

                return audio_map;
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(audio, title),     json::kind::string),
                    std::make_pair(JS_POINTER(audio, performer), json::kind::string),
                    std::make_pair(JS_POINTER(audio, thumbnail), json::kind::object),
                    std::make_pair(JS_POINTER(audio, file_name), json::kind::string),
                    std::make_pair(JS_POINTER(audio, mime_type), json::kind::string),
                    std::make_pair(JS_POINTER(audio, file_size), json::kind::double_)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map(T && map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_id, file_id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_unique_id, file_unique_id));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(file_size, file_size));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(duration, duration));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(thumbnail, thumbnail));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_name, file_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(mime_type, mime_type));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(performer, performer));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(title, title));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return Audio::fields_to_value
                (
                    forward_like<Self>(self.file_id),
                    forward_like<Self>(self.file_unique_id),
                    self.duration,
                    forward_like<Self>(self.performer),
                    forward_like<Self>(self.title),
                    forward_like<Self>(self.file_name),
                    forward_like<Self>(self.mime_type),
                    forward_like<Self>(self.thumbnail),
                    self.file_size
                );
            }


            [[nodiscard]]
            static json::value 
            fields_to_value
            (
                json::string file_id,
                json::string file_unique_id,
                double duration,
                optstr performer = {},
                optstr title = {},
                optstr file_name = {},
                optstr mime_type = {},
                std::optional<TG::PhotoSize> thumbnail = {},
                optuint file_size = {}
            )
            {
                  
                json::object ob = Animation::fields_to_value
                (
                    std::move(file_id),
                    std::move(file_unique_id),
                    0,
                    0,
                    duration,
                    std::move(thumbnail),
                    std::move(file_name),
                    std::move(mime_type),
                    file_size
                ).as_object();

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(title, std::move(title)),
                        MAKE_OP(performer, std::move(performer))
                    );

                MainParser::container_move(std::move(ob2), ob);
                json::object res(MainParser::get_storage_ptr());
                res["audio"] = std::move(ob);
                return res;
            }

        };
    }
}
