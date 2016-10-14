#include "mime_types.hpp"

struct Mapping {
  const char* ext;
  const char* mime_type;
};

// TODO

const Mapping mappings[] = {
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" },
  { 0, 0 },
};

std::string ExtToMimeType(const std::string& ext) {
  for (const Mapping* m = mappings; m->ext; ++m) {
    if (m->ext == ext) {
      return m->mime_type;
    }
  }

  return "text/plain";
}
