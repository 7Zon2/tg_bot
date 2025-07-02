#pragma once
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/json/array.hpp"
#include "boost/url/decode_view.hpp"
#include "entities/YandexRoot.hpp"
#include "head.hpp"
#include "json_head.hpp"
#include "parsing/url_pars.hpp"
#include "session_interface.hpp"
#include "parsing/html_pars.hpp"
#include <utility>
#include <unordered_set>


template<PROTOCOL PR = PROTOCOL::HTTPS>
class Searcher : public session_interface<PR>
{
  protected:

  using session_base_t = session_interface<PR>;

  using session_base_t::req_res;
  using session_base_t::write_request;
  using session_base_t::read_response;
  using session_base_t::stream_;

  Pars::HTML::html_parser parser_;

  protected:

  [[nodiscard]]
  Pars::HTML::html_document 
  make_html_document
  (json::string html)
  {
    Pars::HTML::html_document html_doc = parser_.parse(std::move(html));
    return html_doc;
  }

  public:

  template<typename...Types>
  Searcher
  (
   int version,
   json::string host,
   json::string port, 
   net::any_io_executor executor,
   Types&&...args
  )
  noexcept :
  session_interface<PR>
  (
    version,
    host,
    port,
    executor,
    std::forward<Types>(args)...
  ){}

  public:

  net::awaitable<void>
  run() override
  {
    co_await session_interface<PR>::run();
  }

 
  ///start searching function
  template<typename F>
  net::awaitable<void>
  operator()
  (
    std::optional<json::string> data,
    std::optional<json::string> filename,
    F&& callback
  )
  {
    assert(data || filename);

    if(data)
    {
      co_await request_upload_image
      (
        std::move(data).value(), 
        std::forward<F>(callback)
      );
    }
    else if(filename)
    {
      co_await request_upload_image
      (
        boost::interprocess::file_mapping{}, 
        filename.value(), 
        std::forward<F>(callback)
      );
    }
  }


  template<typename F>
  net::awaitable<void>
  operator()
  (
    Pars::HTML::html_document & doc,
    F && callback
  )
  {
    co_await start_browsing(doc, std::forward<F>(callback));
  }

  protected:

  template<typename F>
  net::awaitable<void>
  request_upload_image
  (
    boost::interprocess::file_mapping map, 
    json::string_view filename, 
    F&& callback
  )
  {
    json::string data = load_file(filename); 
    co_await request_upload_image(std::move(data), std::forward<F>(callback));
  }


  template<typename F>
  net::awaitable<void>
  request_upload_image
  (json::string data, F&& callback)
  {

    auto req = session_base::make_header
      (
        http::verb::post,
        "yandex.ru",
        R"(/images/search/?rpt=imageview&format=json&request={"blocks":[{"block":"b-page_type_search-by-image__link"}]})"
      );


    session_base::prepare_multipart
    (
      req,
      std::make_tuple
      (
        "upfile",
        "blob",
        "image/jpeg",
        std::move(data)
      )
    );


    print("\n",req.target(),"\n");
    auto res = co_await session_base::req_res(*stream_, req);
    http::status status = res.result();
    if
    (
      (status != http::status::found) && 
      (status != http::status::see_other) &&
      (status != http::status::accepted) &&
      (status != http::status::temporary_redirect) &&
      (status != http::status::ok)
    )
    {
      throw std::runtime_error{"\nImage searching request wasn't successful\n"};
    }

    if(status == http::status::ok)
    {
      co_await start_browsing(std::move(res), std::forward<F>(callback));
    }
    else if(status == http::status::temporary_redirect)
    {
      co_await start_redirection(std::move(req), std::forward<F>(callback));
    }
  }


  [[nodiscard]]
  net::awaitable<http::response<http::string_body>>
  make_oneshot_session(json::string_view url)
  {
    print("\nStart OneShot session\n");
    json::string host = make_host(url);
    json::string target = make_relative_url(url);
    auto req = session_base::make_header(http::verb::get, host, target); 
    req.set(http::field::accept_encoding, default_encoding_);
    req.set(http::field::accept, "*/*");
    req.set(http::field::connection,"keep-alive");
    
    print_response(req);

    auto ex = co_await net::this_coro::executor;
    session_interface<PROTOCOL::HTTPS> ses{11, host, "443", ex};
    ses.set_timeout(5000);

    co_await ses.run();

    auto res = co_await ses.redirect(req, target);
    co_return std::move(res);
  }


  template<typename F>
  net::awaitable<void>
  start_browsing
  (
    http::response<http::string_body> res,
    F&& callback
  )
  {
    print("\nSTART BROWSING...\n");

    json::string url = "/images/search?";
    url += parse_answer(std::move(res));

    print("\n\n Parsed Url: ", url,"\n\n");
    auto req = session_base::make_header(http::verb::get, host_, url);
    res = co_await session_base::redirect(req, "/", 5, http::status::found);
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nhtml answer wasn't found\n"};
    }

