/*******************************************************************************

Copyright Datapath Ltd. 2008.

File:    Sample1.h

Purpose: 

History:
         01 JAN 08    RL   Created.

/* Macros *********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/* Constants ******************************************************************/

#define APPLICATIONNAME TEXT("DirectShow Vision Sample1\0")

/******************************************************************************/