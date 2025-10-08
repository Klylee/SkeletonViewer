#include "Path.h"

std::string getFileName(const std::string &filePath)
{
  size_t lastSlash = filePath.find_last_of("/\\");
  return filePath.substr(lastSlash + 1);
}

std::string getFileExtension(const std::string &filePath)
{
  size_t dot = filePath.find_last_of(".");
  return filePath.substr(dot + 1);
}

std::string getDirectory(const std::string &filePath)
{
  size_t lastSlash = filePath.find_last_of("/\\");
  return filePath.substr(0, lastSlash + 1);
}

std::string join(const std::string &dir, const std::string &fileName)
{
  if (dir.empty() || dir.back() == '/' || dir.back() == '\\')
  {
    return dir + fileName;
  }
  else
  {
    return dir + "/" + fileName;
  }
}
