/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/

public class CcsToken_t {
    public CcsToken_t next;
    public int        refcnt;
    public int        kind;
    public long       pos;
    public int        col;
    public int        line;
    public string     val;

    public CcsToken_t(int kind, long pos, int col, int line, string val)
    {
	this.next = null;
	this.refcnt = 0;
	this.kind = kind;
	this.pos = pos;
	this.col = col;
	this.line = line;
	this.val = val;
    }

    ~CcsToken_t()
    {
    }
};
