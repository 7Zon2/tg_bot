#pragma once
#include "PhotoSize.hpp"


namespace Pars
{
    namespace TG
    {
        struct Animation : protected PhotoSize
        {

            double duration;
            std::optional<PhotoSize>thumbnail;
            optstr file_name;
            optstr mime_type;

            public:

            static const inline json::string entity_name = "animation";
            static const constexpr size_t req_fields = 5;
            static const constexpr size_t opt_fields = 4;

            [[nodiscard]]
            json::string
            get_entity_name() override
            {
                return entity_name;
            }

            public:

            Animation(){}

            Animation
            (
                json::string file_id,
                json::string file_unique_id,
                double width,
                double height,
                double duration,
                std::optional<PhotoSize> thumbnail = {},
                optstr file_name =  {},
                optstr mime_type =  {},
                optuint file_size = {}
            )
            :
                PhotoSize
                (
                    std::move(file_id),
                    std::move(file_unique_id),
                    width,
                    height,
                    file_size
                ),
                duration(duration),
                thumbnail(thumbnail),
                file_name(std::move(file_name)),
                mime_type(std::move(mime_type))
            {

            }

            template<is_all_json_entities T>
            Animation(T&& obj)
            {
                create(std::forward<T>(obj));
            }


            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                auto photo_map =  PhotoSize::requested_fields(val, "animation");
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
                MainParser::container_move(std::move(animation_map), photo_map_value);
                return photo_map;
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
                    std::make_pair(JS_POINTER(animation, duration),  json::kind::double_),
                    std::make_pair(JS_POINTER(animation, thumbnail), json::kind::object),
                    std::make_pair(JS_POINTER(animation, file_name), json::kind::string),
                    std::make_pair(JS_POINTER(animation, mime_type), json::kind::string)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map(T && map)
            {
                PhotoSize::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(duration ,duration));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(thumbnail, thumbnail));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_name, file_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(mime_type, mime_type));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return Animation::fields_to_value
                (
                    Utils::forward_like<Self>(self.file_id),
                    Utils::forward_like<Self>(self.file_unique_id),
                    self.width,
                    self.height,
                    self.duration,
                    Utils::forward_like<Self>(self.thumbnail),
                    Utils::forward_like<Self>(self.file_name),
                    Utils::forward_like<Self>(self.mime_type),
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
                optuint file_size = {}
            )
            {
                json::object ob{MainParser::get_storage_ptr()};
                ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(file_id, std::move(file_id)),
                        PAIR(file_unique_id, std::move(file_unique_id)),
                        PAIR(width, width),
                        PAIR(height, height),
                        PAIR(duration, duration)
                    );

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(file_name, std::move(file_name)),
                        MAKE_OP(mime_type, std::move(mime_type)),
                        MAKE_OP(file_size, file_size)
                    );

                if (thumbnail.has_value())
                {
                    json::object ob3(MainParser::get_storage_ptr());
                    auto && ref_thumbnail = std::move(thumbnail).value();
                    ob3 = ref_thumbnail.fields_to_value().as_object();

                    MainParser::container_move(ob3, ob);
                }

                Pars::MainParser::container_move(std::move(ob2), ob);

                json::object res(MainParser::get_storage_ptr());
                res["animation"] = std::move(ob);
                return res;
            }
        };
    }
}
