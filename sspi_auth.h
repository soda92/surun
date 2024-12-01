///////////////////////////////////////////////////////////////////////////////
// This code was taken from the: SSPI Authentication Sample
//
//  This program demonstrates how to use SSPI to authenticate user credentials.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2001.  Microsoft Corporation.  All rights reserved.
///////////////////////////////////////////////////////////////////////////////
#pragma once

HANDLE SSPLogonUser(LPCTSTR szDomain, LPCTSTR szUser, LPCTSTR szPassword);
