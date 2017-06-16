#include "ConfigHandler.hpp"

ConfigHandler::ConfigHandler(){
	std::ifstream config_doc(CONFIG_FILE_PATH, std::ifstream::binary);
	//config_doc >> root;
	Json::CharReaderBuilder rbuilder;
	// Configure the Builder, then ...
	std::string errs;
	bool parsingSuccessful = Json::parseFromStream(rbuilder, config_doc, &root, &errs);
	if (!parsingSuccessful)
	{
		// report to the user the failure and their locations in the document.
		std::cout << "Failed to parse configuration\n"
			<< errs;
		return;
	}

}