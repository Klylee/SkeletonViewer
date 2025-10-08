#ifndef PATH_H
#define PATH_H

#include <string>

std::string getFileName(const std::string &filePath);
std::string getFileExtension(const std::string &filePath);
std::string getDirectory(const std::string &filePath);
std::string join(const std::string &dir, const std::string &fileName);

struct Path
{
    Path(const std::string &p = "") : path(p) {}
    operator std::string() const { return path; }
    Path operator+(const std::string &p) const { return Path(join(path, p)); }

    std::string extension() const { return getFileExtension(path); }
    std::string filename() const { return getFileName(path); }
    Path directory() const { return Path(getDirectory(path)); }

    std::string path;
};

#endif // PATH_H