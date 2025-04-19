#pragma once
#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/verify_mode.hpp"
#include <boost/asio/ssl.hpp>
#include <stdexcept>

namespace ssl = boost::asio::ssl;

namespace CRTF
{
  
    [[nodiscard]]
    inline ssl::context
    load_default_client_ctx
    (ssl::context::method method = ssl::context::tlsv12_client)
    {
      ssl::context ctx{method};
      ctx.set_default_verify_paths();
      ctx.set_verify_mode(ssl::verify_peer);
      return ctx;
    }


    [[nodiscard]]
    inline std::string 
    load_cert(std::string_view filename)
    {
        static unsigned char buf[4096]{};
        std::string str;

        std::FILE* f = std::fopen(filename.data(), "r");
        if(!f)
            return {};

        while(! std::feof(f))
        {
            size_t sz = std::fread(buf, sizeof(char), sizeof(buf), f);
            str.append(buf, buf + sz);
        }

        std::fclose(f);
        return str;
    }

    inline void 
    load_cert(ssl::context& ctx,const std::string& file)
    {
        boost::system::error_code er;
        (void) ctx.load_verify_file(file,er);
        if(er)
        {
          throw std::runtime_error{er.what()};
        }
    }

    inline void 
    set_cert
    (
        ssl::context& ctx, 
        std::string key,
        std::string cert,
        std::string  password
    )
    {
        ctx.set_password_callback
        (
            [&](std::size_t, ssl::context_base::password_purpose)
            {
                return password;
            }
        );

        ctx.set_options
        (
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 
        );

        ctx.use_certificate_chain_file(cert);

        ctx.use_private_key_file(key, ssl::context::file_format::pem);
    }

}//namespace CRTF
