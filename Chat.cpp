// Chat.cpp
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 
#include "Chat.h"
#include "Message.h"
#include "Graph.h"
#include "AutocompleteRU.h"
#include "DictionaryRU.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <limits>
#include <codecvt>
#include <locale>
#include <fstream>
#include <sstream>

using namespace std;

namespace {
    inline wstring utf8_to_wide_local(const string& s) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(s);
    }
    inline string wide_to_utf8_local(const wstring& ws) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(ws);
    }
}

bool Chat::log(string _login, string _pass) {
    auto it = Users.find(_login);
    if (it != Users.end() && it->second.prov(_pass)) {
        if (!it->second.name.empty()) {
            wcout << L"Пользователь " << utf8_to_wide_local(it->second.name) << L" вошёл в чат." << endl;
        }
        else {
            wcout << L"Пользователь " << utf8_to_wide_local(_login) << L" вошёл в чат." << endl;
        }
        return true;
    }
    wcout << L"Ошибка: неверный логин или пароль." << endl;
    return false;
}

bool Chat::reg(const string& _name, const string& _login, const string& _pass) {
    if (Users.find(_login) != Users.end()) {
        wcout << L"Ошибка: логин уже занят." << endl;
        return false;
    }
    uint* hashed_pass = sha1(_pass, static_cast<uint>(_pass.size()));
    if (!hashed_pass) {
        wcout << L"Ошибка: не удалось захешировать пароль." << endl;
        return false;
    }
    User person(_name, _login, hashed_pass);
    Users.emplace(_login, person);
    wcout << L"Пользователь " << utf8_to_wide_local(_name) << L" успешно зарегистрирован!\n" << endl;

    friends->vname.push_back(_login);
    friends->addVertex(static_cast<int>(friends->vname.size() - 1));

    delete[] hashed_pass;
    return true;
}

void Chat::insertRUlib(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        // Заглушка: если словарь не найден, просто ничего не делаем
        return;
    }
    dictRU->loadFromFile(filename);
}

string Chat::T9RU() {
    return wide_to_utf8_local(readInputWithAutocompleteRU(*dictRU));
}

void Chat::logoutUser(const string& login) {
    wcout << L"Пользователь " << utf8_to_wide_local(login) << L" вышел из чата." << endl;
}

void Chat::sendMessage(const string& senderLogin, const string& message, const string& recipient) {
    if (!recipient.empty()) {
        if (Users.find(recipient) == Users.end()) {
            wcout << L"Ошибка: пользователь " << utf8_to_wide_local(recipient) << L" не найден." << endl;
            return;
        }
        privateMessages[senderLogin].emplace_back(senderLogin, message, recipient);
        privateMessages[recipient].emplace_back(senderLogin, message, recipient);
        wcout << utf8_to_wide_local(senderLogin)
            << L" (лично " << utf8_to_wide_local(recipient)
            << L"): " << utf8_to_wide_local(message) << endl;
    }
    else {
        publicMessages.emplace_back(senderLogin, message);
        wcout << utf8_to_wide_local(senderLogin)
            << L" (всем): " << utf8_to_wide_local(message) << endl;
    }
}

void Chat::listUsers(string login) const {
    friends->findMinDistancesFloyd(login);
}

void Chat::viewMessages(const string& login) const {
    wcout << L"Сообщения для " << utf8_to_wide_local(login) << L":" << endl;

    for (const auto& msg : publicMessages) {
        wcout << utf8_to_wide_local(msg._sender)
            << L" (всем): " << utf8_to_wide_local(msg._content) << endl;
    }

    if (privateMessages.count(login)) {
        for (const auto& msg : privateMessages.at(login)) {
            wcout << utf8_to_wide_local(msg._sender)
                << L" (лично " << utf8_to_wide_local(msg._recip) << L"): "
                << utf8_to_wide_local(msg._content) << endl;
        }
    }
}

void Chat::addFriend(const string& user_name) {
    wcout << L"Кого вы хотите добавить в друзья? Для отмены введите 0." << endl;
    listUsers(user_name);
    string friend_name;
    while (true) {
        wcout << L"Введите логин: ";
        cin >> friend_name;
        if (friend_name == "0") break;
        if (Users.find(friend_name) == Users.end()) {
            wcout << L"Ошибка: пользователь не найден." << endl;
            continue;
        }
        if (friend_name == user_name) {
            wcout << L"Нельзя добавить себя." << endl;
            continue;
        }
        friends->addEdge(user_name, friend_name);
        wcout << L"Добавлен в друзья: " << utf8_to_wide_local(friend_name) << endl;
        break;
    }
}

