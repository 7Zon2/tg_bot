#pragma once
#include "PhotoSize.hpp"


namespace Pars
{
  namespace TG
  {
    class Document : protected PhotoSize
    {
      public:

      using File::operator=;

      static const constexpr size_t req_fields = 2;
      static const constexpr size_t opt_fields = 3;

      static const inline json::string entity_name{"document"};

      [[nodiscard]]
      json::string
      get_entity_name() noexcept override 
      {
        return entity_name;
      }

      public:

      std::optional<PhotoSize> thumbnail{};
      optstr file_name{};
      json::stirng mime_type{};

      public:

      Document() noexcept {}
      
      Document(File file) noexcept :
        PhotoSize(std::move(file)){}
      
      template<is_all_json_entities T>
      Document(T&& obj)
      {
        create(std::forward<T>(obj));
      }

      Document
      (
        json::string file_id,
        json::string file_unique_id,
        std::optional<PhotoSize> thumbnail = {},
        optstr file_name = {},
        optstr mime_type = {},
        optdouble file_size = {}
      )
      noexcept 
      :
        PhotoSize
        (
          std::move(file_id),
          std::move(file_unique_id),
          0,
          0,
          file_size
        ),
        file_name(std::move(file_name)),
        mime_type(std::move(mime_type))
      {

      }

      ~Document(){}

      public:

      template<as_json_value T>
      [[nodiscard]]
      static
      opt_fields_map
      requested_fields(T&& val)
      {
        return File::requested_fields(std::forward<T>(val), "document");
      }


      template<as_json_value T>
      [[nodiscard]]
      static 
      fields_map 
      optional_fields(T&& val, json::string inherited_name = get_entity_name())
      {
        return MainParser::mapped_pointers_validation
        (
            std::forward<T>(val),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(thumbnail)), json::kind::object),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_name)), json::kind::string),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(mime_type)), json::kind::string),
            std::make_pair(MainParser::make_json_pointer(inherited_name, FIELD_NAME(file_size)), json::kind::double_)
        );
      }

      
      template<is_fields_map T>
      void 
      fields_from_map(T && map)
      {
        File::fields_from_map(std::forward<T>(map));

        MainParser::field_from_map
        <json::kind::object>(std::forward<T>(map), MAKE_PAIR(thumbnail, thumbnail);

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
        return Document::fields_to_value
        (
          Utils::forward_like<Self>(self.file_id),
          Utils::forward_like<Self>(self.file_unique_id),
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
        json::stirng file_unique_id,
        std::optional<PhotoSize> thumbnail = {},
        optstr file_name = {},
        optstr mime_type = {},
        optdouble file_size = {}
      )
      {
        json::object ob{MainParser::get_storage_ptr()};
        ob = MainParser::parse_ObjPairs_as_obj
          (
            PAIR(file_id, std::move(file_id)),
            PAIR(file_unique_id, std::move(file_unique_id))
          );

        json::object opt_ob{MainParser::get_storage_ptr()};
        opt_ob = MainParser::parse_OptPairs_as_obj
          (
              PAIR(thumbnail, std::move(thumbnail)),
              PAIR(file_name, std::move(file_name)),
              PAIR(mime_type, std::move(mime_type)),
              PAIR(file_size, file_size)
          );

        Pars::MainParser::container_move(std::move(opt_ob), ob);

        json::object res(MainParser::get_storage_ptr());
        res[get_entity_name()] = std::move(ob); 
        return res;
      }

    };    

  }// namespace TG
}// namespace Pars
