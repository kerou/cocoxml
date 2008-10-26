#include <iostream>

int main(int argc, char **argv)
{
    XmlScanner scanner = new XmlScanner(filename);                                                                   
    XmlParser parser = new XmlParser(scanner);
    
    parser.Parse();
    std::cout << parser.rss << std::endl;
}
