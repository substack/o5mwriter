# o5mwriter

c++ o5m encoder

# example

``` c++
#include <o5mwriter.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
  char *buf = (char *) malloc(4096*4);
  o5mwriter::Writer writer(stdout, 4096, buf);
  o5mwriter::Node node(4096, buf+4096*1);
  o5mwriter::Rel rel(4096, buf+4096*2);
  o5mwriter::Way way(4096, buf+4096*3);

  node.id = 1234;
  node.lon = -148.1;
  node.lat = 64.9;
  node.add_tag("hey", "cool");
  writer.write(node);

  node.id = 1235;
  node.lon = -147.9;
  node.lat = 65.2;
  writer.write(node);

  way.id = 1236;
  way.add_ref(1234);
  way.add_ref(1235);
  way.add_tag("beep", "boop");
  writer.write(way);

  rel.id = 1237;
  rel.add_member(1236, o5mwriter::WAY, "");
  writer.write(rel);

  writer.end();
  return 0;
}
```

# api

``` c++
#include <o5mwriter.h>
```

## `o5mwriter::Writer writer(FILE *fh, size_t size, char *buf)`

Create a new `Writer` instance from a writable file handle `fh` and a `buf`
temporary buffer of length `size`.

## `writer.write(Doc doc)`

Write a `Doc` to the output buffer.

The file handle is flushed automatically as the output buffer fills up.

## `writer.end()`

Flush any remaining output and close the file handle.

## `o5mwriter::Doc doc`

Base-class of `Node`, `Way`, and `Rel` classes.

## `uint64_t doc.id`

Set this property to assign an `id`.

## `doc.add_tag(char *key, char *value)`

Add a tag as a `key, value` pair.

## `o5mwriter::Node node(size_t size, char *buf)`

Create a new `Node` instance from a working buffer `buf` of length `size`.

`Node` instances are subclasses of `Doc`.

## `double node.lon`

Set this property to assign a longitude in decimal degrees.

## `double node.lat`

Set this property to assign a latitude in decimal degrees.

## `o5mwriter::Way way(size_t size, char *buf)`

Create a new `Way` instance from a working buffer `buf` of length `size`.

`Way` instances are subclasses of `Doc`.

## `way.add_ref(uint64_t id)`

Associate a ref by its `id` with this way.

## `o5mwriter::Rel(size_t size, char *buf)`

Create a new `Rel` instance from a working buffer `buf` of length `size`.

`Rel` instances are subclasses of `Doc`.

## `rel.add_member(uint64_t id, o5mwriter::TYPE type, char *role)`

Add a member to a relation by its `id`, `type`, and `role` string.

## `unsigned char o5mwriter::TYPE`

Distinguish documents by their types.

These values are defined for convenience:

* `o5mwriter::NODE` - `0x10`
* `o5mwriter::WAY` - `0x11`
* `o5mwriter::REL` - `0x12`

# license

BSD
