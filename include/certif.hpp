#pragma once
#include "head.hpp"

namespace CRTF
{
    json::string 
    load_cert(std::string_view filename)
    {
        static unsigned char buf[4096]{};
        json::string str;

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

    void load_cert(ssl::context& ctx,const std::string& file)
    {
        boost::system::error_code er;
        ctx.load_verify_file(file,er);
        if(er)
        {
            fail(er,"load_verify_file failed");
                throw boost::system::system_error{er};
        }
    }

    void set_cert
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