#pragma once
#include "TelegramEntities.hpp"


namespace Pars
{
    namespace TG
    {
        struct PhotoSize : TelegramEntities<PhotoSize>
        {
            
            protected:

            json::string inherited_name{FIELD_TO_LOWER(PhotoSize)};

            PhotoSize(json::string inherited_name):
            inherited_name(std::move(inherited_name)){}

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
                json::string file_id,
                json::string file_unique_id,
                double width,
                double height,
                optdouble file_size = {}
            )
            :
                file_id(std::move(file_id)),
                file_unique_id(std::move(file_unique_id)),
                width(width),
                height(height),
                file_size(file_size)
            {

            }

            virtual ~PhotoSize(){}

            public:

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                using MainParser::make_json_pointer;

                auto map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
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


            [[nodiscard]]
            static 
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::PhotoSize
                (
                    forward_like<Self>(self.file_id),
                    forward_like<Self>(self.file_unique_id),
                    self.width,
                    self.height,
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
                optdouble file_size = {}
            )
            {
                json::object ob{MainParser::get_storage_ptr()};
                ob = parse_ObjPairs_as_obj
                    (
                        PAIR(std::move(file_id)),
                        PAIR(std::move(file_unique_id)),
                        PAIR(width),
                        PAIR(height)
                    );

                json::object ob2{MainParser::get_storage_ptr()};
                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(file_size)
                    );

                Pars::MainParser::container_move(std::move(ob2), ob);

                json::object res_(MainParser::get_storage_ptr());
                res["photosize"] = std::move(ob);
                return res;
            }


        };
    };
};