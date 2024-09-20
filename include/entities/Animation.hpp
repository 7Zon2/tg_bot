#pragma once
#include "TelegramEntities.hpp"
#include "PhotoSize.hpp"


namespace Pars
{
    namespace TG
    {
        struct Animation : protected PhotoSize
        {

            double duration;
            std::optional<std::reference_wrapper<PhotoSize>> thumbnail;
            optstr file_name;
            optstr mime_type;

            static const constexpr size_t req_fields = PhotoSize::req_fields + 1;
            static const constexpr size_t opt_fields = PhotoSize::opt_fields + 3;

            public:

            Animation():
            PhotoSize(FIELD_TO_LOWER(Animation)){}

            Animation
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double width,
                double height,
                double duration,
                std::optional<std::reference_wrapper<PhotoSize>> thumbnail = {},
                optstrw file_name =  {},
                optstrw mime_type =  {},
                optdouble file_size = {}
            )
            :
                PhotoSize
                (
                    file_id,
                    file_unique_id,
                    width,
                    height,
                    file_size
                ),
                duration(duration),
                thumbnail(thumbnail),
                file_name(file_name),
                mime_type(mime_type)
            {

            }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                auto photo_map = PhotoSize::requested_fields(std::forward<T>(val));
                if( ! photo_map.has_value())
                {
                    return photo_map;
                }

                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(animation, duration),  json::kind::double_),
                    std::make_pair(JS_POINTER(animation, thumbnail), json::kind::object),
                    std::make_pair(JS_POINTER(animation, file_name), json::kind::string),
                    std::make_pair(JS_POINTER(animation, mime_type), json::kind::string)
                )

                if(map.size() != req_fields - PhotoSize::req_fields)
                {
                    return std::nullopt;
                }

                auto& photo_map_value = photo_map.value();
                auto beg = std::make_move_iterator(photo_map_value.begin());
                auto end = std::make_move_iterator(photo_map_value.end());
                return map;
            }

        };
    }
}