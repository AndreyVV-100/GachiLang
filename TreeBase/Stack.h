/*!
	@mainpage This one of infinity realizations of stack.
	@authors <a href = "https://vk.com/andreyid1"> Andrey Vyazovtsev </a>
	@version 1.4.48
	@date 13.10.2020
	@copyright GNU GPL
*/

#define ERR_FREE (stk_type*) 228
#define ASSERT_OK if (StackError (stk)) { StackLog (stk); return;}
#define ASSERT_OK_POP if (StackError (stk)) { StackLog (stk); return STK_POISON;}

enum Types
{
    NIL   = 0,
    DEC   = 1,
    FUNC  = 2,
    LR    = 3,
    PARAM = 4,
    BODY  = 5,
    ARITH = 6,
    NUM   = 7,
    VAR   = 8,
    IND   = 9,
    RET   = 10,
    IF    = 11,
    WHILE = 12,
    CALL  = 13,
    EQUAL = 14,
    COND  = 15
};

struct element
{
    Types type      = NIL;
    element* left   = nullptr;
    element* right  = nullptr;
    char* ind       = nullptr;
    size_t len      = 0;
    char var_pos    = 0;
};


typedef element stk_type;
typedef unsigned long long storm;

/*!
    Bad ded idet v botalku ("v" = "b", "ku" didn`t fit).
*/

const storm STORMY_PETREL = 0xBADDED1DE7BB07A1;

//! This is list of errors with them values.

enum stack_errors { 
	STK_GOOD	  = 0,  //!< Nice!
	STK_NULL	  = 1,  //!< The address of stack is null.
	STK_DEL		  = 2,  //!< The stack was deleted.
	BUF_NULL	  = 3,  //!< The address of buffer is nullptr.
	BAD_SIZE_CAP  = 4,  //!< The size of stack is more then capacity.
	BAD_SIZE_ZERO = 5,  //!< StackPop was called when size was zero.
	BAD_CAP		  = 6,  //!< The capacity of stack is less then minimal capacity.
	STK_PETREL_B  = 7,  //!< The begin stormy petrel in stack was damaged.
	STK_PETREL_E  = 8,  //!< The end stormy petrel in stack was damaged.
	BUF_PETREL_B  = 9,  //!< The begin stormy petrel in buffer was damaged.
	BUF_PETREL_E  = 10, //!< The end stormy petrel in buffer was damaged.
	DO_MEM_ERR	  = 11, //!< The program can`t pick out new memory.
	NOT_CREATED   = 12, //!< Attempt to manipulation with stack which wasn`t constructed.
	BAD_CREATE	  = 13, //!< Trying to construct stack which already not empty.
	ERR_HASH_STK  = 14, //!< Data in stack was changed illegally.
	ERR_HASH_BUF  = 15  //!< Data in buffer was changed illegally.
};

/*!
@{
*/

const stk_type STK_POISON = { NIL, nullptr, nullptr, nullptr };
const size_t   STK_RESIZE = 2;
const int STK_HASH = 2 * sizeof (storm) + 3 * sizeof (size_t) +
					sizeof (stk_type*) + sizeof (int) + 2 * sizeof (unsigned long long);
//! @}

struct Stack
{
    storm stormy_petrel_begin = STORMY_PETREL;

    stk_type* buffer = nullptr;
    size_t	size = 0,
        capacity = 0,
        min_capacity = 0;
    int status_error = 12; // See stack_errors in Stack.h

    unsigned long long stk_hash = 0;
    unsigned long long buf_hash = 0;

    storm stormy_petrel_end = STORMY_PETREL;
};

/*!
	This function initializes your stack.
	@param [out] stk A pointer to stack, which needs initializing.
	@param [in] max_size A capacity and min_capacity of stack (capacity can change after).
*/

void StackConstructor (Stack* stk, size_t max_size);

/*!
	This function deletes your stack.
	@param [out] stk A pointer to stack, which needs deleting.
*/

void StackDestructor  (Stack* stk);

/*!
	This function pushs an element to stack.
	@param [out] stk A pointer to stack, which needs pushing.
	@param [in] elem An elemement, which will be pushed.
*/

void StackPush (Stack* stk, stk_type elem);

/*!
	This function pops top element of stack.
	@param [out] stk A pointer to stack, which needs popping.
	@return This function returns popped element.
*/

stk_type StackPop (Stack* stk);

/*!
	This function enlarges capacity of stack.
	@param [out] stk A pointer to stack, which needs enlarging.
*/

void StackResizeUp (Stack* stk);

/*!
	This function reduces capacity of stack.
	@param [out] stk A pointer to stack, which needs reducing.
*/

void StackResizeDown (Stack* stk);

/*!
	This function checks stack on existence of errors.
	@param [out] stk A pointer to stack, which needs checking.
*/

int StackError (Stack* stk);

/*!
	This function writes stack log.
	@param [out] stk A pointer to stack, which needs logging.
*/

void StackLog (Stack* stk);

/*!
	This function fills part of buffer by poison.
	@param [out] buffer A pointer to start of place, where poison will be filled.
	@param [in] num A number of elements, which needs be poison.
*/

void StackPoison (stk_type* buffer, size_t num);

/*!
	This function counts hash.
	@param [in] buffer A place, when hash will be counted.
	@param [in] num A size of buffer in bytes.
*/

unsigned long long CountHash(char* buffer, size_t num);

/*!
	This function recounts hash of stack and buffer.
	@param [out] buffer A pointer to start of place, where poison will be recounted.
*/

void RecountHash (Stack* stk);
