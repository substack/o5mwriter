#include <o5mwriter.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
  o5mwriter::Writer writer(stdout, (char *) malloc(4096));
  o5mwriter::Node node((char *) malloc(4096));
  o5mwriter::Rel rel((char *) malloc(4096));
  o5mwriter::Way way((char *) malloc(4096));

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
  writer.write(rel0;

  writer.end();
  return 0;
}
