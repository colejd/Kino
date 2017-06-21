#pragma once

#include <string>

namespace Paths {

	/// https://stackoverflow.com/a/5523506
	/// Gives the last component of a string representing a path,
	/// or the string itself if no separators are found.
	inline std::string GetFileNameFromPath(std::string path) {

		size_t pos = path.find_last_of("/");
		if (pos != std::string::npos) {
			return path.substr(pos + 1);
		}
		return path;

	}


}