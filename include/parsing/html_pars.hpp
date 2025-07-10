#pragma once
#include "parsing/lxb_pars.hpp"
#include <lexbor/dom/collection.h>
#include <lexbor/dom/interface.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/html/base.h>
#include <lexbor/html/interface.h>
#include <lexbor/html/interfaces/document.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/tokenizer.h>

namespace Pars
{

namespace HTML
{

  using unique_html = std::unique_ptr
  <lxb_html_document_t, decltype([](auto* p){lxb_html_document_destroy(p);})>;

  using unique_coll = std::unique_ptr
  <lxb_dom_collection_t, decltype([](auto*p){lxb_dom_collection_destroy(p,true);})>;

  using unique_tkz = std::unique_ptr
  <lxb_html_tokenizer_t, decltype([](auto *p){lxb_html_tokenizer_destroy(p);})>;

  using string_vector = std::pmr::vector<json::string>;


  struct html_document
  {
    json::string html_;
    unique_html doc_;

    using html_map_t = std::pmr::unordered_map<json::string, string_vector>;
    html_map_t map_;

    public:

    html_document(json::string html, unique_html doc)
      noexcept:
      html_(std::move(html)),
      doc_(std::move(doc))
      {}
  };


struct html_parser
{

  protected:

  ///makes a key for map like 'name:value'
  [[nodiscard]]
  static json::string
  make_attribute_key
  (json::string_view name, json::string_view value)
  {
    json::string str;
    str.reserve(name.size() + value.size() + 1); 
    str += name;
    str += ":";
    str += value;
    return str;
  }


  static void
  iteration_over_element
  (
    lxb_dom_element_t & el, 
    string_vector & vec, 
    size_t limit = -1
  )
  {
    size_t i = 0;
    for(auto b = el.first_attr; b != nullptr && i < limit; b = b->next, ++i)
    {
      const char * data = LXB::lxb_cast(b->value->data);
      json::string str{data};
      vec.push_back(std::move(str));
    }
  }


  [[nodiscard]]
  static string_vector
  iteration_over_collection
  (
    lxb_dom_collection_t& coll,
    size_t limit = -1
  )
  {
    size_t lenght = lxb_dom_collection_length(&coll);
    string_vector vec;
    vec.reserve(lenght);
    for(size_t i = 0; i < lenght; i++)
    {
      lxb_dom_element_t * el = lxb_dom_collection_element(&coll, i);
      iteration_over_element(*el, vec, limit);
    }
    return vec;
  }


  [[nodiscard]]
  static auto 
  make_collection(lxb_dom_document_t& doc, size_t sz)
  {
    unique_coll coll{lxb_dom_collection_make(&doc, sz)};
    if(coll == nullptr)
      throw std::runtime_error{"\nfailed to make collection\n"};

    return coll;
  }

  public:

  [[nodiscard]]
  static 
  string_vector&
  parse_class_name
  (
    html_document& doc,
    std::string_view name,
    size_t size = 128
  )
  {

    auto& d = doc.doc_;
    auto coll = make_collection(d->dom_document, size);

    lxb_dom_element_t * el =  lxb_dom_interface_element(d->body);
    lxb_status_t status = lxb_dom_elements_by_class_name
    (el, coll.get(), LXB::lxb_cast(name), name.size()); 

    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"Failed to get elements by class name"};

    auto vec = iteration_over_collection(*coll);
    auto& map = doc.map_;
    json::string key{name};
    map[key] = std::move(vec);
    return map.find(key)->second;
  }


  [[nodiscard]] 
  static 
  html_document::html_map_t&
  parse_attributes
  (
    html_document& document, 
    std::initializer_list<std::pair<json::string_view,json::string>> attributes,
    size_t collection_sz = 128,
    bool case_insensitive = true
  )
  {
    auto* doc = document.doc_.get();

    unique_coll coll = make_collection(doc->dom_document, collection_sz);
    lxb_dom_element_t * body = lxb_dom_interface_element(doc->body);

    for(auto& pa : attributes)
    {
      auto name = pa.first;
      auto value = pa.second;
      lxb_status_t status = lxb_dom_elements_by_attr_contain
      (
        body,
        coll.get(),
        LXB::lxb_cast(name),  name.size(),
        LXB::lxb_cast(value), value.size(),
        case_insensitive
      );

      if(status != LXB_STATUS_OK)
        continue;

      auto vec  = iteration_over_collection(*coll, 1);
      auto& map = document.map_;
      auto key = make_attribute_key(name, value);
      map.insert_or_assign(std::move(key), std::move(vec));
    }

    return document.map_;
  }


