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

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                auto photo_map =  PhotoSize::requested_fields(val);
                if (!photo_map.has_value())
                {
                    return std::nullopt;
                }

                auto animation_map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(animation, duration), json::kind::double_)
                );

                if (animation_map.empty())
                {
                    return std::nullopt;
                }

                auto &photo_map_value = photo_map.value();
                auto &animation_map_value = animation_map.value();
                MainParser::container_move(std::move(animation_map_value), photo_map_value);
                return photo_map;
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(animation, duration),  json::kind::double_),
                    std::make_pair(JS_POINTER(animation, thumbnail), json::kind::object),
                    std::make_pair(JS_POINTER(animation, file_name), json::kind::string),
                    std::make_pair(JS_POINTER(animation, mime_type), json::kind::string)
                )
            }


            template<is_fields_map T>
            void 
            fields_from_map(T && map)
            {
                PhotoSize::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(duration));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(thumbnail));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(mime_type));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::Animation
                (
                    self.file_id,
                    self.file_unique_id,
                    self.width,
                    self.height,
                    self.duration,
                    forward_like<Self>(self.thumbnail),
                    forward_like<Self>(self.file_name),
                    forward_like<Self>(self.mime_type),
                    self.file_size
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string file_id,
                json::string file_unique_id,
                double width,
                double height,
                double duration,
                std::optional<TG::PhotoSize> thumbnail = {},
                optstr file_name =  {},
                optstr mime_type =  {},
                optdouble file_size = {}
            )
            {
                json::object ob{MainParser::get_storage_ptr()};
                ob = parse_ObjPairs_as_obj
                    (
                        PAIR(std::move(file_id)),
                        PAIR(std::move(file_unique_id)),
                        PAIR(width),
                        PAIR(height),
                        PAIR(duration)
                    );

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(std::move(file_name)),
                        MAKE_OP(std::move(mime_type)),
                        MAKE_OP(file_size)
                    );

                if (thumbnail.has_value())
                {
                    json::object ob2(MainParser::get_storage_ptr());
                    auto && ref_thumbnail = std::move(thumbnail.value());
                    ob2 = std::move(ref_thumbnail.fields_to_value()).as_object();

                    Pars::obj_move(ob2, ob);
                }

                Pars::MainParser::container_move(std::move(ob), photo_ob);

                json::object res(MainParser::get_storage_ptr());
                res["animation"] = std::move(photo_ob);
                return res;
            }
        };
    }
}