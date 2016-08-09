#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

namespace o5mwriter {
  typedef unsigned char TYPE;
  const TYPE NODE = 0x10;
  const TYPE WAY = 0x11;
  const TYPE REL = 0x12;
  size_t xsigned (char *out, int64_t v) {
    uint64_t x = abs(v) - (v < 0 ? 1 : 0);
    uint64_t npow = 1;
    unsigned char r;
    unsigned char m = 0x40;
    size_t i = 0;
    while (x > 0) {
      r = (x / npow) % m;
      x -= r * npow;
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
      r = (x / npow) % 0x80;
      x -= r * npow;
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
    public:
    TYPE type;
    uint64_t id;
    void init (size_t len, char *buf) {
      buffer = buf;
      length = len;
    }
    virtual void reset () {}
    virtual size_t data (char **buf, size_t prev_id) {
      size_t pos = 0;
      pos += xsigned(buffer+pos, id - prev_id);
      buffer[pos++] = 0x00; // version
      *buf = buffer;
      return pos;
    }
    size_t add_tag (char *key, char *value) {
    }
    size_t add_tag (const char *key, const char *value) {
      add_tag((char *) key, (char *) value);
    }
  };
  class Node : public Doc {
    public:
    double lon, lat;
    double prev_lon, prev_lat;
    Node (size_t len, char *buf) {
      init(len, buf);
      type = NODE;
      prev_lon = 0;
      prev_lat = 0;
    }
    size_t data (char **buf, size_t prev_id) {
      size_t pos = Doc::data(buf, prev_id);
      pos += xsigned(buffer+pos, roundl((lon - prev_lon) * 1e7));
      pos += xsigned(buffer+pos, roundl((lat - prev_lat) * 1e7));
      return pos;
    }
    void reset () {
      Doc::reset();
      prev_lon = lon;
      prev_lat = lat;
      lon = 0;
      lat = 0;
    }
  };
  class Way : public Doc {
    char *refbuf;
    size_t refpos, reflen;
    uint64_t prev_ref;
    public:
    Way (size_t len, char *buf) {
      init(len/2, buf);
      refbuf = buf+(len+1)/2;
      reflen = len-(len+1)/2;
      refpos = 0;
      prev_ref = 0;
      type = WAY;
    }
    size_t add_ref (uint64_t ref) {
      refpos += xsigned(refbuf+refpos, ref - prev_ref);
      prev_ref = ref;
    }
    size_t data (char **buf, size_t prev_id) {
      size_t pos = Doc::data(buf, prev_id);
      pos += xunsigned(buffer+pos, refpos);
      memcpy(buffer+pos, refbuf, refpos);
      pos += refpos;
      return pos;
    }
    void reset () {
      Doc::reset();
      refpos = 0;
    }
  };
  class Rel : public Doc {
    public:
    Rel (size_t len, char *buf) {
      init(len, buf);
      type = REL;
    }
    size_t add_member (uint64_t ref, TYPE type, char *role) {
    }
    size_t add_member (uint64_t ref, TYPE type, const char *role) {
      add_member(ref, type, (char *) role);
    }
    size_t data (char **buf, size_t prev_id) {
      size_t pos = 0;
      return pos;
    }
  };
  class Writer {
    uint64_t prev_id;
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
      prev_id = 0;
      buffer[pos++] = 0xff;
    }
    void write (Doc &doc) {
      char *buf;
      size_t size = doc.data(&buf, prev_id);
      prev_id = doc.id;
      size_t n = xunsigned(lenbuf, size);
      if (pos + size + 1 + n > length) flush();
      buffer[pos++] = doc.type;
      memcpy(buffer+pos, lenbuf, n);
      pos += n;
      memcpy(buffer+pos, buf, size);
      pos += size;
      doc.reset();
    }
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
