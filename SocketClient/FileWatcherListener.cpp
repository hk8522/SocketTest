#include "stdafx.h"
#include "network_init.h"
#include "FileWatcherListener.h"
#include <iostream>
#include <conio.h>
#include <memory.h>

#include "Client.h"

using namespace std;

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileChange(const std::wstring& path)
{
	wcout << L"OnFileChange " << path << endl;
	writeFileEvent(path, L"file_change");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileAdded(const std::wstring& path)
{
	wcout << L"OnFileAdded " << path << endl;
	writeFileEvent(path, L"file_created");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileRemoved(const std::wstring& path)
{
	wcout << L"OnFileRemoved " << path << endl;
	writeFileEvent(path, L"file_deleted");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileRenamed(const std::wstring& path)
{
	wcout << L"OnFileRenamed " << path << endl;
	writeFileEvent(path, L"file_renamed");
}
