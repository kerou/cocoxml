#include <iostream>
#include "XmlParser.h"
#include "XmlScanner.h"
#include "RSSData.hxx"

int main(int argc, char **argv)
{
    XmlScanner scanner = new XmlScanner(filename);                                                                   
    XmlParser parser = new XmlParser(scanner);
    
    parser.Parse();
    std::cout << parser.rss << std::endl;
}
