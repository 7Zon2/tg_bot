#pragma once
#include "lxb_pars.hpp"
#include <lexbor/unicode/idna.h>
#include <lexbor/url/url.h>

namespace Pars
{

namespace URL
{
  using unique_url_parser = 
  std::unique_ptr<lxb_url_parser_t, decltype([](auto * p){lxb_url_parser_destroy(p, true);})>;

  using unique_url = 
  std::unique_ptr<lxb_url_t, decltype([](auto *p){lxb_url_memory_destroy(p);})>;

  using unique_idna = 
  std::unique_ptr<lxb_unicode_idna_t, decltype([](auto *p){lxb_unicode_idna_destroy(p, true);})>;


  [[nodiscard]]
  inline std::optional<json::string>
  find_url_field
  (json::string_view url, json::string field_name)
  {
    field_name +="=";
    size_t begin = url.find(field_name);
    if(begin == json::string::npos)
    {
      return {};
    }
    
    begin += field_name.size();
    size_t end = url.find("&", begin);
    if(end == json::string::npos)
    {
      end = url.size();
    }
    return json::string{url.begin() + begin, url.begin() + end};
  }


  [[nodiscard]]
  inline auto 
  make_default_url_callback
  (json::string& str)
  {
    return LXB::lxb_func_t{[&](const lxb_char_t* ch, size_t len)
      {
        str += json::string{LXB::lxb_cast(ch)};
        return LXB_STATUS_OK;
      }}; 
  }


  struct url_document
  {
    unique_url_parser parser;
    unique_url  url;
    unique_idna idna;

    public:

    [[nodiscard]]
    json::string 
    serialize_url() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize(url.get(), LXB::lxb_generic_callback, &fun, false);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_domen() const 
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_scheme(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }

    
    [[nodiscard]]
    json::string
    serialize_username() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_username(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_password() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_password(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }

    
    [[nodiscard]]
    json::string
    serialize_host() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_host(lxb_url_host(url.get()), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_host_unicode() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_host_unicode(idna.get(), lxb_url_host(url.get()), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_port() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_port(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_path() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_path(lxb_url_path(url.get()), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_query() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_query(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }


    [[nodiscard]]
    json::string
    serialize_fragment() const
    {
      json::string str;
      LXB::lxb_func_t fun = make_default_url_callback(str);
      lxb_url_serialize_fragment(url.get(), LXB::lxb_generic_callback, &fun);
      return str;
    }
  };



  [[nodiscard]]
  inline auto 
  make_url_parser()
  {
    unique_url_parser parser{new lxb_url_parser_t{}};
    auto status = lxb_url_parser_init(parser.get(), nullptr);
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"\nurl parser init failed\n"};
    return parser;
  }


  [[nodiscard]]
  inline auto 
  make_url_idna()
  {
    unique_idna idna{new lxb_unicode_idna_t{}};
    auto status = lxb_unicode_idna_init(idna.get());
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"\nFailed to Init IDNA\n"};
    return idna;
  }


  [[nodiscard]]
  url_document
  inline pars_url
  (json::string_view vw)
  {
    auto parser = make_url_parser();

    unique_url url{lxb_url_parse(parser.get(), nullptr, LXB::lxb_cast(vw), vw.size())};
    if(!url)
      throw std::runtime_error{"\nFailed to parse url\n"};

    auto idna = make_url_idna();

    return url_document
    {
      .parser = std::move(parser), 
      .url = std::move(url), 
      .idna = std::move(idna)
    };
  }


  [[nodiscard]]
  url_document
  inline pars_url
  (url_document& base_url, json::string_view vw)
  {
    auto parser = make_url_parser();

    unique_url url{lxb_url_parse(parser.get(), base_url.url.get(), LXB::lxb_cast(vw), vw.size())};
    if(!url)
      throw std::runtime_error{"\nFailed to parse url\n"};

    auto idna = make_url_idna();

    return url_document
    {
      .parser = std::move(parser),
      .url = std::move(url),
      .idna = std::move(idna)
    };
  }


}//namespace URL

}//namespace Pars
