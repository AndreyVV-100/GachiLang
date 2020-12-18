#include "Stack.h"

void StackConstructor (Stack* stk, const size_t max_size)
{
	assert (stk);
	assert (max_size);

	if (!(stk->buffer == ERR_FREE && stk->status_error == STK_DEL ||
		  stk->buffer == nullptr  && stk->status_error == NOT_CREATED))
	{
		stk->status_error = BAD_CREATE;
		StackLog (stk);
		return;
	}

	stk_type* buf = (stk_type*) calloc (max_size + 2, sizeof (stk_type));
	
	if (!buf)
	{
		stk->status_error = DO_MEM_ERR;
		StackLog (stk);
		return;
	}
	
	*((storm*)  buf)				 = STORMY_PETREL;
	*((storm*) (buf + max_size + 1)) = STORMY_PETREL;

	StackPoison (buf + 1, max_size);

	stk->buffer		  = buf + 1;
	stk->status_error = STK_GOOD;
	stk->capacity     = max_size;
	stk->min_capacity = max_size;

	RecountHash (stk);

	return;
}

void StackDestructor (Stack* stk)
{
	ASSERT_OK
	
	stk->buffer -= 1;

	StackPoison (stk->buffer, stk->capacity + 2);

	free (stk->buffer);

	stk->buffer = ERR_FREE;
	stk->size = 0;
	stk->capacity = 0;
	stk->status_error = NOT_CREATED;
	stk->buf_hash = 0;
	stk->stk_hash = 0;

	return;
}

void StackPoison (stk_type* buffer, size_t num)
{
	assert (buffer);
	
	for (size_t elem = 0; elem < num; elem++)
	{
		free (buffer[elem].ind);
		buffer[elem] = STK_POISON;
	}

	return;
}

void StackPush (Stack* stk, stk_type elem)
{
	ASSERT_OK

	if (stk->size == stk->capacity)
		StackResizeUp (stk);

	if (stk->status_error == DO_MEM_ERR)
		return;

	*(stk->buffer + (stk->size)++) = elem;

	RecountHash (stk);

	ASSERT_OK

	return;
}

stk_type StackPop (Stack* stk)
{
	ASSERT_OK_POP
	
	if (stk->size == 0)
	{
		stk->status_error = BAD_SIZE_ZERO;
		StackLog (stk);
		return STK_POISON;
	}
	
	if (stk->capacity > stk->min_capacity &&
		stk->capacity >= STK_RESIZE * STK_RESIZE * stk->size)
			StackResizeDown(stk);

	if (stk->status_error == DO_MEM_ERR)
		return STK_POISON;

	stk_type popped = stk->buffer[--(stk->size)];
	*(stk->buffer + stk->size) = STK_POISON;

	RecountHash(stk);

	ASSERT_OK_POP

	return popped;
}

void StackResizeUp (Stack* stk)
{
	ASSERT_OK

	stk_type* buf = stk->buffer - 1;

	buf = (stk_type*) realloc (buf, sizeof (stk_type) * (stk->capacity * STK_RESIZE + 2));

	if (!buf)
	{
		stk->status_error = DO_MEM_ERR;
		StackLog (stk);
		return;
	}

	(stk->capacity) *= STK_RESIZE;

	*((storm*)  buf)					  = STORMY_PETREL;
	*((storm*) (buf + stk->capacity + 1)) = STORMY_PETREL;

	StackPoison (buf + stk->size + 1, stk->capacity - stk->size);

	stk->buffer = buf + 1;

	RecountHash (stk);

	ASSERT_OK

	return;
}

void StackResizeDown (Stack* stk)
{
	ASSERT_OK

	stk_type* buf = stk->buffer - 1;

	if (!buf)
	{
		stk->status_error = DO_MEM_ERR;
		StackLog (stk);
		return;
	}

	*(buf + stk->capacity + 1) = STK_POISON;

	if (stk->capacity >= STK_RESIZE * stk->size && 
		stk->capacity >= STK_RESIZE * stk->min_capacity)
			buf = (stk_type*) realloc (buf, 
									  sizeof (stk_type) * (stk->capacity / STK_RESIZE + 2));

	stk->capacity /= STK_RESIZE;
	*((storm*) (buf + stk->capacity + 1)) = STORMY_PETREL;
	stk->buffer = buf + 1;

	RecountHash (stk);

	ASSERT_OK

	return;
}

int StackError (Stack* stk)
{
	if (!stk)
		return STK_NULL;

	if (stk->stormy_petrel_begin != STORMY_PETREL)
		return stk->status_error = STK_PETREL_B;

	if (stk->stormy_petrel_end != STORMY_PETREL)
		return stk->status_error = STK_PETREL_E;
	
	if (stk->buffer == ERR_FREE)
		return stk->status_error = STK_DEL;
	
	if (!stk->buffer)
		return stk->status_error = BUF_NULL;
	
	if (stk->size > stk->capacity)
		return stk->status_error = BAD_SIZE_CAP;

	if (stk->capacity < stk->min_capacity)
		return stk->status_error = BAD_CAP;
	
	if (*((storm*) (stk->buffer - 1)) != STORMY_PETREL)
		return stk->status_error = BUF_PETREL_B;

	if (*((storm*) (stk->buffer + stk->capacity)) != STORMY_PETREL)
		return stk->status_error = BUF_PETREL_E;

	if (stk->status_error != 0)
		return stk->status_error;

	unsigned long long hash1 = stk->stk_hash;
	unsigned long long hash2 = stk->buf_hash;
	RecountHash (stk);

	if (stk->stk_hash != hash1)
		return stk->status_error = ERR_HASH_STK;

	if (stk->buf_hash != hash2)
		return stk->status_error = ERR_HASH_BUF;
	
	return 0;
}

