#pragma once

#include "IFileWatcherListener.h"

class FileWatcherListener : public IFileWatcherListener
{
	// Inherited via IFileWatcherListener
	virtual void OnFileChange(const std::wstring& path) override;

	// Inherited via IFileWatcherListener
	virtual void OnFileAdded(const std::wstring& path) override;

	// Inherited via IFileWatcherListener
	virtual void OnFileRemoved(const std::wstring& path) override;

	// Inherited via IFileWatcherListener
	virtual void OnFileRenamed(const std::wstring& path) override;
};
