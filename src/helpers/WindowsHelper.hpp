#pragma once

#include <windows.h>
#include <stdio.h>
#include <string>

using namespace std;

inline bool CreateDirectoryIfNotExist(string path) {
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, wString, 4096);
	return CreateDirectoryW(wString, NULL) || ERROR_ALREADY_EXISTS == GetLastError();
}

/**
Gives the number of regular files in a directory.
*/
inline size_t NumberOfFilesInDirectory(filesystem::path path)
{
	using filesystem::directory_iterator;
	using fp = bool(*)(const filesystem::path&);
	return count_if(directory_iterator(path), directory_iterator{}, (fp)filesystem::is_regular_file);
}