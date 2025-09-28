// Message.h
#pragma once
// Изменено: добавлен using namespace std для единообразия.
#include <string>
using namespace std;

class Mess {
public:
    string _sender;
    string _recip;
    string _content;

    Mess(const string& sender, const string& content, const string& recip = "")
        : _sender(sender), _content(content), _recip(recip) {
    }

};