```c++
#include "json/json.h"

int main() {
    Json::Value root;

    Json::FastWriter writer;
    str_to_fill = writer.write(root);

    // str_to_fill = root.toStyledString();

    // Json::StreamWriterBuilder builder;
    // std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    // std::stringstream ss;
    // writer->write(root, &ss);
    // str_to_fill = ss.str();
    return 0;
}
```