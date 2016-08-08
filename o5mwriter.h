#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

namespace o5mwriter {
  typedef unsigned char TYPE;
  const TYPE NODE = 0x10;
  const TYPE WAY = 0x11;
  const TYPE REL = 0x12;
  size_t xsigned (char *out, int64_t v) {
    uint64_t x = abs(v);
    uint64_t npow = 1;
    unsigned char r;
    unsigned char m = 0x40;
    size_t i = 0;
    while (x > 0) {
      r = x % m;
      x = (x - r) / (r * npow);
      if (i == 0) r = r*2 + (v > 0 ? 0 : 1);
      if (x > 0) r += 0x80;
      out[i++] = r;
      npow *= m;
      m = 0x80;
    }
    return i;
  }
  size_t xunsigned (char *out, uint64_t x) {
    unsigned char r;
    size_t i = 0;
    uint64_t npow = 1;
    while (x > 0) {
      r = x % 0x80;
      x = (x - r) / (r * npow);
      if (x > 0) r += 0x80;
      out[i++] = r;
    }
    return i;
  }
  class Doc {
    protected:
    char *buffer;
    size_t length;
    size_t pos;
    uint64_t prev_id;
    public:
    uint64_t id;
    void init (size_t len, char *buf) {
      buffer = buf;
      length = len;
    }
    virtual void reset () {
      prev_id = id;
    }
    virtual size_t data (char **buf) = 0;
    size_t add_tag (char *key, char *value) {
    }
    size_t add_tag (const char *key, const char *value) {
      add_tag((char *) key, (char *) value);
    }
  };
  class Node : public Doc {
    public:
    double lon;
    double lat;
    Node (size_t len, char *buf) {
      init(len, buf);
    }
    size_t data (char **buf) {
      size_t pos = 0;
      pos += xsigned(buffer+pos, id - prev_id);
      *buf = buffer;
      return pos;
    }
    void reset () {
      lon = 0;
      lat = 0;
    }
  };
  class Way : public Doc {
    public:
    Way (size_t len, char *buf) {
      init(len, buf);
    }
    size_t add_ref (uint64_t ref) {
    }
    size_t data (char **buf) {
      size_t pos = 0;
      return pos;
    }
  };
  class Rel : public Doc {
    public:
    Rel (size_t len, char *buf) {
      init(len, buf);
    }
    size_t add_member (uint64_t ref, TYPE type, char *role) {
    }
    size_t add_member (uint64_t ref, TYPE type, const char *role) {
      add_member(ref, type, (char *) role);
    }
    size_t data (char **buf) {
      size_t pos = 0;
      return pos;
    }
  };
  class Writer {
    char *buffer;
    size_t length;
    size_t pos;
    char lenbuf[8];
    FILE *handle;
    public:
    Writer (FILE *fh, size_t len, char *buf) {
      handle = fh;
      buffer = buf;
      length = len;
      pos = 0;
      buffer[pos++] = 0xff;
    }
    void write (TYPE type, Doc &doc) {
      char *buf;
      size_t size = doc.data(&buf);
      size_t n = xunsigned(lenbuf, size);
      if (pos + size + 1 + n > length) flush();
      buffer[pos++] = type;
      memcpy(buffer+pos, lenbuf, n);
      pos += n;
      memcpy(buffer+pos, buf, size);
      pos += size;
      doc.reset();
    }
    void write (Node node) { write(NODE, node); }
    void write (Way way) { write(WAY, way); }
    void write (Rel rel) { write(REL, rel); }
    void end () {
      if (pos + 1 > length) flush();
      buffer[pos++] = 0xfe;
      flush();
    }
    void flush () {
      fwrite(buffer, 1, pos, handle);
      pos = 0;
    }
  };
}
