#pragma once 
#include "json_head.hpp"
#include "TelegramEntities.hpp"


namespace Pars
{
  namespace TG
  {
    struct File : public TelegramEntities<File>
    {
      public:

        using  TelegramEntities<File>::operator =;

        json::string file_id = {};
        json::string file_unique_id = {};
        optdouble file_size = {};
        optstr file_path = {};

        static const constexpr size_t req_fields = 2;
        static const constexpr size_t opt_fields = 2;

        static const inline json::string entity_name{"file"};

        [[nodiscard]]
        json::string
        get_entity_name() noexcept override 
        {
          return entity_name;
        }

      public:

        File() noexcept {}
        
        File
        (
         json::string file_id,
         json::string file_unique_id,
         optdouble file_size  = {},
         optstr file_path = {}
        )
        noexcept 
        :
            file_id(std::move(file_id)),
            file_unique_id(std::move(file_unique_id)),
            file_size(file_size),
            file_path(std::move(file_path))
        {

        }

        template<is_all_json_entities T>
        File(T&& obj)
        {
          create(std::forward<T>(obj));
        }

        virtual ~File(){}

      public:

        template<as_json_value T>
        [[nodiscard]]
        static 
        opt_fields_map
        requested_fields(T&& val, json::string inherited_name = "file")
        {
          auto map = MainParser::mapped_pointers_validation
            (
              std::forward<T>(val),
              std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_id)), json::kind::string),
              std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_unique_id)), json::kind::string)
            );

          if(req_fields != map.size())
          {
            return {};
          }

          return map;
        }


      template<as_json_value T>
      [[nodiscard]]
      static 
      fields_map
      optional_fields(T&& val, json::string inherited_name = "file")
      {
        return MainParser::mapped_pointers_validation
        (
            std::forward<T>(val),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_size)), json::kind::double_),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_path)), json::kind::string)
        );
      }


      template<is_fields_map T>
      void
      fields_from_map(T && map)
      {
        MainParser::field_from_map
        <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_id, file_id));

        MainParser::field_from_map
        <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_unique_id, file_unique_id));

        MainParser::field_from_map
        <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(file_size, file_size));

        MainParser::field_from_map
        <json::kind::string>(std::forward<T>(map), MAKE_PAIR(file_path, file_path));
      }


      template<typename Self> 
      [[nodiscard]]
      json::value 
      fields_to_value(this Self&& self)
      {
        return File::fields_to_value
          (
            Utils::forward_like<Self>(self.file_id),
            Utils::forward_like<Self>(self.file_unique_id),
            self.file_size,
            Utils::forward_like<Self>(self.file_path)
          );
      }


      [[nodiscard]]
      static json::value 
      fields_to_value
      (
        json::string file_id,
        json::string file_unique_id,
        optdouble file_size = {},
        optstr file_path = {}
      )
      {
        json::object ob{MainParser::get_storage_ptr()};
        ob = MainParser::parse_ObjPairs_as_obj
          (
            PAIR(file_id, std::move(file_id)),
            PAIR(file_unique_id, std::move(file_unique_id))
          );

        json::object ob2{MainParser::get_storage_ptr()};
        ob2 = MainParser::parse_OptPairs_as_obj
          (
            MAKE_OP(file_size, file_size),
            MAKE_OP(file_path, std::move(file_path))
          );

        Pars::MainParser::container_move(std::move(ob2), ob);

        json::object res(MainParser::get_storage_ptr());
        res["file"] = std::move(ob);
        return res;
      }

    };

  }//namespace TG
  
}//namespace Pars
