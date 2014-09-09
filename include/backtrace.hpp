#pragma once

//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// #ifndef BOOST_BACKTRACE_HPP
// #define BOOST_BACKTRACE_HPP

#include <boost/config.hpp>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <iosfwd>

namespace boost {

    namespace stack_trace {
        int trace(void **addresses,int size);
        void write_symbols(void *const *addresses,int size,std::ostream &);
        std::string get_symbol(void *address);
        std::string get_symbols(void * const *address,int size);
    } // stack_trace

    class backtrace {
    public:
        
        static size_t const default_stack_size = 32;

        backtrace(size_t frames_no = default_stack_size) 
        {
            if(frames_no == 0)
                return;
            frames_.resize(frames_no,0);
            int size = stack_trace::trace(&frames_.front(),frames_no);
            frames_.resize(size);
        }

        virtual ~backtrace() throw()
        {
        }

        size_t stack_size() const
        {
            return frames_.size();
        }

        void *return_address(unsigned frame_no) const
        {
            if(frame_no < stack_size())
                return frames_[frame_no];
            return 0;
        }

        void trace_line(unsigned frame_no,std::ostream &out) const
        {
            if(frame_no < frames_.size())
                stack_trace::write_symbols(&frames_[frame_no],1,out);
        }

        std::string trace_line(unsigned frame_no) const
        {
            if(frame_no < frames_.size())
                return stack_trace::get_symbol(frames_[frame_no]);
            return std::string();
        }

        std::string trace() const
        {
            if(frames_.empty())
                return std::string();
            return stack_trace::get_symbols(&frames_.front(),frames_.size());
        }

        void trace(std::ostream &out) const
        {
            if(frames_.empty())
                return;
            stack_trace::write_symbols(&frames_.front(),frames_.size(),out);
        }
    
    private:
        std::vector<void *> frames_;
    };

    //class exception : public std::exception, public backtrace {
    //public:
    //};
    
    class bad_cast : public std::bad_cast, public backtrace {
    public:
    };

    class runtime_error: public std::runtime_error, public backtrace {
    public:
        explicit runtime_error(std::string const &s) : std::runtime_error(s) 
        {
        }
    };

    class range_error: public std::range_error, public backtrace {
    public:
        explicit range_error(std::string const &s) : std::range_error(s) 
        {
        }
    };

    class overflow_error: public std::overflow_error, public backtrace {
    public:
        explicit overflow_error(std::string const &s) : std::overflow_error(s) 
        {
        }
    };

    class underflow_error: public std::underflow_error, public backtrace {
    public:
        explicit underflow_error(std::string const &s) : std::underflow_error(s) 
        {
        }
    };

    class logic_error: public std::logic_error, public backtrace {
    public:
        explicit logic_error(std::string const &s) : std::logic_error(s) 
        {
        }
    };

    class domain_error: public std::domain_error, public backtrace {
    public:
        explicit domain_error(std::string const &s) : std::domain_error(s) 
        {
        }
    };

    class length_error: public std::length_error, public backtrace {
    public:
        explicit length_error(std::string const &s) : std::length_error(s) 
        {
        }
    };

    class invalid_argument : public std::invalid_argument, public backtrace {
    public:
        explicit invalid_argument(std::string const &s) : std::invalid_argument(s)
        {
        }
    };
    
    class out_of_range : public std::out_of_range, public backtrace {
    public:
        explicit out_of_range(std::string const &s) : std::out_of_range(s)
        {
        }
    };

    namespace details {
        class trace_manip {
        public:
            trace_manip(backtrace const *tr) :
                tr_(tr)
            {
            }
            std::ostream &write(std::ostream &out) const
            {
                if(tr_)
                    tr_->trace(out);
                return out;
            }
        private:
            backtrace const *tr_;
        };

        inline std::ostream &operator<<(std::ostream &out,details::trace_manip const &t)
        {
            return t.write(out);
        }
    }

    template<typename E>
    details::trace_manip trace(E const &e)
    {
        backtrace const *tr = dynamic_cast<backtrace const *>(&e);
        return details::trace_manip(tr);
    }


} // boost


//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// #include <boost/backtrace.hpp>

#if defined(__linux) || defined(__APPLE__) || defined(__sun)
#define BOOST_HAVE_EXECINFO
#define BOOST_HAVE_DLADDR
#endif

#if defined(__GNUC__)
#define BOOST_HAVE_ABI_CXA_DEMANGLE
#endif

#ifdef BOOST_HAVE_EXECINFO
#include <execinfo.h>
#endif

#ifdef BOOST_HAVE_ABI_CXA_DEMANGLE
#include <cxxabi.h>
#endif

#ifdef BOOST_HAVE_DLADDR
#include <dlfcn.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <ostream>
#include <sstream>
#include <iomanip>

#if defined(BOOST_MSVC)
#define NOMINMAX
#include <windows.h>
#include <stdlib.h>
#include <dbghelp.h>
#endif


