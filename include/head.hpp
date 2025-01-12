#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "json_head.hpp"


 namespace beast = boost::beast;         // from <boost/beast.hpp>
 namespace http = beast::http;           // from <boost/beast/http.hpp>
 namespace net = boost::asio;            // from <boost/asio.hpp>
 namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
 using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


void print_endpoint(const tcp::endpoint& ep)
{
    print("address:", ep.address().to_string(),"\n");
    print("port:", ep.port(),"\n");
    print("protocol:", ep.protocol().protocol(),"\n");
    print("protocol type:", ep.protocol().type(), "\n");
    print("protocol family:", ep.protocol().family(),"\n");
}


void print_result_type(const tcp::resolver::results_type& res)
{
    print("\nserver endpoints:\n");
    print("number of endpoints:", res.size(),"\n");
    for(auto && i : res)
    {
        print("\nendpoint:\n");
        print("service_name:",i.service_name(),"\n");
        print("host_name:", i.host_name(),"\n");
        print_endpoint(i.endpoint());
    }
}


// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat
(
    beast::string_view base,
    beast::string_view path
)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}


enum class RequestErrors : char
{
    bad_request,
    not_found,
    server_error,
    ok
};


template<class Body, class Allocator>
RequestErrors
handle_head
(
    beast::string_view path,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    http::response<http::empty_body>& res_
)
{
    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.data(), beast::file_mode::scan, ec);


    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return RequestErrors::not_found;

    // Handle an unknown error
    if(ec)
        return RequestErrors::server_error;

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    http::response<http::empty_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());

    res_ = std::move(res);

    return RequestErrors::ok;
}


template<class Body, class Allocator>
RequestErrors
handle_get
(
    beast::string_view path,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    http::response<http::file_body>& res_
)
{
    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.data(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return RequestErrors::not_found;

    // Handle an unknown error
    if(ec)
        return RequestErrors::server_error;

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to GET request
    http::response<http::file_body> res
    {
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())
    };

    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());

    res_ = std::move(res);
    
    return RequestErrors::ok;
}


template<class Body, class Allocator>
RequestErrors
handle_post
(
    beast::string_view path,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    http::response<http::string_body>& res_
)
{
    beast::error_code er;
    
    auto& body = req.body();
    auto val = Pars::MainParser::try_parse_message(body);

    if(val.is_null())
    {
        std::cout<<nullptr<<std::endl;
        return RequestErrors::bad_request;
    }
    else
    {

        val.find_pointer("/user", er);
        if(er)
        {
            std::cout<<nullptr<<std::endl;
            return RequestErrors::bad_request;
        }

        auto flag = Pars::MainParser::parse_all_json_to_file(path, std::move(val));
        if(!flag)
        {
            std::cout<<"path was not found\n";
            return RequestErrors::not_found;
        }

        std::string str{"Ok"};

        http::response<http::string_body> res{http::status::created, req.version()};
        res.keep_alive(req.keep_alive());
        res.set(http::field::location, path);
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_location, path);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(str.size());
        res.body()  = str;
        res.prepare_payload();

        res_ = std::move(res);

        return RequestErrors::ok;
    }
}


template<typename Body>
http::message_generator
prepare_response
(
    http::response<Body>&& res_,
    RequestErrors status
)
{

    // Returns a bad request response
    auto const bad_request =
    [&](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, res_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(res_.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };


    // Returns a not found response
    auto const not_found =
    [&](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::not_found, res_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(res_.keep_alive());
        res.body() = std::string(what);
        res.prepare_payload();
        return res;
    };


    // Returns a server error response
    auto const server_error =
    [&](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, res_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(res_.keep_alive());
        res.body() = std::string(what);
        res.prepare_payload();
        return res;
    };


    switch (status)
    {
        case RequestErrors::bad_request  : return bad_request("Request is wrong");
        
        case RequestErrors::not_found    : return not_found("Requested file is not found");

        case RequestErrors::server_error : return server_error("Internal server error");

        case RequestErrors::ok : return std::move(res_);

        default: 
            return server_error("Unknown error");
    }

}


// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator
handle_request
(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req
)
{
 
    // Make sure we can handle the method
    if( req.method() != http::verb::get  &&
        req.method() != http::verb::head &&
        req.method() != http::verb::post)
    {
        http::response<http::empty_body> res{http::status::bad_request, req.version()};
        res.keep_alive(req.keep_alive());
        return prepare_response(std::move(res), RequestErrors::bad_request);
    }
       


    // Request path must be absolute and not contain "..".
    if( req.target().empty()    ||
        req.target()[0] != '/'  ||
        req.target().find("..") != beast::string_view::npos)
        {
            http::response<http::empty_body> res{http::status::not_acceptable, req.version()};
            res.keep_alive(req.keep_alive());
            return prepare_response(std::move(res), RequestErrors::not_found);
        }

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());


    if (req.method() == http::verb::get)
    {
        http::response<http::file_body> res;
        RequestErrors code = handle_get(path, std::move(req), res);
        return prepare_response(std::move(res), code);
    }


    if(req.method() == http::verb::head)
    {
        http::response<http::empty_body> res;
        RequestErrors code = handle_head(path, std::move(req), res);
        return prepare_response(std::move(res), code);
    }


    if(req.method() == http::verb::post)
    {
        http::response<http::string_body> res;
        RequestErrors code = handle_post(path, std::move(req), res);
        return prepare_response(std::move(res), code);
    }


    http::response<http::empty_body> res{http::status::bad_request, req.version()};
    res.keep_alive(req.keep_alive());
    return prepare_response(std::move(res), RequestErrors::bad_request);

}


// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    if(ec == net::ssl::error::stream_truncated)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}
