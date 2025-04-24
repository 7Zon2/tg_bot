#pragma once
#include "TelegramEntities.hpp"
#include "json_head.hpp"

namespace Pars
{
    namespace TG
    {
        struct TelegramResponse : TelegramEntities<TelegramResponse>
        {
            public:

            using TelegramEntities::operator =;

            bool ok;
            optint error_code;
            optstr description;
            optvalue result; 
            optuint update_id;

            static const inline json::string entity_name{"telegramresponse"};
            static constexpr size_t req_fields = 1;
            static constexpr size_t opt_fields = 3;

            public:

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            TelegramResponse(){}

            TelegramResponse
            (
                bool ok,
                optint error_code,
                optstr description,
                optvalue result
            )
            :
                ok(ok),
                error_code(error_code),
                description(std::move(description)),
                result(std::move(result))
                {

                }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map 
            requested_fields(T&& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JSP(ok), json::kind::bool_)
                );

                if(map.size() != req_fields)
                    return std::nullopt;
                else
                    return map;                
            } 

            
            template<as_json_value T>
            [[nodiscard]]
            static 
            fields_map
            optional_fields(T&& val)
            {
                const static json::string resP{JSP(result)};
                auto pair = std::make_pair(resP, json::kind::array);
                std::error_code er;
                auto* pv = val.find_pointer(resP, er);
                if (pv)
                {
                    if ( pv->is_array())
                    {
                        pair = std::make_pair(resP, json::kind::array);
                    }
                    else if (pv->is_object())
                    {
                        pair = std::make_pair(resP, json::kind::object);
                    }
                    else
                    {
                        throw std::runtime_error{"\nresult pointer has unknown type\n"};
                    }
                }


                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val), 
                    std::make_pair(JSP(error_code), json::kind::int64),
                    std::make_pair(JSP(description), json::kind::string),
                    pair
                );
                return map;
            }


            template<is_fields_map T>
            void fields_from_map
            (T&& map)
            {
                auto findUpdate = [this]()
                {
                    boost::system::error_code er;
                    json::value* v = result.value().find_pointer(JSP(update_id), er);
                    if (!er)
                        update_id = v->as_int64();
                    else
                        throw std::runtime_error{"failed to find update id"};
                };

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), std::make_pair("ok", std::ref(ok)));

                MainParser::field_from_map
                <json::kind::uint64>(std::forward<T>(map), std::make_pair("error_code", std::ref(error_code)));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), std::make_pair("description", std::ref(description)));


                optarray arr{MainParser::get_storage_ptr()};
                MainParser::field_from_map
                <json::kind::array>(std::forward<T>(map), std::make_pair("result", std::ref(arr)));

                if (arr.has_value() && !arr.value().empty())
                {
                    result = MainParser::parse_jsonArray_as_value(std::move(arr.value()));
                    findUpdate();
                }
                else
                {
                    optobj obj{MainParser::get_storage_ptr()};
                    MainParser::field_from_map
                    <json::kind::object>(std::forward<T>(map), std::make_pair("result", std::ref(obj)));
                    result = std::move(obj);
                }
            }

            
            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramResponse::fields_to_value
                (
                    self.ok,
                    self.error_code,
                    forward_like<Self>(self.description),
                    forward_like<Self>(self.result),
                    self.update_id
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                bool ok,
                optint error_code,
                optstr description,
                optvalue result,
                optuint  update_id
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob = MainParser::parse_ObjPairs_as_obj(PAIR(ok, ok));

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = MainParser::parse_OptPairs_as_obj
                (
                    MAKE_OP(error_code, error_code),
                    MAKE_OP(descrtiption, std::move(description)),
                    MAKE_OP(update_id, update_id)
                );

                Pars::MainParser::container_move(std::move(ob2), ob);

                if (result.has_value())
                {
                    json::value val_arr{std::move(result.value())};
                    ob.emplace(FIELD_NAME(result), std::move(val_arr));
                }
                return ob;
            }
            
        };

    }//namespace TG

}//namespace Pars
