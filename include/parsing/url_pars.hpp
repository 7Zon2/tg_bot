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
  std::unique_ptr<lxb_url_t, decltype([](auto *p){lxb_url_destroy(p);})>;

  using unique_idna = 
  std::unique_ptr<lxb_unicode_idna_t, decltype([](auto *p){lxb_unicode_idna_destroy(p, true);})>;


  struct url_document
  {
    unique_url_parser parser;
    unique_url  url;
    unique_idna idna;

    [[nodiscard]]
    json::string 
    serialize_url()
    {
      json::string str;
      LXB::lxb_func_t fun =
      [&](const lxb_char_t* ch, size_t len)
      {
        str = json::string{LXB::lxb_cast(ch)};
        return LXB_STATUS_OK;
      };
      lxb_url_serialize(url.get(), LXB::lxb_generic_callback, &fun, false);
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

    url_document doc
    {
      .parser = std::move(parser), 
      .url = std::move(url), 
      .idna = std::move(idna)
    };
    return doc;
  }


}//namespace URL

}//namespace Pars