    json::string html{std::move(res).body()};
    auto html_doc = make_html_document(std::move(html));
    co_await start_browsing(html_doc, std::forward<F>(callback));
  }

  
  [[nodiscard]]
  Pars::HTML::string_vector 
  parse_root_tag 
  (Pars::HTML::html_document& doc)
  {
    auto seek_cbirSimilar = 
    [](Pars::HTML::string_vector& tags)
    {
      Pars::HTML::string_vector vec;
      for(auto && tag : tags)
      {
        if (tag[0] != '{')
        {
          continue;
        }

        tag = Pars::MainParser::align_braces(std::move(tag));
        print("\n",tag,"\n");
        json::value val;

        try
        {
          val = Pars::MainParser::try_parse_message(tag);
          json::value * pv = YandexEntities::find_InitialState(val);
          if(!pv)
          {
            print("\nInitialState wasn't found\n");
            continue;
          }
          val = std::move(*pv);
        }
        catch(const std::exception& ex)
        {
          print("\n", ex.what(), "\n");
          tag = Pars::MainParser::find_object_from_string(tag, "cbirSimilar");
          val = Pars::MainParser::try_parse_message(tag);
        }

        Pars::MainParser::pretty_print(std::cout, val);
        auto opt_thumbs = YandexEntities::find_cbirSimilar_Thumbs(val);
        if(!opt_thumbs)
        {
          continue;
        }

        auto &thumbs = opt_thumbs.value();
        vec.reserve(thumbs.size() * 2);
        for(YandexEntities::Thumb & thumb : thumbs)
        {
          auto opt_url = Pars::URL::find_url_field(thumb.linkUrl, "img_url");
          if(!opt_url)
          {
            continue;
          }
          json::string url = Pars::MainParser::decode_url(opt_url.value());
          vec.push_back(std::move(url));
        }
      }
      return vec;
    };

    Pars::HTML::string_vector urls;
    Pars::HTML::string_vector& vec = Pars::HTML::html_parser::parse_class_name(doc, "Root");
    print("\n\nRoot class:","\nRoot vec size:", vec.size(),"\n\n");
    for(auto& i: vec)
    {
      try
      {
        Pars::HTML::string_vector tags = Pars::HTML::html_parser::html_tokenize(i);
        Pars::HTML::string_vector new_urls = seek_cbirSimilar(tags);
        Pars::MainParser::container_move(std::move(new_urls), urls); 
      }
      catch(const std::exception& ex)
      {
        print(ex.what(),"\n\n");
      }
    }
    return urls;
  }
  
  
  [[nodiscard]]
  bool 
  is_relative_shot
  (json::string_view url) const noexcept 
  {
    static const json::string http_head{"http"};

    json::string substr_http{url.begin(), url.begin() + http_head.size()};
    if(substr_http == http_head)
    {
      return false;
    }

    return true;
  }

  
  template<typename F>
  net::awaitable<void>
  start_browsing 
  (
    Pars::HTML::html_document & html_doc,
    F && callback
  )
  {
    auto it = html_doc.map_.find("href:http");
    if(it == html_doc.map_.end())
    {
      (void)parser_.parse_attributes(html_doc, {{"href", "http"}});
    }

    it = html_doc.map_.find("src:http");
    if(it == html_doc.map_.end())
    {
      (void)parser_.parse_attributes(html_doc, {{"src", "http"}});
    }

    it = html_doc.map_.find("img");
    if (it == html_doc.map_.end())
    {
      (void)parser_.parse_tag(html_doc, "img");
    }

    std::pmr::unordered_set<json::string> set;

    auto & src_vec = html_doc.map_["src:http"];
    print("\nsrc vec size: ", src_vec.size(),"\n");

    auto & img_vec = html_doc.map_["img"];
    print(img_vec, "\nimg vec size: ", img_vec.size(),"\n");

    auto & href_vec = html_doc.map_["href:http"];
    print("\nhref vec size: ", href_vec.size(), "\n");

    auto root_vec = parse_root_tag(html_doc);
    print("\n\n root vec size:", root_vec.size(), "\n\n");
   
    Pars::MainParser::container_move(src_vec,  root_vec);
    Pars::MainParser::container_move(img_vec,  root_vec);
    Pars::MainParser::container_move(href_vec, root_vec);
    set.reserve(root_vec.size());

    for(auto & url : root_vec)
    {
      print("\n",url,"\n");
      auto it = set.insert(url);
      if(it.second == false)
      {
        continue;
      }

      
      try
      {
        http::response<http::string_body> res;
        if(is_relative_shot(url))
        {
          print("\nLocal Shot\n");
          auto req = session_base::make_header(http::verb::get, host_, url);
          req.set(http::field::accept_encoding, default_encoding_);
          req.set(http::field::accept, "*/*");
          req.set(http::field::connection, "keep-alive");
          print_response(req);
          
          res = co_await session_base_t::req_res(std::move(req));
        }
        else
        {
          res = co_await make_oneshot_session(url);
        }

        json::string data = decode_data(res);
        print(data);
      }
      catch(const std::exception& ex)
      {
        print("\n",ex.what(),"\n");
      }
    }
  }
  

  template<typename F>
  net::awaitable<void> 
  start_redirection
  (
    http::request<http::string_body> req,
    F && callback
  )
  {
    print("\nSTART REDIRECTION...\n");
    auto res = co_await session_base::redirect(req, "/");
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nredirection for finding an image wasn't successful\n"};
    }
   
    co_await start_browsing(std::move(res), std::forward<F>(callback)); 
  }


  [[nodiscard]]
  json::string parse_answer
  (http::response<http::string_body> res)
  {
    print("\n\nStart parsing answer...\n\n");

    json::value var = decode_data(std::move(res));
    var = Pars::MainParser::try_parse_value(std::move(var));
    Pars::MainParser::pretty_print(std::cout, var);

    boost::system::error_code er;
    auto * it = var.find_pointer("/blocks/0/params/url", er);
    if(er)
    {
      throw std::runtime_error{"\n\n block pointer wasn't found\n\n"};
    }

    json::string url = it->as_string();
    return url; 
  }

  public:

  json::string default_encoding_{"gzip, deflate, br"};
  static inline json::string host_{"yandex.ru"};
};
