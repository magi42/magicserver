/***************************************************************************
 *   This file is part of the MagiCServer++ library.                       *
 *                                                                         *
 *   Copyright (C) 2003 Marko Grönroos <magi@iki.fi>                       *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Library General Public            *
 *  License as published by the Free Software Foundation; either           *
 *  version 2 of the License, or (at your option) any later version.       *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Library General Public License for more details.                       *
 *                                                                         *
 *  You should have received a copy of the GNU Library General Public      *
 *  License along with this library; see the file COPYING.LIB.  If         *
 *  not, write to the Free Software Foundation, Inc., 59 Temple Place      *
 *  - Suite 330, Boston, MA 02111-1307, USA.                               *
 *                                                                         *
 ***************************************************************************/

#ifndef __MAGICSERVER_MSRVCONTAINER_H__
#define __MAGICSERVER_MSRVCONTAINER_H__

#include <magicserver/msrverror.h>
#include <stdlib.h>

begin_namespace (MSrv);

/*******************************************************************************
 * List item.
 *
 * These items form a double-linked list useful for queues.
 ******************************************************************************/
template <class TYPE>
class ListItem {
  public:
	/** Creates a new list item and links it to the next. */
							ListItem (TYPE* req, ListItem<TYPE>* pNext=NULL) {
								mpData    = req;
								mpPrev    = NULL;
								mpNext    = pNext;
							}

	/** Destroys the list item AND associated data AND (recursively) all linked items. */
							~ListItem () {
								delete mpData;
								delete mpNext;
							}

	/** Returns the data object contained in the list item. */
	inline TYPE*			get 	() {return mpData;}

	/** Returns pointer to the previous list item in the list. */
	inline ListItem<TYPE>*	prev	() {return mpPrev;}

	/** Returns pointer to the next list item in the list. */
	inline ListItem<TYPE>*	next	() {return mpNext;}

	/** Sets the previous list item in the list. */
	inline void             setPrev	(ListItem<TYPE>* prev) {mpPrev = prev;}
	
	/** Sets the next list item in the list. */
	inline void             setNext	(ListItem<TYPE>* next) {mpNext = next;}

	/** Sets the data object contained in the list item. */
	inline void				set		(TYPE* pItem) {mpData = pItem;}
	
  private:
	TYPE*			mpData;
	ListItem<TYPE>*	mpPrev;
	ListItem<TYPE>*	mpNext;
};

/*******************************************************************************
 * Queue
 ******************************************************************************/
template <class TYPE>
class Queue {
  public:
	/** Creates a new empty queue. */
	Queue () {
		mpFirstItem = NULL;
		mpLastItem  = NULL;
	}

	/** Destroys the queue and all the items is contains. */
	~Queue () {
		mThreadLock.lock ();

		delete mpFirstItem;

		mThreadLock.unlock ();
	}

	/** Pushes an item to the beginning of the queue. */
	void push (TYPE* pItem) {
		mThreadLock.lock ();

		ListItem<TYPE>* pOldFirst = mpFirstItem;

		/* Insert new item in the beginning of the queue. */
		mpFirstItem = new ListItem<TYPE> (pItem, pOldFirst);

		if (pOldFirst)
			pOldFirst->setPrev (mpFirstItem);

		if (!mpLastItem)
			mpLastItem = mpFirstItem;
	
		mThreadLock.unlock ();
	}

	/** Pulls an item from the end of the queue. */
	TYPE* pull () {
		TYPE* pResult = NULL;
		
		mThreadLock.lock ();
		
		ListItem<TYPE>* pLastItem = mpLastItem;

		/* If there are items in the queue. */
		if (pLastItem) {
			/* Get the last item. */
			pResult = pLastItem->get ();
			
			/* If there are other items remaining. */
			if (pLastItem->prev()) {
				/* Readjust the container. */
				mpLastItem = pLastItem->prev();

				/* Readjust the new last item. */
				mpLastItem->setNext (NULL);
			}
			else {
				/* No remaining items -> no first either. */
				mpLastItem = mpFirstItem = NULL; 
			}
			
			pLastItem->set (NULL);
			delete pLastItem;
		}

		mThreadLock.unlock ();

		return pResult;
	}
	
	
  private:
	ListItem<TYPE>*	mpFirstItem; /**< First item in the queue.     */
	ListItem<TYPE>*	mpLastItem;  /**< Last item in the queue.      */
	ThreadLock		mThreadLock;
};

/*******************************************************************************
 * Generic array container
 ******************************************************************************/
template <class TYPE>
class Array {
  public:
	/** Creates an empty array. */
	Array () {
		mpItems    = NULL;
		mItemCount = 0;
	}

	/** Adds an item to the end of array. */
	MSrvResult add		(TYPE* item) {
		mThreadLock.lock ();

		/* Resize the array. */
		TYPE* pNewItems = (TYPE*) malloc ((mItemCount + 1) * sizeof(TYPE));

		/* Copy the old descriptors. */
		if (mItemCount > 0)
			memcpy (pNewItems, mpItems, mItemCount * sizeof(TYPE));
	
		/* Append the new descriptor. */
		pNewItems[mItemCount] = *item;

		/* Update the member attributes. */
		free (mpItems);
		mpItems = pNewItems;
		mItemCount++;

		mThreadLock.unlock ();
		return 0;
	}

	/** Removes the item at given position and shifts down the rest of the array. */
	MSrvResult remove (int pos) {
		mThreadLock.lock ();

		if (pos > mItemCount)
			return MSRVERR_INVALID_ARGUMENT;

		/* If it is the last item... */
		if (mItemCount == 1) {
			/* Yes, It's the last one. Empty the entire descriptor array. */
			free (mpItems);
			mpItems = NULL;
		}
		else {
			/* There will be other items left. */
			
			/* Create a resized array. */
			TYPE* pNewItems = (TYPE*) malloc ((mItemCount - 1) * sizeof (TYPE));
			
			/* Copy the lower part of the array. */
			if (pos > 0)
				memcpy (pNewItems, mpItems, pos * sizeof (TYPE));

			/* Drop the upper part of the array on step down. */
			if (pos < mItemCount - pos)
				memcpy (pNewItems + pos,
						mpItems + (pos+1),
						(mItemCount - pos - 1) * sizeof (TYPE));
			
			/* Update the members. */
			free (mpItems);
			mpItems = pNewItems;
		}
		mItemCount--;

		mThreadLock.unlock ();
		return 0;
	}

	/** Returns the number of items in the array. */
	int length	() const {
		return mItemCount;
	}

	/** Returns a reference to the item in given position. */
	TYPE& operator[] (int pos) {return mpItems[pos];}

	/** Array iterator. */
	class Iterator {
	  public:
		/** Creates an iterator for the given array. */
		Iterator (Array<TYPE>& array) : mrArray (array) {
			mPos = 0;
		}

		/** Retrieves reference to the data item at current position. */
		TYPE&	get		() {return mrArray[mPos];}

		/** Moves to next array position. */
		void	next	() {mPos++;}

		/** Are there any more items? */
		bool	exhausted	() {return mPos >= mrArray.length ();}
		
	  private:
		int         mPos;
		Array<TYPE> mrArray;
	};

  private:
	TYPE*		mpItems;    /**< Descriptors to watch for status change. */
	int			mItemCount; /**< Number of descriptors.                  */
	ThreadLock	mThreadLock;
};




end_namespace (MSrv);

#endif