  [[nodiscard]]
  static string_vector&
  parse_tag(html_document& document, json::string_view tag)
  {
     size_t size = 1024;
     auto* doc = document.doc_.get();
     
     unique_coll coll = make_collection(doc->dom_document, size);
     lxb_html_body_element_t * body = lxb_html_document_body_element(doc);
     lxb_dom_element_t * elem = lxb_dom_interface_element(body);

     lxb_status_t status = lxb_dom_elements_by_tag_name
     (
        elem, 
        coll.get(), 
        (const lxb_char_t *)(tag.data()),
        tag.size()
    );


    if(status != LXB_STATUS_OK || lxb_dom_collection_length(coll.get()) == 0)
      throw std::runtime_error{"\nfailed to find element\n"};

    string_vector vec = iteration_over_collection(*coll);
    auto pair = document.map_.insert_or_assign(tag, std::move(vec));
    return pair.first->second;
  }
  

  [[nodiscard]]
  static html_document 
  parse(json::string html)
  {
    auto html_callback = [](const lxb_char_t* data, size_t len, void * ctx) -> lxb_status_t
    {
      json::string str{data, data + len};
      return LXB_STATUS_OK;
    };

    print("\nCreating HTML parser...\n");
    lxb_html_parser_t* parser = lxb_html_parser_create();
    lxb_status_t status = lxb_html_parser_init(parser);
    if(status != LXB_STATUS_OK)
    {
      throw std::runtime_error{"\nFailed to create HTML parser\n"};
    }

    const unsigned char * data = LXB::lxb_cast(html);
    const size_t sz = html.size() - 1;

    print("\n\nParsing html document...\n\n");
    unique_html  doc{lxb_html_parse(parser, data , sz)};
    if(doc == nullptr)
    {
      throw std::runtime_error{"\nFaield to create Document object\n"};
    }

    lxb_dom_node_t* node = lxb_dom_interface_node(doc.get());
    status = lxb_html_serialize_pretty_tree_cb
    (node, LXB_HTML_SERIALIZE_OPT_UNDEF, 0, html_callback, nullptr);

    if(status != LXB_STATUS_OK)
    {
      throw std::runtime_error{"\nFailed to serialization HTML tree\n"};
    }

    return html_document{std::move(html), std::move(doc)};
  }


  static string_vector
  html_tokenize(json::string_view data)
  {
    string_vector vec;
    vec.reserve(data.size());
    auto callback = []
    (lxb_html_tokenizer_t * tkz, lxb_html_token_t * token, void * ctx) -> lxb_html_token_t *
    {
      auto b = token->begin;
      auto e = token->end;
      string_vector *pv = static_cast<string_vector*>(ctx);
      pv->push_back(json::string{b,e});
      return token;
    };

    unique_tkz tkz{ lxb_html_tokenizer_create() };
    lxb_status_t status = lxb_html_tokenizer_init(tkz.get());
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"Failed to create tokenizer\n"};

    lxb_html_tokenizer_callback_token_done_set(tkz.get(), callback, &vec);
    
    status = lxb_html_tokenizer_begin(tkz.get());
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"Failed to prepare token for parsing\n"};

    status = lxb_html_tokenizer_chunk(tkz.get(), LXB::lxb_cast(data), data.size() - 1);
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"Failed to parser the html data\n"};

    status = lxb_html_tokenizer_end(tkz.get());
    if(status != LXB_STATUS_OK)
      throw std::runtime_error{"Failed to ending of parsing the html data\n"};

    return vec;
  }

};


}//namespace HTML 


}//namespace Pars
