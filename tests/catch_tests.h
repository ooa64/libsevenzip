#pragma once

#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define QUITE ">nul 2>&1"
#define mkdir(_d_,_m_) (_mkdir(_d_))
#else
#include <unistd.h>
#define QUITE "> /dev/null 2>&1"
#endif

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "sevenzip.h"

namespace sevenzip_test {

inline int run_cmd(const std::string& cmd) {
    return std::system(cmd.c_str());
}

inline std::string quote(const std::string& path) {
    return "\"" + path + "\"";
}

inline std::string find_7z_executable() {
    const char* env = std::getenv("SEVENZIPBIN");
    if (env && *env) {
        std::string exe = env;
        if (run_cmd(exe + " -h " QUITE) == 0) {
            return exe;
        }
        return {};
    }
    const std::vector<std::string> candidates = {"7z", "7za", "7zz"};
    for (const auto& exe : candidates) {
        if (run_cmd(exe + " -h " QUITE) == 0) {
            return exe;
        }
    }
    return {};
}

inline bool ensure_dir(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::string current;
    size_t start = 0;
    if (!path.empty() && path[0] == '/') {
        current = "/";
        start = 1;
    }
    while (start < path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string::npos) {
            end = path.size();
        }
        std::string part = path.substr(start, end - start);
        if (!part.empty()) {
            if (!current.empty() && current[current.size() - 1] != '/') {
                current += "/";
            }
            current += part;
            if (mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
                return false;
            }
        }
        start = end + 1;
    }
    return true;
}

inline bool path_exists(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

inline bool is_dir(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFDIR) != 0;
}

inline unsigned long long file_size(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<unsigned long long>(st.st_size);
}

inline bool remove_file(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

inline std::string get_base_dir(const std::string& suite) {
    std::string base = std::string("temps") + "/catch2/" + suite;
    ensure_dir(base);
    return base;
}

inline bool write_text_file(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str(), std::ios::binary);
    out << content;
    return out.good();
}

inline std::string create_7z_archive(const std::string& exe, const std::string& base) {
    std::string input = base + "/sample.txt";
    std::string archive = base + "/sample.7z";

    if (!write_text_file(input, "hello libsevenzip")) {
        return {};
    }

    if (path_exists(archive)) {
        remove_file(archive);
    }

    std::string cmd = exe + " a -t7z -y " + quote(archive) + " " + quote(input);
    int rc = run_cmd(cmd + " " QUITE);
    if (rc != 0 || !path_exists(archive)) {
        return std::string();
    }
    return archive;
}

struct FakeIstream : public sevenzip::Istream {
    bool open_ok = true;
    bool seek_ok = true;
    bool read_ok = true;
    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Read(void* /*data*/, UInt32 /*size*/, UInt32& processed) override {
        processed = 0;
        return read_ok ? S_OK : S_FALSE;
    }
    virtual void Close() override {
    }
    virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64& /*position*/) override {
        return seek_ok ? S_OK : S_FALSE;
    }
};

struct FakeOstream : public sevenzip::Ostream {
    bool open_ok = true;
    bool write_ok = true;
    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Write(const void* /*data*/, UInt32 /*size*/, UInt32& processed) override {
        processed = 0;
        return write_ok ? S_OK : S_FALSE;
    }
    virtual void Close() override {
    }
};

struct FileIstream : public sevenzip::Istream {
    std::ifstream stream;
    bool preopened = false;

    bool openPath(const std::string& path) {
        stream.open(path.c_str(), std::ios::binary);
        preopened = stream.is_open();
        return preopened;
    }

    HRESULT Open(const wchar_t* filename) override {
        if (preopened) {
            return S_FALSE;
        }
        stream.open(sevenzip::toBytes(filename), std::ios::binary);
        return sevenzip::getResult(stream.is_open());
    }

    void Close() override {
        stream.close();
    }

    HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        stream.read(reinterpret_cast<char*>(data), size);
        processed = static_cast<UInt32>(stream.gcount());
        return sevenzip::getResult(stream.good() || stream.eof());
    }

    HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        stream.clear();
        stream.seekg(offset, static_cast<std::ios_base::seekdir>(origin));
        position = static_cast<UInt64>(stream.tellg());
        return sevenzip::getResult(stream.good());
    }

    UInt64 GetSize(const wchar_t* filename) override {
        std::string p = sevenzip::toBytes(filename);
        if (!path_exists(p)) {
            return 0;
        }
        return static_cast<UInt64>(file_size(p));
    }

    bool IsDir(const wchar_t* filename) override {
        std::string p = sevenzip::toBytes(filename);
        return is_dir(p);
    }

    Istream* Clone() const override {
        return new FileIstream();
    }
};

struct FileOstream : public sevenzip::Ostream {
    std::fstream stream;
    bool preopened = false;
    std::string path;

    bool openPath(const std::string& path) {
        this->path = path;
        stream.open(path.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        preopened = stream.is_open();
        return preopened;
    }

    HRESULT Open(const wchar_t* filename) override {
        if (preopened) {
            return S_FALSE;
        }
        path = sevenzip::toBytes(filename);
        stream.open(path.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        return sevenzip::getResult(stream.is_open());
    }

    void Close() override {
        stream.close();
    }

    HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        stream.write(reinterpret_cast<const char*>(data), size);
        processed = size;
        return sevenzip::getResult(stream.good());
    }

    HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        stream.clear();
        stream.seekp(offset, static_cast<std::ios_base::seekdir>(origin));
        position = static_cast<UInt64>(stream.tellp());
        return sevenzip::getResult(stream.good());
    }

    HRESULT SetSize(UInt64 size) override {
#ifdef _WIN32
        (void)size; // Unused parameter
        return S_FALSE; // Not implemented on Windows
#else        
        stream.flush();
        if (path.empty()) {
            return S_FALSE;
        }
        int rc = ::truncate(path.c_str(), static_cast<off_t>(size));
        return sevenzip::getResult(rc == 0);
#endif
    }

    Ostream* Clone() const override {
        return new FileOstream();
    }
};

} // namespace sevenzip_test
