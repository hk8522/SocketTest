// FileWatcherTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "network_init.h"
#include "FileWatcherListener.h"
#include "CFileSystemWatcher.h"
#include "EventSink.h"
#include <iostream>
#include <conio.h>

using namespace std;

int main()
{
	int err = network_init();
	if (err != 0) {
		fprintf(stderr, "Network initialization failed.\n");
		return EXIT_FAILURE;
	}

	CreationEvent::registerCreationCallback(nullptr);

	/*!
	 * Creates watcher by passing the watch directory
	 * 
	 */
	//CFileSystemWatcher watcher(L"D:\\Log");
	CFileSystemWatcher watcher(L"C:\\");
	/*!
	 *
	 * Creates Linstener
	 */
	IFileWatcherListener* listener = new FileWatcherListener();
	
	//passes the listener to watcher
	watcher.AddFileChangeListener(listener);
#if false
	int choice;
	do
	{
		cout << "\n\nMENU";
		cout << "\n1. Start Directory Watcher";
		cout << "\n2. Stop Directory Watcher";
		cout << "\n3. Exit";
		cout << "\n\nEnter your choice 1-3 :";
		cin >> choice;

		switch (choice)
		{
		case 1:
		  {
			if (!watcher.Running())
				watcher.Start();
			else
				cout << "\nWatcher is already started ..";
		  }
			break;
		case 2:		
			watcher.Stop();
			break;		
		case 3: 
			exit(0);
			break;
		default:
			cout << "\nInvalid choice";
		}
	} while (choice != 3);
#else
	watcher.Start();
	cout << "\nPress any key to exit.";
	cin.get();
#endif;

	watcher.Stop();
	delete listener;

	network_cleanup();

    return 0;
}

