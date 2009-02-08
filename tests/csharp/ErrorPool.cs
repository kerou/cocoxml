/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
using System;
using System.IO;

public class CcsErrorPool_t {
    TextWriter writer;
    int warningCount;
    int errorCount;

    public CcsErrorPool_t(TextWriter writer)
    {
	this.writer = writer;
	warningCount = 0;
	errorCount = 0;
    }

    ~CcsErrorPool_t()
    {
    }

    public void Info(string s)
    {
	writer.WriteLine(s);
    }

    public void Warning(int line, int col, string s)
    {
	writer.WriteLine("Warning({0}, {1}): {2}", line, col, s);
	++warningCount;
    }

    public void Error(int line, int col, string s)
    {
	writer.WriteLine("Error({0}, {1}): {2}", line, col, s);
	++errorCount;
    }

    public void Fatal(int line, int col, string s)
    {
	writer.WriteLine("Fatal Error({0}, {1}): {2}", line, col, s);
	Environment.Exit(-1);
    }
};