namespace boost {

    namespace stack_trace {
        #if defined(BOOST_HAVE_EXECINFO)
        
        int trace(void **array,int n)
        {
            return :: backtrace(array,n);
        }
        
        #elif defined(BOOST_MSVC)

        int trace(void **array,int n)
        {
            if(n>=63)
                n=62;
            return RtlCaptureStackBackTrace(0,n,array,0);
        }

        #else

        int trace(void ** /*array*/,int /*n*/)
        {
            return 0;
        }

        #endif
        
        #if defined(BOOST_HAVE_DLADDR) && defined(BOOST_HAVE_ABI_CXA_DEMANGLE)
        
        std::string get_symbol(void *ptr)
        {
            if(!ptr)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            res << ptr<<": ";
            Dl_info info = {0};
            if(dladdr(ptr,&info) == 0) {
                res << "???";
            }
            else {
                if(info.dli_sname) {
                    int status = 0;
                    char *demangled = abi::__cxa_demangle(info.dli_sname,0,0,&status);
                    if(demangled) {
                        res << demangled;
                        free(demangled);
                    }
                    else {
                        res << info.dli_sname;
                    }
                }
                else {
                    res << "???";
                }

                unsigned offset = (char *)ptr - (char *)info.dli_saddr;
                res << std::hex <<" + 0x" << offset ;

                if(info.dli_fname)
                    res << " in " << info.dli_fname;
            }
           return res.str();
        }

        std::string get_symbols(void *const *addresses,int size)
        {
            std::string res;
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    res+=tmp;
                    res+='\n';
                }
            }
            return res;
        }
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    out << tmp << '\n';
                }
            }
            out << std::flush;
        }

        #elif defined(BOOST_HAVE_EXECINFO)
        std::string get_symbol(void *address)
        {
            char ** ptr = backtrace_symbols(&address,1);
            try {
                if(ptr == 0)
                    return std::string();
                std::string res = ptr[0];
                free(ptr);
                ptr = 0;
                return res;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }
        
        std::string get_symbols(void * const *address,int size)
        {
            char ** ptr = backtrace_symbols(address,size);
            try {
                if(ptr==0)
                    return std::string();
                std::string res;
                for(int i=0;i<size;i++) {
                    res+=ptr[i];
                    res+='\n';
                }
                free(ptr);
                ptr = 0;
                return res;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }

        
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            char ** ptr = backtrace_symbols(addresses,size);
            try {
                if(ptr==0)
                    return;
                for(int i=0;i<size;i++)
                    out << ptr[i] << '\n';
                free(ptr);
                ptr = 0;
                out << std::flush;
            }
            catch(...) {
                free(ptr);
                throw;
            }
        }
        
        #elif defined(BOOST_MSVC)
        
        namespace {
            HANDLE hProcess = 0;
            bool syms_ready = false;
            
            void init()
            {
                if(hProcess == 0) {
                    hProcess = GetCurrentProcess();
                    SymSetOptions(SYMOPT_DEFERRED_LOADS);

                    if (SymInitialize(hProcess, NULL, TRUE))
                    {
                        syms_ready = true;
                    }
                }
            }
        }
        
        std::string get_symbol(void *ptr)
        {
            if(ptr==0)
                return std::string();
            init();
            std::ostringstream ss;
            ss << ptr;
            if(syms_ready) {
                DWORD64  dwDisplacement = 0;
                DWORD64  dwAddress = (DWORD64)ptr;

                std::vector<char> buffer(sizeof(SYMBOL_INFO) + MAX_SYM_NAME);
                PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)&buffer.front();

                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = MAX_SYM_NAME;

                if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
                {
                    ss <<": " << pSymbol->Name << std::hex << " +0x" << dwDisplacement;
                }
                else
                {
                    ss << ": ???";
                }
            }
            return ss.str();
        }

        std::string get_symbols(void *const *addresses,int size)
        {
            std::string res;
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    res+=tmp;
                    res+='\n';
                }
            }
            return res;
        }
        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            for(int i=0;i<size;i++) {
                std::string tmp = get_symbol(addresses[i]);
                if(!tmp.empty()) {
                    out << tmp << '\n';
                }
            }
            out << std::flush;
        }
        
        #else

        std::string get_symbol(void *ptr)
        {
            if(!ptr)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            res << ptr;
            return res.str();
        }

        std::string get_symbols(void *const *ptrs,int size)
        {
            if(!ptrs)
                return std::string();
            std::ostringstream res;
            res.imbue(std::locale::classic());
            write_symbols(ptrs,size,res);
            return res.str();
        }

        void write_symbols(void *const *addresses,int size,std::ostream &out)
        {
            for(int i=0;i<size;i++) {
                if(addresses[i]!=0)
                    out << addresses[i]<<'\n';
            }
            out << std::flush;
        }

        #endif

    } // stack_trace

} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
