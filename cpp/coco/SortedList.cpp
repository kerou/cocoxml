/*-------------------------------------------------------------------------
Compiler Generator Coco/R,
Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
extended by M. Loeberbauer & A. Woess, Univ. of Linz
ported to C++ by Csaba Balazs, University of Szeged
with improvements by Pat Terry, Rhodes University

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
for more details.

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, Inc., 
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

As an exception, it is allowed to write an extension of Coco/R that is
used as a plugin in non-free software.

If not otherwise stated, any source code generated by Coco/R (other than 
Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/

#include <stdio.h>
#include "SortedList.h"
#include "Tab.h"

namespace Coco {

SortedEntry::SortedEntry(Symbol* Key, void* Value) {
	this->Key   = Key;
	this->Value = Value;
	this->next = NULL;
}

SortedEntry::~SortedEntry() {
};

SortedList::SortedList() {
	Count = 0;
	Data = NULL;
}

SortedList::~SortedList() {
}

bool SortedList::Find(Symbol* key) {
	SortedEntry* pSortedEntry = Data;
	while (pSortedEntry) {
		if (!(pSortedEntry->Key)->CompareTo(key))
			return true;
		pSortedEntry = pSortedEntry->next;
	}
	return false;
}

void SortedList::Set(Symbol *key, void *value) {
	if (!Find(key)) {
		// new entry
		SortedEntry* pSortedEntry = Data;
		SortedEntry* pSortedEntryPrev = NULL;
		SortedEntry* newSortedEntry = new SortedEntry(key, value);
		if (pSortedEntry) {
			// insert

			if (pSortedEntry->Key->CompareTo(key) > 0) {	// before the first
				newSortedEntry->next = Data;
				Data = newSortedEntry;
			} else {
				while (pSortedEntry) {
					if (pSortedEntry->Key->CompareTo(key) < 0) {
						pSortedEntryPrev = pSortedEntry;
						pSortedEntry = pSortedEntry->next;
					} else {
						break;
					}
				}
				pSortedEntryPrev->next = newSortedEntry;
				newSortedEntry->next = pSortedEntry;
			}
		} else {
			Data = newSortedEntry;			// first entry
		}
		Count++;
	} else {
		// exist entry - overwrite
		SortedEntry* pSortedEntry = Data;
		while ((pSortedEntry->Key)->CompareTo(key)) {
			pSortedEntry = pSortedEntry->next;
		}
		pSortedEntry->Value = value;
	}
}

void* SortedList::Get( Symbol* key ) const // Value
{
	SortedEntry* pSortedEntry = Data;
	while (pSortedEntry) {
		if (!(pSortedEntry->Key)->CompareTo(key))
			return pSortedEntry->Value;
		pSortedEntry = pSortedEntry->next;
	}
	return NULL;
}


void* SortedList::GetKey( int index ) const // Key
{
	if (0 <= index && index < Count) {
		SortedEntry* pSortedEntry = Data;
		for (int i=0; i<index; i++) {
			pSortedEntry = pSortedEntry->next;
		}
		return pSortedEntry->Key;
	} else {
		return NULL;
	}
}

SortedEntry* SortedList::operator[]( int index ) const {
	if (0 <= index && index < Count) {
		SortedEntry* pSortedEntry = Data;
		for (int i=0; i<index; i++) {
			pSortedEntry = pSortedEntry->next;
		}
		return pSortedEntry;
	} else {
		return NULL;
	}
}

}; // namespace
