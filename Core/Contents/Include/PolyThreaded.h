/*
 *  PolyThreaded.h
 *  Poly
 *
 *  Created by Ivan Safrin on 3/6/09.
 *  Copyright 2009 Ivan Safrin. All rights reserved.
 *
 */
// @package Core
#pragma once
#include "PolyString.h"
#include "PolyGlobals.h"

namespace Polycode{
	
	class _PolyExport Threaded {
	public:
		Threaded(){ threadRunning = true; }
		~Threaded(){}
		
		virtual void killThread() { threadRunning = false; }		
		virtual void runThread(){while(threadRunning) updateThread(); }
		
		virtual void updateThread() = 0;
		
		bool threadRunning;
	};
	
}