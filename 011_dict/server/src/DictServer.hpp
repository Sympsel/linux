#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
using namespace sym;
using std::string;

class DictServer {
   private:
    string split = "=";

   public:
    DictServer(const string& dictFile = "") : _dictFile(dictFile) {
        if (dictFile.empty() || !std::filesystem::exists(dictFile)) {
            _dictFile = std::filesystem::weakly_canonical(
                std::filesystem::current_path() / "resource" / "Dictionary.ini");
        } else {
            _dictFile = dictFile;
        }
        Load();
    }

    void Load() {
        std::ifstream ifs{_dictFile};
        std::cout << _dictFile << std::endl;
        if (ifs.fail()) {
            LOG(log_level_t::FATAL) << "no such dictfile";
            exit(1);
        }
        string line;
        while (getline(ifs, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t pos = line.find(split);
            if (pos == string::npos) {
                LOG(log_level_t::ERROR) << "error format";
                continue;
            }
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            _dict.emplace(key, value);
        }
        LOG(log_level_t::INFO) << "load dictfile successfully";
    }

    ~DictServer() {}

    string Translate(const string& word) {
        string result;
        if (_dict.count(word)) {
            result = _dict[word];
        }
        if (result.empty()) {
            LOG(log_level_t::WARNING) << "Unknown word. "
                                      << "We notice it and may update dictionary library! "
                                      << "Now we handle it with origin word.";
            return word;
        }
        return result;
    }

   private:
    std::unordered_map<string, string> _dict;
    string _dictFile;
};