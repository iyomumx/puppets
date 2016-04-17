#pragma once

#include <stdexcept>
#include <Windows.h>

namespace win32
{

    class win32_exception : public std::exception
    {
    private:
        HRESULT err_code;
        mutable char * message = nullptr;
    public:
        win32_exception() : win32_exception(GetLastError()) {}
        win32_exception(HRESULT hr) : err_code(hr) {}
        win32_exception(const win32_exception& other) : err_code(other.err_code)
        {
            SIZE_T sz = LocalSize(other.message);
            message = reinterpret_cast<char *>(LocalAlloc(0, sz));
            CopyMemory(message, other.message, sz);
        }
        win32_exception(win32_exception&& other) : err_code(other.err_code)
        {
            std::swap(message, other.message);
        }
        win32_exception& operator=(const win32_exception& other)
        {
            if (&other != this)
            {
                this->~win32_exception();
                new (this) win32_exception(other);
            }
        }
        win32_exception& operator=(win32_exception&& other)
        {
            err_code = other.err_code;
            std::swap(message, other.message);
        }
        const char * what() const override
        {
            if (!message)
            {
                FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    err_code,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&message),
                    0, NULL);
            }
            return message;
        }
        const HRESULT& error_code() const { return err_code; }
        ~win32_exception() { if (message) LocalFree(reinterpret_cast<HLOCAL>(message)); }
        static inline void throw_if_fail(HRESULT hr) noexcept(false) { if (FAILED(hr)) throw win32_exception(hr); }
    };
}