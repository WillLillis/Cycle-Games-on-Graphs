/*
* 
* Just going to throw all the random helper functions in here that don't logically
* fit within the other files
* 
*/
#pragma once
#include <cstdarg>
#include <cmath>

/*
*
* Have to have the following #define mess because __FUNCSIG__ isn't valid 
* outside of Microsoft's compiler...
* 
*/

// can clean this up with #elif?

// can add more as the need arises
#ifndef __FUNCSIG__
	#ifdef __PRETTY_FUNCTION__
		#define __FUNCSIG__ __PRETTY_FUNCTION__
	#else 
		#ifdef __func__ // not a macro so not defined?
			#define __FUNCSIG__ __func__
		#else
			#ifdef __FUNCTION__
				#define __FUNCSIG__ __FUNCTION__
			#else
				#define __FUNCSIG__ "<Function name macro error>"
			#endif // __FUNCTION__
		#endif // __func__
	#endif // __PRETTY_FUNCTION__
#endif //__FUNCSIG__




/****************************************************************************
* display_error
*
* - Prints a message to the console indicating an error. Gives the file, line
* number, and function in which the error occured. Also allows the caller to 
* provide a custom message to print along with the error, in a "printf style"
* using a const char* format string and variable arguments
*
* Parameters :
* - file_name : name of the file in which the error occurred, grabbed using the
* __FILE__ macro
* - line_num : the line number where the file occurred, grabbed using the 
* __LINE__ macro
* - func_sig : the signature of the function in which the error occurred, grabbed
* using the __FUNCSIG__ macro
* - user_clear : indicates whether the user will have to provide an input in order
* to clear/ continue past the error
* - err_msg : format string for the user's custom error message
* - ... : variable number of optional arguments corresponding to the format string
*
* Returns :
* - none 
****************************************************************************/
void display_error(const char* file_name, int line_num, const char* func_sig, bool user_clear, const char* err_msg, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, err_msg);

	printf("ERROR: ");
	vprintf(err_msg, arg_ptr);
	va_end(arg_ptr);

	printf("\n\t[FILE] %s\n", file_name);
	printf("\t[LINE] %d\n", line_num);
	printf("\t[FUNC] %s\n", func_sig);

	if (user_clear)
	{
		printf("Press [ENTER] to continue...\n");
		char throw_away = std::getchar();
	}
}

/****************************************************************************
* is_number
*
* - Indicates whether the supplied std::string contains just a number
* - Used as a lil helper function when verifying user inputs
* - Empty strings will be returned as false
*
* Parameters :
* - input : the std::string in question
* 
* Returns :
* - bool : true if input is a number, false otherwise
****************************************************************************/
bool is_number(std::string input)
{
	bool non_empty = false;
	for (char c : input) // scary modern C++ voodoo magic iterator
	{
		non_empty = true;
		if (!std::isdigit(c))
		{
			return false;
		}
	}

	return non_empty; // want to make sure an empty string wasn't passed in
}

/****************************************************************************
* erase_lines
*
* - Clears the specified number of lines in the console
* - Uses ASCII escape sequences
* - https://copyprogramming.com/howto/c-how-do-i-erase-a-line-from-the-console
*
* Parameters :
* - num_lines : the number of lines to clear
*
* Returns :
* - none
****************************************************************************/
void erase_lines(uint_fast16_t num_lines)
{
	if (num_lines > 0)
	{
		printf("\x1b[2K"); // Delete current line

		for (uint_fast16_t line = 1; line < num_lines; line++) // line = 1 because we included the first line
		{
			printf("\x1b[1A"); // Move cursor up one
			printf("\x1b[2K"); // Delete the entire line
		}
		printf("\r"); // Resume the cursor at beginning of line
	}
}

/****************************************************************************
* index_translation
*
* - Translates a 2-dimensional i,j coordinate to its equivalent 1-dimensional
* index
* - only valid for "matrices" with n columns
*
* Parameters :
* - num_cols : the total number of columns in the matrix
* - row : the row of the entry in question
* - col : the column of the entry in question
*
* Returns :
* - size_t : the translated 2-D to 1-D index
****************************************************************************/
// Do we want to make the switch from adjacency matrices to listings, and 
// have this function do the necessary translations?
// just swap the max value into the row or something 
// would need to allocate n(n+1)/2 instead of n*n->Talk with Kelvey/ Gates
	// if directed graphs are at all a possibility, don't do it
inline size_t index_translation(uint_fast16_t num_cols, uint_fast16_t row, uint_fast16_t col)
{
	return ((size_t)num_cols * (size_t)row) + (size_t)col; // casts necessary?
}

/****************************************************************************
* get_file_length
*
* - Takes in a pointer to an ALREADY OPEN std::fstream and returns the length
* of the file in bytes
* - Any iostream errors/ flags set before the function call will be cleared
*
* Parameters :
* - file : pointer to the fstream in question
*
* Returns :
* - size_t : the length in bytes of the file
****************************************************************************/
size_t get_file_length(std::fstream* file)
{
	if (!(*file).is_open())
	{
		display_error(__FILE__, __LINE__, __FUNCSIG__, true,
			"The supplied file stream is not open.");
		return 0;
	}
	//std::streampos pos = (*file).tellg(); 
	size_t pos = (size_t)(*file).tellg(); // save the initial position so we can return the file back to the caller unchanged
	(*file).ignore(std::numeric_limits<std::streamsize>::max());
	size_t file_length = (size_t)(*file).gcount();
	(*file).clear(); // clear the EOF bit set by our going to the end of the file, any other errors
	(*file).seekg(0, /*(std::ios_base::seekdir)*/pos); // set the position back to where the caller had it because we're courteous like that
	return file_length;
}

/****************************************************************************
* num_digits
*
* - Helper function
* - Gives the number of digits required to represent an unsigned integer in
* base 10
*
* Parameters :
* - input : the number in question
*
* Returns :
* - size_t : the required quantity of digits
****************************************************************************/
inline size_t num_digits(uint_fast16_t input)
{
	return input == 0 ? 1 : (size_t)std::log10(input) + 1;
}


