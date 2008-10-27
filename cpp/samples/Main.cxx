#include <iostream>
#include "XmlParser.hxx"
#include "XmlScanner.hxx"
#include "RSSData.hxx"

using namespace cocoxml;

int main(int argc, char **argv)
{
    XmlScanner *scanner = new XmlScanner(coco_string_create(argv[1]));
    XmlParser *parser = new XmlParser(scanner);
    
    parser->Parse();
    std::cout << parser->rss << std::endl;
}
