/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Copyright (C) 2025
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include "libsevenzip/exceptions.hpp"

namespace sevenzip {

Exception::Exception(const std::string& message)
    : std::runtime_error(message) {
}

ArchiveOpenException::ArchiveOpenException(const std::string& message)
    : Exception(message) {
}

FormatException::FormatException(const std::string& message)
    : Exception(message) {
}

ExtractionException::ExtractionException(const std::string& message)
    : Exception(message) {
}

CompressionException::CompressionException(const std::string& message)
    : Exception(message) {
}

PasswordException::PasswordException(const std::string& message)
    : Exception(message) {
}

CancelledException::CancelledException(const std::string& message)
    : Exception(message) {
}

} // namespace sevenzip
