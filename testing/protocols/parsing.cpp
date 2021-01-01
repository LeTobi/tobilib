#include "../../protocols/parser/ph2rfp.h"

using namespace tobilib;

h2rfp::Message testData1()
{
    h2rfp::Message out;
    out.name = "myEvent";
    out.id = 12;
    out.data.put("origin","myEvent");
    out.data.put("parameter",5);
    return out;
}

h2rfp::Message testData2()
{
    h2rfp::Message out;
    out.name = "otherEvent";
    out.id = 13;
    out.data.put("origin","unknown");
    h2rfp::JSObject list;
    for (int i=0;i<5;i++)
    {
        h2rfp::JSObject item;
        item.put_value(i);
        list.push_back(std::make_pair("",item));
    }
    out.data.put_child("somelist",list);
    return out;
}

int main()
{
    std::string raw = testData1().to_string() + testData2().to_string() + testData1().to_string();
    std::cout << "raw: " << raw << std::endl;
    std::cout << "try parse..." << std::endl;
    h2rfp::detail::Parser parser;
    parser.feed(raw);
    std::cout << "results: " << std::endl;
    while (!parser.output.empty())
        std::cout << "  " << parser.output.next().to_string() << std::endl;
    return 0;
}