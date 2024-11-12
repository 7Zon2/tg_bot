#pragma once
#include <exception>



struct BadRequestException : public std::exception
{
    const char * 
    what() const noexcept override
    {
        return "BadRequest 404";
    }
};