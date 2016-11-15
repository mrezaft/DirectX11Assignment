
//
// GUConsole.h
//

// GUConsole represents a (singleton) console window

#pragma once

#include <GUObject.h>

class GUConsole : public GUObject {

	// Flag to determine if AllocConsole was successful or not
	BOOL _allocSuccessful = FALSE;

	// Standard input / output file handles
	FILE *stdinFile = nullptr;
	FILE *stdoutFile = nullptr;
	FILE *stderrFile = nullptr;


	//
	// Private interface
	//

	// Private constructor
	GUConsole(const wchar_t* consoleTitle);

public:

	//
	// Public interface
	//

	// Console factory method
	static GUConsole* CreateConsole(const wchar_t* consoleTitle);

	// Destructor
	~GUConsole();
};
