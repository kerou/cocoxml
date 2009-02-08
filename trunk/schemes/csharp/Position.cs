/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/

public class CcsPosition_t {
    public long beg;
    public int len;
    public int col;
    public string text;

    public CcsPosition_t(long beg, int len, int col, string text)
    {
	this.beg = beg;
	this.len = len;
	this.col = col;
	this.text = text;
    }

    ~CcsPosition_t()
    {
    }

    public CcsPosition_t Clone()
    {
	return new CcsPosition_t(beg, len, col, text);
    }
}
