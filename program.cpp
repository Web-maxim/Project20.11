// program.cpp
#include <iostream>
#include <sstream>
#include <string>
#include "Message.h"
#include "Chat.h"
#include "Graph.h"
#include "program.h"
#include "Trie.h"

using namespace std;

program::program(const map<string, string>& cfg)
    : config(cfg) {
}

void program::prog()
{
    Chat chat;
    chat.insert_lib();
    chat.insertRUlib(config.at("dictionary"));

    // 🔹 загружаем состояние
    if (config.count("users_file")) {
        chat.loadUsersFromFile(config.at("users_file"));
    }
    if (config.count("messages_file")) {
        chat.loadMessagesFromFile(config.at("messages_file"));
    }

    // 🔹 сразу считаем зарегистрированных
    int registeredUsers = static_cast<int>(chat.getUserCount());

    cout << "Добро пожаловать в наш чат!" << endl;

    int exitFlag = 0;

    do {
        cout << "Нажмите 1 для регистрации" << endl;
        if (registeredUsers > 0)
            cout << "Нажмите 2 для входа" << endl;
        cout << "Для выхода нажмите 0" << endl;

        int choice;
        cin >> choice;

        switch (choice) {
        case 1: {
            string name, login, password;
            cout << "Введите имя: ";       cin >> name;
            cout << "Введите логин: ";     cin >> login;
            cout << "Введите пароль: ";    cin >> password;
            if (chat.reg(name, login, password)) {
                ++registeredUsers;
                chat.saveUsersToFile(config.at("users_file"));
            }
            break;
        }
        case 2: {
            string login, password;
            cout << "Введите логин: ";  cin >> login;
            cout << "Введите пароль: "; cin >> password;

            if (chat.log(login, password)) {
                int option;
                do {
                    cout << endl
                        << "Выберите:" << endl
                        << "1 - отправить сообщение" << endl
                        << "2 - просмотреть сообщения" << endl
                        << "3 - добавить друга" << endl
                        << "4 - выйти из аккаунта" << endl;
                    cin >> option;

                    switch (option) {
                    case 1: {
                        string message, recipient;
                        cout << "Введите сообщение: ";
                        message = chat.T9();

                        int scope = 2;
                        if (registeredUsers > 1) {
                            cout << "Кому отправить сообщение:" << endl;
                            cout << "1 - личное, 2 - всем" << endl;
                            cin >> scope;
                        }

                        if (scope == 2) {
                            chat.sendMessage(login, message);
                        }
                        else if (scope == 1) {
                            chat.listUsers(login);
                            cout << "Введите логин получателя: ";
                            cin >> recipient;
                            chat.sendMessage(login, message, recipient);
                        }
                        else {
                            cout << "Ошибка, попробуйте ещё раз" << endl;
                        }

                        chat.saveMessagesToFile(config.at("messages_file"));
                        break;
                    }
                    case 2:
                        chat.viewMessages(login);
                        break;
                    case 3:
                        chat.addFriend(login);
                        break;
                    case 4:
                        chat.logoutUser(login);
                        break;
                    default:
                        cout << "Неверный выбор! Попробуйте ещё раз." << endl;
                    }
                } while (option != 4);
            }
            break;
        }
        case 0:
            exitFlag = 1;
            break;
        default:
            cout << "Неверный выбор! Попробуйте ещё раз." << endl;
        }
    } while (exitFlag == 0);

    if (config.count("users_file")) {
        chat.saveUsersToFile(config.at("users_file"));
    }
    if (config.count("messages_file")) {
        chat.saveMessagesToFile(config.at("messages_file"));
    }


    cout << "До свидания!" << endl;
}
