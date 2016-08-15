#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <assert.h>

namespace o5mwriter {
  typedef unsigned char TYPE;
  const TYPE NODE = 0x10;
  const TYPE WAY = 0x11;
  const TYPE REL = 0x12;
  const size_t STRSIZE = 15000 * 256 + sizeof(size_t);
  size_t xsigned (char *out, int64_t v) {
    uint64_t x = abs(v);
    if (v == 0) x++;
    uint64_t npow = 1;
    unsigned char r;
    unsigned char m = 0x40;
    size_t i = 0;
    while (x > 0) {
      r = (x / npow) % m;
      x -= r * npow;
      if (i == 0) r = r*2 - (v < 0 ? 1 : 0);
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
      npow *= 0x80;
    }
    return i;
  }
  size_t strfind (size_t slen, char *str, char *strings) {
    size_t len = *((size_t *) strings);
    char *data = strings + sizeof(size_t);
    for (size_t i = 0; i < len; i++) {
      if (memcmp(data+i*256, str, slen) == 0) return len - i;
    }
    return 0;
  }
  size_t strpack (char *out, char *strings, char *s) {
    size_t pos = 0;
    size_t len = strlen(s);
    out[pos++] = 0x00;
    memcpy(out+pos, s, len);
    pos += len;
    return pos;
  }
  size_t strpair (char *out, char *strings, char *a, char *b) {
    size_t pos = 0;
    pos += strpack(out+pos, strings, a);
    pos += strpack(out+pos, strings, b);
    out[pos++] = 0x00;

    size_t find = strfind(pos-1, out+1, strings);
    size_t slen = ((size_t *) strings)[0];
    char *ptr = strings + sizeof(size_t) + slen * 256;

    if (find == 0) { // not found, insert record
      ((size_t *) strings)[0]++;
      memcpy(ptr, out+1, pos-1);
      return pos;
    } else { // found in string table
      return xunsigned(out, find);
    }
  }
  class Doc {
    protected:
    char *buffer;
    size_t length;
    char *tagbuf;
    size_t taglen;
    size_t tagpos;
    public:
    TYPE type;
    uint64_t id;
    uint64_t version;
    int64_t timestamp;
    uint64_t changeset;
    uint64_t uid;
    char *user;
    uint64_t prev_timestamp;
    uint64_t prev_changeset;
    char *strings;
    char tmp[8];
    void init (size_t len, char *buf, char *sbuf) {
      buffer = buf;
      length = len/2;
      taglen = (len+1)/2;
      tagbuf = buf+(len-taglen);
      strings = sbuf;
      timestamp = 0;
      changeset = 0;
      reset();
    }
    virtual void reset () {
      prev_timestamp = timestamp;
      prev_changeset = changeset;
      tagpos = 0;
      version = 0;
      timestamp = 0;
      changeset = 0;
      uid = 0;
      user = NULL;
    }
    virtual size_t data (char **buf, size_t prev_id) {
      size_t pos = 0;
      pos += xsigned(buffer+pos, id-prev_id);
      if (version) {
        pos += xunsigned(buffer+pos, version);
        if (timestamp) {
          pos += xsigned(buffer+pos, timestamp-prev_timestamp);
          pos += xsigned(buffer+pos, changeset-prev_changeset);
          tmp[xunsigned(tmp,uid)] = 0x00;
          pos += strpair(buffer+pos, strings, tmp, user);
        } else {
          buffer[pos++] = 0x00;
        }
      } else {
        buffer[pos++] = 0x00;
      }
      return pos;
    }
    size_t tag_data (char *out) {
      if (tagpos == 0) return 0;
      memcpy(out, tagbuf, tagpos);
      return tagpos;
    }
    void add_tag (char *key, char *value) {
      tagpos += strpair(tagbuf+tagpos, strings, key, value);
    }
    void add_tag (const char *key, const char *value) {
      add_tag((char *) key, (char *) value);
    }
  };
  class Node : public Doc {
    public:
    double lon, lat;
    double prev_lon, prev_lat;
    Node (size_t len, char *buf, char *sbuf) {
      init(len, buf, sbuf);
      type = NODE;
      prev_lon = 0;
      prev_lat = 0;
    }
    size_t data (char **buf, size_t prev_id) {
      *buf = buffer;
      size_t pos = Doc::data(buf, prev_id);
      pos += xsigned(buffer+pos, roundl((lon-prev_lon)*1e7));
      pos += xsigned(buffer+pos, roundl((lat-prev_lat)*1e7));
      pos += tag_data(buffer+pos);
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
    int64_t prev_ref;
    public:
    Way (size_t len, char *buf, char *sbuf) {
      init(len/2, buf, sbuf);
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
      size_t pos = 0;
      *buf = buffer;
      pos += Doc::data(buf, prev_id);
      pos += xunsigned(buffer+pos, refpos);
      memcpy(buffer+pos, refbuf, refpos);
      pos += refpos;
      pos += tag_data(buffer+pos);
      return pos;
    }
    void reset () {
      Doc::reset();
      refpos = 0;
    }
  };
  class Rel : public Doc {
    char *membuf;
    char *rolebuf;
    size_t mempos, memlen, rolelen;
    int64_t prev_ref;
    public:
    Rel (size_t len, char *buf, char *sbuf) {
      rolebuf = buf;
      rolelen = 128;
      init(len/2, buf+rolelen, sbuf);
      memlen = (len-rolelen)-(len-rolelen)/2;
      membuf = buf+(len-rolelen)/2;
      mempos = 0;
      prev_ref = 0;
      type = REL;
    }
    void reset () {
      Doc::reset();
      mempos = 0;
    }
    void add_member (uint64_t ref, TYPE type, char *role) {
      mempos += xsigned(membuf+mempos, ref-prev_ref);
      rolebuf[0] = 0x20 + type;
      size_t rlen = strlen(role);
      assert(rlen < rolelen);
      memcpy(rolebuf+1, role, rlen);
      mempos += strpack(membuf+mempos, strings, rolebuf);
      membuf[mempos++] = 0x00;
      prev_ref = ref;
    }
    void add_member (uint64_t ref, TYPE type, const char *role) {
      add_member(ref, type, (char *) role);
    }
    size_t data (char **buf, size_t prev_id) {
      *buf = buffer;
      size_t pos = Doc::data(buf, prev_id);
      pos += xunsigned(buffer+pos, mempos);
      memcpy(buffer+pos, membuf, mempos);
      pos += mempos;
      pos += tag_data(buffer+pos);
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
    char *strings;
    size_t strpos;
    public:
    Writer (FILE *fh, size_t len, char *buf, char *sbuf) {
      handle = fh;
      buffer = buf;
      strings = sbuf;
      strpos = 0;
      length = len;
      pos = 0;
      prev_id = 0;
      buffer[pos++] = 0xff;
      ((size_t *) sbuf)[0] = 0;
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
