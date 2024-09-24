#pragma once
#include "TelegramEntities.hpp"
#include "Animation.hpp"


namespace Pars
{
    namespace TG
    {
        struct Audio : protected Animation
        {

            optstr performer;
            optstr title;


            static const constexpr size_t req_fields = 3;
            static const constexpr size_t opt_fields = 6;

            public:

            Audio()
            {
                inherited_name = "audio";
            }

            Audio
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double duration,
                optstrw performer = {},
                optstrw title = {},
                std::optional<std::reference_wrapper<PhotoSize>> thumbnail = {},
                optstrw file_name =  {},
                optstrw mime_type =  {},
                optdouble file_size = {} 
            )
            :
                Animation
                (
                    file_id,
                    file_unique_id,
                    0,
                    0,
                    duration,
                    std::move(thumbnail),
                    file_name,
                    mime_type,
                    file_size
                ),
                performer(performer),
                title(title)
            {
                inherited_name = "audio";
            }

            public:

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                using mp = std::make_pair;

                auto audio_map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    mp(JS_POINTER(audio, file_id), json::kind::string),
                    mp(JS_POINTER(audio, file_unique_id), json::kind::string),
                    mp(JS_POINTER(audio, duration), json::kind::double_),
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
                return MainParser::mappe_pointers_validation
                (
                    std::move(val),
                    mp(JS_POINTER(audio, title),     json::kind::string),
                    mp(JS_POINTER(audio, performer), json::kind::string),
                    mp(JS_POINTER(audio, thumbnail), json::kind::object),
                    mp(JS_POINTER(audio, file_name), json::kind::string),
                    mp(JS_POINTER(audio, mime_type), json::kind::string),
                    mp(JS_POINTER(audio, file_size), json::kind::double_)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map(T && map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_id));

                MainParser::fields_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_unique_id));

                MainParser:;fields_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(file_size));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(duration));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(thumbnail));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(mime_type));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(performer));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(title));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::Audio
                (
                    file_id,
                    file_unique_id,
                    duration,
                    forward_like<Self>(performer),
                    forward_like<Self>(title),
                    forward_like<Self>(file_name),
                    forward_like<Self>(mime_type),
                    forward_like<Self>(thumbnail),
                    file_size
                );
            }

        }
    }
}