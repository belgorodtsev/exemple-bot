#include "api.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "utils.hpp"

using namespace std;
using namespace TgBot;
using namespace Utils;
using value_type = unsigned short;
bool WAITING_MESSAGE_ENCRYPT = false;
bool WAITING_MESSAGE_DECRYPT = false;
bool WAITING_KEY_ENCRYPT = false;
bool WAITING_KEY_DECRYPT = false;
int KEY_ENCRYPT;
int KEY_DECRYPT;

void onCommandStart(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Hi!");
}

void onCommandEnd(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Bye!");
}

void onCommandEncrypt(Bot& bot, Message::Ptr message)
{   
    bot.getApi().sendMessage(message->chat->id, "Enter the message for encryption:");
    WAITING_MESSAGE_ENCRYPT = true;
}

void onCommandDecrypt(Bot& bot, Message::Ptr message)
{  
    bot.getApi().sendMessage(message->chat->id, "Enter the message to decrypt:");
    WAITING_MESSAGE_DECRYPT = true;
}

void onCommandKeyEnc(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Enter the key:");
    WAITING_KEY_ENCRYPT = true;
}

void onCommandKeyDec(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Enter the key:");
    WAITING_KEY_DECRYPT = true;
}

std::string hexString(unsigned char ch)
{
    std::string hexStr;
    std::stringstream sstream;
    sstream << std::setfill('0') << std::setw(2)
        << std::hex << std::uppercase << (int)ch;
    hexStr = sstream.str();
    sstream.clear();
    return hexStr;
}

std::string unHexString(const std::string& mess)
{
    std::string UnHexString;
    value_type dec;
    std::string tmp;
    for (int i = 0; i < mess.size(); i = i + 2)
    {
        tmp.push_back(mess[i]);
        tmp.push_back(mess[i + 1]);
        std::istringstream(tmp) >> std::hex >> dec;
        UnHexString.push_back(dec);
        tmp.clear();
    }
    return UnHexString;
}
std::string encryption(const std::string &message)
{
    value_type gamma = 0;
    value_type mess = 0;
    value_type cipher = 0;
    std::string encryptMes;
    for (int i = 0; i < message.size(); i = i + 2)
    {
        value_type mess = (message[i] << 8) | message[i + 1];
        gamma = rand();
        cipher = mess ^ gamma;
        mess = cipher;
        mess >>= 15;
        cipher <<= 1;
        cipher |= mess;
        unsigned char firstS = (cipher >> 8) & 0xFF;
        std::string hexOneb = hexString(firstS);
        encryptMes += hexOneb;
        unsigned char secondS = cipher & 0xFF;
        std::string hexTwob = hexString(secondS);
        encryptMes += hexTwob;
    }
    return encryptMes;
}
void encrypt(Bot& bot, Message::Ptr message)
{
    srand(KEY_ENCRYPT);
    std::string enc = encryption(message->text);
    bot.getApi().sendMessage(message->chat->id, enc);
}

std::string decryption(const std::string &message)
{
    value_type gamma = 0;
    value_type mess = 0;
    value_type cipher = 0;
    std::string messageCh;
    std::string m = unHexString(message);
    for (int i = 0; i < m.size(); i = i + 2)
    {
        value_type first = (m[i] << 8) | m[i];
        value_type second = m[i + 1] & 0xFF;
        value_type mess = (first << 8) | second;
        gamma = rand();
        cipher = mess;
        mess <<= 15;
        cipher >>= 1;
        cipher |= mess;
        cipher = cipher ^ gamma;
        unsigned char firstS = (cipher >> 8) & 0xFF;
        messageCh += firstS;
        unsigned char secondS = cipher & 0xFF;
        messageCh += secondS;
    }
    return messageCh;
}
void decrypt(Bot& bot, Message::Ptr message)
{  
    try 
    {
        srand(KEY_DECRYPT);
        std::string dec = decryption(message->text);
        bot.getApi().sendMessage(message->chat->id, dec);
    }
    catch (...) 
    {
        bot.getApi().sendMessage(message->chat->id, "Decryption error!");
    }    
}

bool chec_KEY(Bot&bot, Message::Ptr message)
{
    if (StringTools::startsWith(message->text, "/start")||
        StringTools::startsWith(message->text, "/end") ||
        StringTools::startsWith(message->text, "/encrypt") ||
        StringTools::startsWith(message->text, "/decrypt"))
        return true;
    else
        return false;
}
bool chec_MESSAGE_ENCRYPT(Bot&bot, Message::Ptr message)
{
    if (StringTools::startsWith(message->text, "/start") ||
        StringTools::startsWith(message->text, "/end") ||
        StringTools::startsWith(message->text, "/keyencrypt") ||
        StringTools::startsWith(message->text, "/keydecrypt") ||
        StringTools::startsWith(message->text, "/decrypt"))
        return true;
    else 
        return false;
}
bool chec_MESSAGE_DECRYPT(Bot&bot, Message::Ptr message)
{
    if (StringTools::startsWith(message->text, "/start") ||
        StringTools::startsWith(message->text, "/keyencrypt") ||
        StringTools::startsWith(message->text, "/keydecrypt") ||
        StringTools::startsWith(message->text, "/end") ||
        StringTools::startsWith(message->text, "/encrypt"))
        return true;
    else
        return false;
}

void onAnyMessage(Bot&bot, Message::Ptr message)
{   
    if (WAITING_KEY_ENCRYPT)
    {
        if (chec_KEY(bot, message))
            return;
        KEY_ENCRYPT = atoi(message->text.c_str());
        WAITING_KEY_ENCRYPT = false;
    }
    if (WAITING_KEY_DECRYPT)
    {
        if (chec_KEY(bot, message))
            return;
        KEY_DECRYPT = atoi(message->text.c_str());
        WAITING_KEY_DECRYPT = false;
    }
    if (WAITING_MESSAGE_ENCRYPT)
    {
        if (chec_MESSAGE_ENCRYPT(bot, message))
            return;
        encrypt(bot, message);
        WAITING_MESSAGE_ENCRYPT = false;
    }  
    if (WAITING_MESSAGE_DECRYPT)
    {
        if (chec_MESSAGE_DECRYPT(bot, message))
            return;
        decrypt(bot, message);
        WAITING_MESSAGE_DECRYPT = false;
    }
}

std::map<std::string, std::function<void(Bot&, Message::Ptr)>> getAllCommands()
{
    std::map<std::string, std::function<void(Bot&, Message::Ptr)>> commands =
    {
        { "start", onCommandStart },
        { "end", onCommandEnd },
        { "encrypt", onCommandEncrypt },
        { "decrypt", onCommandDecrypt },
        { "keyencrypt", onCommandKeyEnc },
        { "keydecrypt", onCommandKeyDec }
    };
    return commands;
}
