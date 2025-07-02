#pragma once
#include "head.hpp"
#include <lexbor/core/base.h>
#include <lexbor/core/types.h>


namespace Pars
{
  namespace LXB
  {
    using lxb_func_t = std::function<lxb_status_t(const lxb_char_t*, size_t)>;

    [[nodiscard]]
    inline const lxb_char_t*
    lxb_cast(std::string_view view) noexcept
    {
      return reinterpret_cast<const lxb_char_t*>(view.data());
    }


    [[nodiscard]]
    inline const char*
    lxb_cast(const lxb_char_t* data) noexcept
    {
      if(!data)
      {
        return "";
      }
      return reinterpret_cast<const char*>(data);
    }


    inline lxb_status_t
    lxb_generic_callback
    (const lxb_char_t * data, size_t len, void * ctx)
    {
      auto * fun = static_cast<lxb_func_t*>(ctx);
      return (*fun)(data, len);
    }
  }
}