void StackLog(Stack* stk)
{
	FILE* file = fopen ("stklog.txt", "a");

	struct tm* mytime = nullptr;
	time_t alltime = 0;
	time (&alltime);
	mytime = localtime (&alltime);
	fprintf (file, "\n%s", asctime (mytime));

	if (!stk)
	{
		fprintf (file, "Stack [STK_NULL] : The address of stack is null.\n\n");

		fclose (file);

		return;
	}

	switch (stk->status_error)
	{
		case BUF_NULL:
			fprintf(file, "Stack [BUF_NULL] : The address of buffer is nullptr.\n" "End of log.\n");
			fclose(file);
			return;

		case STK_PETREL_B:
			fprintf(file, "Stack [STK_PETREL_B] : The begin stormy petrel in stack was damaged.\n" "End of log.\n");
			fclose(file);
			return;

		case STK_PETREL_E:
			fprintf(file, "Stack [STK_PETREL_E] : The end stormy petrel in stack was damaged.\n" "End of log.\n");
			fclose(file);
			return;

		case STK_DEL:
			fprintf(file, "Stack [STK_DEL] : The stack was deleted.\n" "End of log.\n");
			fclose(file);
			return;

		case DO_MEM_ERR:
			fprintf(file, "Stack [DO_MEM_ERR] : The program can`t pick out new memory.\n" "End of log.\n");
			fclose(file);
			return;

		case NOT_CREATED:
			fprintf(file, "Stack [NOT_CREATED] : Attempt to manipulation with stack which wasn`t constructed.\n" "End of log.\n");
			fclose(file);
			return;

		case STK_GOOD:
			fprintf (file, "Stack [STK_GOOD] : Nice!\n");
			break;

		case BAD_SIZE_CAP:
			fprintf (file, "Stack [BAD_SIZE_CAP] : The size of stack is more then capacity.\n");
			break;

		case BAD_SIZE_ZERO:
			fprintf (file,  "Stack [BAD_SIZE_ZERO] : StackPop was called when size was zero.\n");
			break;

		case BAD_CAP:
			fprintf (file, "Stack [BAD_CAP] : The capacity of stack is less then minimal capacity.\n");
			break;

		case BUF_PETREL_B:
			fprintf (file, "Stack [BUF_PETREL_B] : The begin stormy petrel in buffer was damaged.\n");
			break;

		case BUF_PETREL_E:
			fprintf (file, "Stack [BUF_PETREL_E] : The end stormy petrel in buffer was damaged.");
			break;

		case BAD_CREATE:
			fprintf (file, "Stack [BAD_CREATE] : Trying to construct stack which already not empty.\n");
			break;

		case ERR_HASH_STK:
			fprintf (file, "Stack [ERR_HASH_STK] : Data in stack was changed illegally.");
			break;

		case ERR_HASH_BUF:
			fprintf (file, "Stack [ERR_HASH_BUF] : Data in buffer was changed illegally.");
			break;

		default:
			fprintf (file, "Stack [UNK] : Unknown error.\n");
			break;
	};

	fprintf (file, "size = %d\n" "capacity = %d\n" "min_capacity = %d\n" "buffer:\n", 
			   stk->size, 	 stk->capacity,	   stk->min_capacity);

	/*
	fprintf (file, "begin buffer petrel = %llX\n" "end buffer petrel   = %llX\n",
		*((storm*) (stk->buffer - 1)),
		*((storm*) (stk->buffer + stk->capacity)));
	

	for (int element = 0; element < stk->capacity; element++)
		fprintf (file, "[%d] = %lf\n", element, stk->buffer[element]);
		*/
	fprintf (file, "\n" "End of log.\n");
	fclose  (file);

	return;
}

unsigned long long CountHash (char* buffer, size_t num)
{
	const int shift = 5;
	unsigned long long ans = 0;
	char buf = 0;
	
	for (size_t count = 0; count < num; count++)
	{
		ans += buffer[count];
		buf = ans >> (sizeof (unsigned long long) - shift);
		ans = (ans << shift) + buf;
	}

	return ans;
}

void RecountHash (Stack* stk)
{
	return; // Warning: using for work with all elements. Isn't main stack mode.
	
	assert (stk);
	
	stk->stk_hash = 0;
	stk->buf_hash = 0;
	
	stk->stk_hash = CountHash ((char*) stk,			STK_HASH);
	stk->buf_hash = CountHash ((char*) stk->buffer, stk->capacity * sizeof (stk_type));

	return;
}
