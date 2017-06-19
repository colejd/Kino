#pragma once

#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>


#include "json/json.h"
#include <fstream>

using namespace std;

/**
 Manages the config file for the program.
 THIS IS NOT THREAD SAFE IN C++03 -- USE C++11!
 */
class ConfigHandler {
protected:
    //==== SINGLETON STUFF ==============================================//
    static ConfigHandler& GetInstance()
    {
        static ConfigHandler instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    //==== END SINGLETON STUFF ==============================================//

	template<typename Out>
	static void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}


	static std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	Json::Value root;
    
public:
    
    /**
     Convenience function for finding a value in the config. If you want to access a
	 nested value in the config file, use a period as a delimiter in the key. For example:

	 "a" : { "b": 0 }

	 To get "b", you'd use the key "a.b".
     */
	static Json::Value GetValue(const std::string &key, const Json::Value &defaultValue) {

		std::vector<std::string> tokens = split(key, '.');

		// If there is no nesting, we're already at the root. Use the key as the actual key
		if (tokens.size() == 1) {
			return GetInstance().root.get(key, defaultValue);
		}

		// Step through each '.' separated name until we get to the variable we want.
		Json::Value currentRoot = GetInstance().root[tokens[0]];
		for (int i = 1; i < tokens.size() - 1; i++) {
			currentRoot = currentRoot[tokens[i]];
		}
		return currentRoot.get(tokens[tokens.size() - 1], defaultValue);

	}
    
private:
    //==== SINGLETON STUFF ==============================================//
    ConfigHandler();
    // C++11:
    // Stop the compiler from generating copy methods for the object
    ConfigHandler(ConfigHandler const&) = delete;
    void operator=(ConfigHandler const&) = delete;
    //==== END SINGLETON STUFF ==============================================//
    
    const std::string CONFIG_FILE_PATH = "data/config/config.json";


};
