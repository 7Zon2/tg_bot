#pragma once
#include "concept_entities.hpp"
#include "tg_requestes.hpp"

namespace Pars
{
    namespace TG
    {
        template<typename Derived>
        struct TelegramEntities
        {
            protected:

            template<is_all_json_entities T>
            void create(T&& val)
            {
                json::value val_ = std::forward<T>(val);
                auto map = verify_fields(std::move(val_));
                if (map.has_value())
                {
                    fields_from_map(std::move(map.value()));
                }
            }

            public:

            using type = Derived;

            static const inline  json::string entity_name = Derived::entity_name;

            static constexpr  size_t req_fields = Derived::req_fields;
            static constexpr  size_t opt_fields = Derived::opt_fields;

            public:
            
            TelegramEntities(){}

            template<is_all_json_entities T>
            void operator=(T&& val)
            {
                json::value val_ = std::forward<T>(val);
                auto map = verify_fields(std::move(val_));
                if (map.has_value())
                {
                    fields_from_map(std::move(map.value()));
                }
            }


            template<is_fields_map T>
            void operator=(T&& map)
            {
                if constexpr (std::is_polymorphic_v<std::remove_reference_t<Derived>>)
                {
                    auto it = map.find(get_entity_name());
                    if (it!=map.end())
                    {
                        auto entity_map = verify_fields(Utils::forward_like<T>(it->second), get_entity_name());
                        if (entity_map.has_value())
                        {
                            fields_from_map(std::move(entity_map.value()));
                        }
                    }
                }
                else
                {
                    auto it = map.find(entity_name);
                    if (it!=map.end())
                    {
                        *this = Utils::forward_like<T>(it->second);
                    }
                }
            }
            

            [[nodiscard]]
            virtual json::string get_entity_name() = 0;

            [[nodiscard]]
            virtual
            opt_fields_map
            verify_fields
            (json::value& val, json::string_view name)
            {
                return {};
            }


            [[nodiscard]]
            virtual 
            opt_fields_map
            verify_fields
            (json::value && val, json::string_view name)
            {
                return {};
            }

            public:

            template<typename...T>
            [[nodiscard]]
            static json::object
            parse_tg_entenies_to_obj(T&&...objs)
            {
                json::object ob{MainParser::get_storage_ptr()};

                auto lamb = [&ob]<typename U>(U&& obj)
                {
                    json::object temp{MainParser::get_storage_ptr()};
                    if constexpr(is_opt<U>)
                    {
                        if constexpr (is_wrapper<typename U::value_type>)
                            temp = (obj.has_value()) ? Utils::forward_like<U>(obj.value().get()).fields_to_value().as_object() : json::object{};
                        else
                            temp = (obj.has_value()) ? Utils::forward_like<U>(obj.value()).fields_to_value().as_object() : json::object{};
                    }
                    else
                    {
                        if constexpr (is_wrapper<U>)
                            temp = Utils::forward_like<U>(obj.get()).fields_to_value().as_object();
                        else
                            temp = std::forward<U>(obj).fields_to_value().as_object();
                    }

                    MainParser::container_move(std::move(temp), ob);
                };

                (lamb(std::forward<T>(objs)),...);

                return ob;
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                return Derived::requested_fields(std::forward<T>(val)); 
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            fields_map
            optional_fields(T&& val)
            {
                return Derived::optional_fields(std::forward<T>(val));
            }


            template<is_fields_map T>
            void
            fields_from_map(T&& map)
            {
                return static_cast<Derived&>(*this).fields_from_map(std::forward<T>(map));
            }


            [[nodiscard]]
            json::string
            fields_to_url() &
            {
                return static_cast<Derived&>(*this).fields_to_url();
            }


            [[nodiscard]]
            json::string
            fields_to_url() &&
            {
                return static_cast<Derived&&>(*this).fields_to_url();
            }

            public:

            template<typename Self>
            [[nodiscard]]
            json::value
            entity_to_value(this Self&& self)
            {
                return self.fields_to_value();
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            verify_fields(T&& val)
            {
                auto req_map = Derived::requested_fields(val);
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = Derived::optional_fields(std::forward<T>(val));

                auto map = std::move(req_map.value());

                for(auto && i : opt_map)
                {
                    map.insert_or_assign(std::move(i.first), std::move(i.second));
                }

                return map;
            }

            virtual ~TelegramEntities() = 0;
        };

        template<typename Derived>
        TelegramEntities<Derived>::~TelegramEntities(){}

    }//namespace TG

}//namespace Pars
