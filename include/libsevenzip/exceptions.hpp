/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Copyright (C) 2025
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#ifndef LIBSEVENZIP_EXCEPTIONS_HPP
#define LIBSEVENZIP_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace sevenzip {

/**
 * Base exception class for all libsevenzip errors
 */
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message);
    virtual ~Exception() noexcept = default;
};

/**
 * Exception thrown when archive file cannot be opened
 */
class ArchiveOpenException : public Exception {
public:
    explicit ArchiveOpenException(const std::string& message);
};

/**
 * Exception thrown when archive format is not supported or cannot be detected
 */
class FormatException : public Exception {
public:
    explicit FormatException(const std::string& message);
};

/**
 * Exception thrown during extraction operations
 */
class ExtractionException : public Exception {
public:
    explicit ExtractionException(const std::string& message);
};

/**
 * Exception thrown during compression operations
 */
class CompressionException : public Exception {
public:
    explicit CompressionException(const std::string& message);
};

/**
 * Exception thrown when password is required but not provided or incorrect
 */
class PasswordException : public Exception {
public:
    explicit PasswordException(const std::string& message);
};

/**
 * Exception thrown when operation is cancelled by user
 */
class CancelledException : public Exception {
public:
    explicit CancelledException(const std::string& message);
};

} // namespace sevenzip

#endif // LIBSEVENZIP_EXCEPTIONS_HPP
