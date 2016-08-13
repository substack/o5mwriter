#include <o5mwriter.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
  char *buf = (char *) malloc(4096*4);
  char *strings = (char *) malloc(o5mwriter::STRSIZE);
  o5mwriter::Writer writer(stdout, 4096, buf, strings);
  o5mwriter::Node node(4096, buf+4096*1, strings);
  o5mwriter::Way way(4096, buf+4096*2, strings);
  o5mwriter::Rel rel(4096, buf+4096*3, strings);

  node.id = 1234;
  node.lon = -148.25;
  node.lat = 64.9;
  node.add_tag("hey", "cool");
  writer.write(node);

  node.id = 1235;
  node.lon = -147.75;
  node.lat = 65.2;
  writer.write(node);

  way.id = 1236;
  way.add_ref(1234);
  way.add_ref(1235);
  way.add_tag("one", "two");
  writer.write(way);

  way.id = 1237;
  way.add_ref(1234);
  way.add_ref(1235);
  way.add_tag("three", "four");
  writer.write(way);

  way.id = 1238;
  way.add_ref(3000);
  way.add_ref(3003);
  way.add_tag("five", "six");
  writer.write(way);

  way.id = 1239;
  way.add_ref(1005);
  way.add_ref(1000);
  way.add_tag("beep", "boop");
  writer.write(way);

  rel.id = 1240;
  rel.add_member(1236, o5mwriter::WAY, "");
  rel.add_member(1237, o5mwriter::WAY, "");
  rel.version = 1337;
  rel.timestamp = 1471033291;
  rel.changeset = 87654321;
  rel.uid = 5555;
  rel.user = (char *) "hello";
  writer.write(rel);

  writer.end();
  return 0;
}