string Chat::T9() {
    auto wsuggestions = trie->autocomplete(L"");

    if (wsuggestions.empty()) {
        wcout << L"Словарь пуст. Введите своё слово: ";
        string input;
        do {
            getline(cin >> ws, input);
        } while (input.empty());
        trie->insert(utf8_to_wstring(input));
        return input;
    }

    const size_t kMaxShow = 5;
    size_t shown = min(kMaxShow, wsuggestions.size());

    wcout << L"\nВведите префикс. Ниже вы видите варианты:\n";

    for (size_t i = 0; i < shown; ++i) {
        wcout << (i + 1) << L" - " << wsuggestions[i] << L"  ";
    }

    wcout << L"\nЗатем введите цифру между 1 и " << shown
        << L" и нажмите Enter (0 — ввести своё): ";

    string choiceLine;
    getline(cin >> ws, choiceLine);

    if (choiceLine.empty()) {
        return wstring_to_utf8(wsuggestions[0]);
    }

    int choice = -1;
    try {
        choice = stoi(choiceLine);
    }
    catch (...) {
        wcout << L"Введите своё слово: ";
        string input;
        getline(cin >> ws, input);
        if (!input.empty()) trie->insert(utf8_to_wstring(input));
        return input;
    }

    if (choice == 0) {
        wcout << L"Введите своё слово: ";
        string input;
        do {
            getline(cin >> ws, input);
        } while (input.empty());
        trie->insert(utf8_to_wstring(input));
        return input;
    }

    if (choice >= 1 && static_cast<size_t>(choice) <= shown) {
        return wstring_to_utf8(wsuggestions[static_cast<size_t>(choice) - 1]);
    }

    wcout << L"Некорректный выбор. Введите своё слово: ";
    string input;
    do {
        getline(cin >> ws, input);
    } while (input.empty());
    trie->insert(utf8_to_wstring(input));
    return input;
}

void Chat::insert_lib() {
    trie->insert(utf8_to_wstring("Hello"));
    trie->insert(utf8_to_wstring("How"));
    trie->insert(utf8_to_wstring("are"));
    trie->insert(utf8_to_wstring("you"));
    trie->insert(utf8_to_wstring("hi"));
}

void Chat::saveUsersToFile(const string& filename) const {
    ofstream file(filename, ios::trunc);
    if (!file.is_open()) return;

    for (const auto& [login, user] : Users) {
        file << login << ";" << user.name << ";";
        const auto& h = user.get_hash();
        for (int i = 0; i < 5; i++) {
            file << h[i];
            if (i < 4) file << ",";
        }
        file << "\n";
    }
}

void Chat::loadUsersFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string login, name, hash_str;
        if (getline(ss, login, ';') && getline(ss, name, ';') && getline(ss, hash_str)) {
            vector<uint> hash;
            string num;
            stringstream hs(hash_str);
            while (getline(hs, num, ',')) {
                hash.push_back(stoul(num));
            }
            if (hash.size() == 5) {
                Users.insert_or_assign(login, User(name, login, hash.data()));
            }
        }
    }
}

void Chat::saveMessagesToFile(const string& filename) const {
    ofstream file(filename, ios::trunc);
    if (!file.is_open()) return;

    for (const auto& msg : publicMessages) {
        file << msg._sender << ";" << msg._recip << ";" << msg._content << "\n";
    }
    for (const auto& [login, msgs] : privateMessages) {
        for (const auto& msg : msgs) {
            file << msg._sender << ";" << msg._recip << ";" << msg._content << "\n";
        }
    }
}

void Chat::loadMessagesFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.size() >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF)
        {
            line.erase(0, 3);
        }

        stringstream ss(line);
        string sender, recip, text;
        if (getline(ss, sender, ';') && getline(ss, recip, ';') && getline(ss, text)) {
            if (recip.empty()) {
                publicMessages.emplace_back(sender, text);
            }
            else {
                privateMessages[recip].emplace_back(sender, text, recip);
            }
        }
    }
}
