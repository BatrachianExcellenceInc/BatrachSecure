// vim: ts=4 sw=4 fenc=utf-8
#pragma once

#include <QString>
#include <exception>

namespace BtxSecurity {
    class BtxException : public std::exception {
        protected:
            QString msg;
        public:
            BtxException(QString msg);
            virtual const char* what() const throw();
            ~BtxException() throw();
    };
}

