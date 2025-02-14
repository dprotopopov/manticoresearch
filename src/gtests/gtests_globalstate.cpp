//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "threadutils.h"

// global stuff

const char * g_sTmpfile = "__libsphinxtest.tmp";
const char * g_sMagickTmpfile = "__libsphinxtest2.tmp";
const char * g_sMagic = "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82\xD1\x82\xD1\x82";

bool CreateSynonymsFile ( const char * sMagic = nullptr )
{
	FILE * fp = fopen ( sMagic ? g_sMagickTmpfile : g_sTmpfile, "w+" );
	if ( !fp )
		return false;

	fprintf ( fp, "AT&T      => AT&T\n"
			"   AT & T => AT & T  \n"
			"standarten fuehrer => Standartenfuehrer\n"
			"standarten   fuhrer  => Standartenfuehrer\n"
			"OS/2 => OS/2\n"
			"Ms-Dos => MS-DOS\n"
			"MS DOS => MS-DOS\n"
			"feat. => featuring\n"
			"U.S. => US\n"
			"U.S.A. => USA\n"
			"U.S.B. => USB\n"
			"U.S.D. => USD\n"
			"U.S.P. => USP\n"
			"U.S.A.F. => USAF\n"
			"life:) => life:)\n"
			"; => ;\n"
			);
	if ( sMagic )
		fprintf ( fp, "%s => test\n", sMagic );
	fclose ( fp );
	return true;
}

class Environment : public ::testing::Environment
{
public:
	// Override this to define how to set up the environment.
	void SetUp () override
	{
		char cTopOfMainStack;
		Threads::Init ();
		Threads::PrepareMainThread ( &cTopOfMainStack );
		CreateSynonymsFile ();
		CreateSynonymsFile ( g_sMagic );
		auto iThreads = sphCpuThreadsCount();
		//		iThreads = 1; // uncomment if want to run all coro tests in single thread
		SetMaxChildrenThreads ( iThreads );
		WipeGlobalSchedulerOnShutdownAndFork();
		StartGlobalWorkPool();
#if _WIN32
		// init WSA on Windows
	WSADATA wsa_data;
	int wsa_startup_err;

	wsa_startup_err = WSAStartup ( WINSOCK_VERSION, &wsa_data );
	if ( wsa_startup_err )
		printf ( "failed to initialize WinSock2: error %d", wsa_startup_err );
#endif

	}

	// Override this to define how to tear down the environment.
	void TearDown () override
	{
		unlink ( g_sTmpfile );
		unlink ( g_sMagickTmpfile );
	}
};

// it will create 2 synonyms file before all tests (globally), and delete them on finish.
::testing::Environment VARIABLE_IS_NOT_USED * const env = ::testing::AddGlobalTestEnvironment ( new Environment );
