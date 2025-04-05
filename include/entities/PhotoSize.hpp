#pragma once
#include "File.hpp"


namespace Pars
{
    namespace TG
    {
        struct PhotoSize : public File
        {
            public:

            using File::operator=;

            double width;
            double height;

            static const constexpr size_t req_fields = 4;
            static const constexpr size_t opt_fields = 1;

            static const inline json::string entity_name{"photosize"};

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            PhotoSize() noexcept {}

            PhotoSize(File file) noexcept:
                File(std::move(file)){}

            template<is_all_json_entities T>
            PhotoSize(T&& obj)
            {
                create(std::forward<T>(obj));
            }

            PhotoSize
            (
                json::string file_id,
                json::string file_unique_id,
                double width,
                double height,
                optdouble file_size = {}
            ) 
            noexcept 
            :
                File
                (
                    std::move(file_id),
                    std::move(file_unique_id),
                    file_size
                ),
                width(width),
                height(height)
            {

            }

            ~PhotoSize(){}

            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val, json::string inherited_name = "photosize")
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_id)), json::kind::string),
                    std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_unique_id)), json::kind::string),
                    std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(width)),  json::kind::double_),
                    std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(height)), json::kind::double_)
                );

                if (req_fields != map.size())
                {
                    return std::nullopt;
                }

                return map;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            fields_map
            optional_fields(T&& val)
            {
                return File::optional_fields(std::forward<T>(val), FIELD_NAME(PhotoSize));
            }


            template<is_fields_map T>
            void 
            fields_from_map(T && map)
            {
                File::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(width, width));

                MainParser::field_from_map
                <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(height, height));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return PhotoSize::fields_to_value
                (
                    Utils::forward_like<Self>(self.file_id),
                    Utils::forward_like<Self>(self.file_unique_id),
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
                ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(file_id, std::move(file_id)),
                        PAIR(file_unique_id, std::move(file_unique_id)),
                        PAIR(width, width),
                        PAIR(height, height)
                    );

                json::object ob2{MainParser::get_storage_ptr()};
                ob2 = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(file_size, file_size)
                    );

                Pars::MainParser::container_move(std::move(ob2), ob);

                json::object res(MainParser::get_storage_ptr());
                res["photosize"] = std::move(ob);
                return res;
            }
        };

    }//namespace TG
}//namespace Pars
