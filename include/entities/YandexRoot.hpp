#include "TelegramEntities.hpp"
#include "boost/json/kind.hpp"
#include "json_head.hpp"


namespace YandexEntities
{
  using namespace Pars;

  class Thumbs : Pars::TG::TelegramEntities<Thumbs>
  {
    protected:

    json::string imageUrl;
    size_t width = 0;
    size_t height = 0;
    json::string title;
    json::string linkUrl;

    public:

    static const inline json::string entity_name{"Thumbs"};
    static const constexpr size_t req_fields = 4;
    static const constexpr size_t opt_fields = 0;

    json::string 
    get_entity_name() noexcept override
    {
      return "Thumbs";
    }

    public:

    Thumbs() noexcept {}

    Thumbs
    (
      json::string imageUrl,
      size_t width,
      size_t height,
      json::string title,
      json::string linkUrl
    )
    noexcept:
    imageUrl(std::move(imageUrl)),
    width(width),
    height(height),
    title(title),
    linkUrl(std::move(linkUrl))
    {

    }

    public:

    template<typename T>
    [[nodiscard]]
    static opt_fields_map 
    optional_fields(T&& val)
    {
      return {};
    }


    template<typename T>
    [[nodiscard]]
    static fields_map 
    requested_fields(T&& val)
    {
      auto map = MainParser::mapped_pointers_validation
        (
          std::forward<T>(val),
          std::make_pair(JSP(imageUrl), json::kind::string),
          std::make_pair(JSP(width), json::kind::int64),
          std::make_pair(JSP(height), json::kind::int64),
          std::make_pair(JSP(title), json::kind::string),
          std::make_pair(JSP(linkUrl), json::kind::string)
        );

      if(req_fields != map.size())
      {
        return {};
      }

      return map;
    }


    template<is_fields_map T>
    void 
    fields_from_map(T && map)
    {
      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), MAKE_PAIR(imageUrl, imageUrl));
      
      MainParser::field_from_map
      <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(width, width));

      MainParser::field_from_map
      <json::kind::double_>(std::forward<T>(map), MAKE_PAIR(height, height));

      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), MAKE_PAIR(title, title));

      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), MAKE_PAIR(linkUrl, linkUrl));
    }


    template<typename Self>
    [[nodiscard]]
    json::value
    fields_to_value(this Self&& self)
    {
      return Thumbs::fields_to_value
        (
          Utils::forward_like<Self>(self.imageUrl),
          Utils::forward_like<Self>(self.width),
          Utils::forward_like<Self>(self.height),
          Utils::forward_like<Self>(self.title),
          Utils::forward_like<Self>(self.linkUrl)
        );
    }


    [[nodiscard]]
    static json::value
    fields_to_value
    (
      json::string imageUrl,
      size_t width,
      size_t height,
      json::string title,
      json::string linkUrl
    )
    {
      json::object ob{MainParser::get_storage_ptr()};
      ob = MainParser::parse_ObjPairs_as_obj
        (
          PAIR(imageUrl, std::move(imageUrl)),
          PAIR(width, width),
          PAIR(height, height),
          PAIR(title, std::move(title)),
          PAIR(linkUrl, linkUrl)
        );

      json::object res{MainParser::get_storage_ptr()};
      res["Thumbs"] = std::move(ob);
      return res;
    }

  };


  class YandexRoot : Pars::TG::TelegramEntities<YandexRoot>
  {

  };


} //YandexEntities
