#pragma once
#include "concept_entities.hpp"
#include "json_head.hpp"
#include "tg_requestes.hpp"

namespace Pars
{
    namespace TG
    {
        template<typename Derived>
        struct TelegramEntities
        {
            protected:

            template<typename Self, is_all_json_entities T>
            void create(this Self&& self, T&& val)
            {
                json::value val_ = std::forward<T>(val);
                auto map = self.verify_fields(std::move(val_));
                if (map.has_value())
                {
                  self.fields_from_map(std::move(map.value()));
                }
            }

            public:

            using selfType = TelegramEntities<Derived>;
            using type = Derived;

            static const inline  json::string entity_name = Derived::entity_name;

            static constexpr  size_t req_fields = Derived::req_fields;
            static constexpr  size_t opt_fields = Derived::opt_fields;

            public:
            
            TelegramEntities(){}

            template<typename Self, is_all_json_entities T>
            void operator = (this Self&& self, T&& val)
            {
                json::value val_ = std::forward<T>(val);
                auto map = self.verify_fields(std::move(val_));
                if (map.has_value())
                {
                    self.fields_from_map(std::move(map.value()));
                }
            }


            template<typename Self, is_fields_map T>
            void operator=(this Self&& self, T&& map)
            {
                if constexpr (std::is_polymorphic_v<std::remove_reference_t<Derived>>)
                {
                    auto it = map.find(self.get_entity_name());
                    if (it!=map.end())
                    {
                        auto entity_map = self.verify_fields(Utils::forward_like<T>(it->second), self.get_entity_name());
                        if (entity_map.has_value())
                        {
                            self.fields_from_map(std::move(entity_map.value()));
                        }
                    }
                }
                else
                {
                    auto it = map.find(entity_name);
                    if (it!=map.end())
                    {
                        self = Utils::forward_like<T>(it->second);
                    }
                }
            }
            

            [[nodiscard]]
            virtual json::string get_entity_name() noexcept = 0;

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

                auto obj_pars = [&ob]<typename U>(U&& obj)
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


                auto type_pars = [&obj_pars]<typename U>(U&& obj)
                {
                  if constexpr(is_opt<U>)
                  {
                    bool constexpr is_container = requires {obj.value().begin(); obj.value().end();};
                    if constexpr(is_container)
                    {                    
                      for(auto&& i : obj.value())
                      {
                        obj_pars(Utils::forward_like<U>(i));
                      }
                    }
                    else
                    {
                      obj_pars(std::forward<U>(obj));
                    }
                  }
                  else
                  {
                    bool constexpr is_container = requires{obj.begin(); obj.end();};
                    if constexpr(is_container)
                    {
                      for(auto&& i : obj)
                      {
                        obj_pars(Utils::forward_like<U>(i));
                      }
                    }
                    else
                    {
                      obj_pars(std::forward<U>(obj));
                    }
                  }
                };

                (type_pars(std::forward<T>(objs)),...);

                return ob;
            }


            template<as_json_value T>
            [[nodiscard]]
            static opt_fields_map
            requested_fields(T&& val)
            {
                return Derived::requested_fields(std::forward<T>(val)); 
            }


            template<as_json_value T>
            [[nodiscard]]
            static fields_map
            optional_fields(T&& val)
            {
                return Derived::optional_fields(std::forward<T>(val));
            }


            template<typename D = Derived, is_fields_map T>
            void
            fields_from_map(T&& map)
            {
                return static_cast<D&>(*this).fields_from_map(std::forward<T>(map));
            }


            template<typename Self>
            [[nodiscard]]
            json::string
            field_to_url
            (this Self&& self)
            {
              return self.fields_to_url();
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

                auto map = std::move(req_map).value();

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
