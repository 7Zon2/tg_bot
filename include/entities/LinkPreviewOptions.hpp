#pragma once
#include "TelegramEntities.hpp"


namespace Pars
{
    namespace TG
    {
        struct LinkPreviewOptions : TelegramEntities<LinkPreviewOptions>
        {
            optbool is_disabled;
            optstr url; 
            optbool prefer_small_media;
            optbool prefer_large_media;
            optbool show_above_text;

            static inline size_t req_fields  = 0;
            static inline size_t opt_fields  = 5;
        };
    }
}