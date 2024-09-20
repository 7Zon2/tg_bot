#pragma once
#include "TelegramEntities.hpp"
#include "concept_entities.hpp"


namespace Pars
{
    namespace TG
    {
        struct PhotoSize : TelegramEntities<PhotoSize>
        {
            
            protected:

            json::string inherited_name{FIELD_TO_LOWER(PhotoSize)};

            PhotoSize(json::string_view inherited_name):
            inherited_name(inherited_name){}

            public:

            using TelegramEntities::operator=;

            json::string file_id;
            json::string file_unique_id;
            double width;
            double height;
            optdouble file_size;


            static const constexpr size_t req_fields = 4;
            static const constexpr size_t opt_fields = 1;

            public:

            PhotoSize(){}

            PhotoSize
            (
                json::string_view file_id,
                json::string_view file_unique_id,
                double width,
                double height,
                optdouble file_size = {}
            )
            :
                file_id(file_id),
                file_unique_id(file_unique_id),
                width(width),
                height(height),
                file_size(file_size)
            {

            }

            virtual ~PhotoSize(){}

            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                using MainParser::make_json_pointer;

                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(make_json_pointer(inherited_name, FIELD_NAME(file_id)), json::kind::string),
                    std::make_pair(make_json_pointer(inherited_name, FIELD_NAME(file_unique_id)), json::kind::string),
                    std::make_pair(make_json_pointer(inherited_name, FIELD_NAME(width)),  json::kind::double_),
                    std::make_pair(make_json_pointer(inherited_name, FIELD_NAME(height)), json::kind::double_)
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
            optional_fields(T && val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(PhotoSize, file_size), json::kind:double_)
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

                MainParser::fields_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(width));

                MainParser::fields_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(height));

                MainParser:;fields_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(file_size));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramEntities::PhotoSize
                (
                    file_id,
                    file_unique_id,
                    width,
                    height,
                    file_size
                );
            }

        };
    };
};