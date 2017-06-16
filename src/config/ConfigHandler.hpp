#pragma once

#include <stdlib.h>
#include <iostream>

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
    
public:
    
    /**
     Get the config statically using fewer commands
     */
	static Json::Value GetValue(const std::string &key, const Json::Value &defaultValue) {
		return GetInstance().root.get(key, defaultValue);
	}
	Json::Value root;
    
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
