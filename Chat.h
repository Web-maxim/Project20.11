// Chat.h
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>

#include "Message.h"
#include "Trie.h"
#include "Graph.h"
#include "User.h"
#include "AutocompleteRU.h"
#include "DictionaryRU.h"
#include "sha1.h"

using namespace std;

class Chat
{
private:
    unordered_map<string, User> Users;         // пользователи
    vector<Mess> publicMessages;               // общие сообщения
    map<string, vector<Mess>> privateMessages; // личные сообщения

public:
    // словари автодополнения
    unique_ptr<DictionaryRU> dictRU = make_unique<DictionaryRU>();
    string T9RU();
    void insertRUlib(const string& filename);

    unique_ptr<Graph> friends = make_unique<Graph>();
    unique_ptr<Trie> trie = make_unique<Trie>();

    // регистрация и вход
    bool reg(const string& _name, const string& _login, const string& _pass);
    bool log(string _login, string _pass);
    void logoutUser(const string& login);

    // работа с сообщениями
    void sendMessage(const string& senderLogin, const string& message, const string& recipient = "");
    void listUsers(string login) const;
    void viewMessages(const string& login) const; // 🔹 теперь вывод через wcout

    // друзья
    void addFriend(const string& user_name);

    // автодополнение
    string T9();
    void insert_lib();

    // работа с файлами
    void saveUsersToFile(const string& filename) const;
    void loadUsersFromFile(const string& filename);
    void saveMessagesToFile(const string& filename) const;
    void loadMessagesFromFile(const string& filename);

    size_t getUserCount() const { return Users.size(); }
};
