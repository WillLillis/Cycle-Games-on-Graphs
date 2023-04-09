/*
* 
* Just going to throw all the random helper functions in here that don't logically
* fit within the other files
* 
*/
#pragma once
#include <vector>
#include <cstdarg>
#include <cmath>
#include <stdio.h>

/*
*
* Have to have the following #define mess because __FUNCSIG__ isn't valid 
* outside of Microsoft's compiler...
* 
*/

// After a bunch of digging online I finally found this:
// https://stackoverflow.com/questions/2989810/which-cross-platform-preprocessor-defines-win32-or-win32-or-win32 
// which linked this:
// https://web.archive.org/web/20140625123200/http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros

// It seems reasonable to only worry about the Miscrosoft, GCC, and CLANG compilers,
// but if we need to add more later on, then we can do so
// Since we're using Microsoft's __FUNCSIG__ by default, we really only
// need to check for CLANG AND GCC when WIN32 isn't defined
	// WIN32 or _WIN32?-> WIN32 for the compiler, _WIN32 for the OS?
// Add some #pragma message()'s here to inform person building what macro is being used?

#if !defined(_WIN32)
	#if defined(__clang__) || defined(__GNUC__)
		#pragma message "Redefining __FUNCSIG__ to __PRETTY_FUNCTION__"
		#define __FUNCSIG__ __PRETTY_FUNCTION__
	#else
		#pragma message "Redefining __FUNCSIG__ to __func__"
		#define __FUNCSIG__ __func__ // sources online say __FUNCTION__ should work pretty much everywhere, but __func__ is more standardized
	#endif // __clang__ || __GNUC__
#endif // !WIN32

// going to define the below function-like macro expression to ease the use of the display_error function
// someone editing the code can skip passing in __FILE__, _LINE__, __FUNCSIG__ as the first args to display_error...
// ...and instead simply type DISPLAY_ERR(user_clear, err_msg, ...)
#define DISPLAY_ERR(user_clear, err_msg, ...) display_error(__FILE__, __LINE__, __FUNCSIG__, user_clear, err_msg  __VA_OPT__(,) __VA_ARGS__)

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
* using the __FUNCSIG__ macro (or by whatever it's being used as an alias for, 
* depending on the compiler)
* - user_clear : indicates whether the user will have to provide an input in order
* to clear/ continue past the error
* - err_msg : format string for the user's custom error message
* - ... : variable number of optional arguments corresponding to the format string
*
* Returns :
* - none 
****************************************************************************/
void display_error(const char* __restrict file_name, const int line_num, 
	const char* __restrict func_sig, const bool user_clear, const char* __restrict err_msg, ...)
{
	printf("ERROR: ");

	va_list arg_ptr;
	va_start(arg_ptr, err_msg);
	vprintf(err_msg, arg_ptr);
	va_end(arg_ptr);

	printf("\n\t[FILE] %s\n", file_name);
	printf("\t[LINE] %d\n", line_num);
	printf("\t[FUNC] %s\n", func_sig);

	if (user_clear) {
		printf("Press [ENTER] to continue...\n");
		char throw_away = std::getchar();
	}
}

/****************************************************************************
* clear_screen
*
* - Clears the console's screen by calling the relevant OS's clear screen 
* command
*
* Parameters :
* - none
*
* Returns :
* - none
****************************************************************************/
// combine with erase_lines function, allow some special argument to specify erasing all lines
inline void clear_screen()
{
#if defined(_WIN32) || defined(_WIN64)
	system("cls"); // error checking needed here?
#elif  defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
	int err;
	if ((err = system("clear")) != 0) {
		DISPLAY_ERR(false, "Failed to clear screen. Error code returned from \"system\" call: %d.", err);
	}
#else
	DISPLAY_ERR(false,
		"Failed to clear the screen. Unable to identify operating system in use.");
#endif // _WIN32 OR _WIN64
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
bool is_number(const std::string input)
{
	bool non_empty = false;
	for (char c : input) { 
		non_empty = true;
		if (!std::isdigit(c)) {
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
void erase_lines(const uint_fast16_t num_lines)
{
	if (num_lines > 0) {
		printf("\x1b[2K"); // Delete current line

		for (uint_fast16_t line = 1; line < num_lines; line++) { // line = 1 because we included the first line
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
inline size_t index_translation(const uint_fast16_t num_cols, const uint_fast16_t row, const uint_fast16_t col)
{
	return ((size_t)num_cols * (size_t)row) + (size_t)col; // casts necessary?
}

/****************************************************************************
* get_file_length
*
* - Takes in a pointer to an ALREADY OPEN std::fstream and returns the length
* of the file in bytes
* - Any iostream errors/ flags set before the function call will be cleared
* - the position of the file reader will be set back to the beginning 
* after the call
* - https://stackoverflow.com/questions/2409504/using-c-filestreams-fstream-how-can-you-determine-the-size-of-a-file
* - Would like to just use std::filesystem::file_size(), but it reports a 
* different value than this function, and this function is working properly
* as far as I can tell
*	- maybe it's reading in the size from the OS's inode, which might include
*	reserved space as in addition to the space actually used by the file
*	- could bench mark to see if there's a speed difference between the two
*
* Parameters :
* - file : pointer to the fstream in question
*
* Returns :
* - size_t : the length in bytes of the file
****************************************************************************/
// is there a way to save the initial position in the file and later restore it 
// in a way that the gcc compiler doesn't flip out on us?
size_t get_file_length(std::fstream* __restrict file)
{
	if (!(*file).is_open()) {
		DISPLAY_ERR(true, "The supplied file stream is not open.");
		return 0;
	}
	(*file).ignore(std::numeric_limits<std::streamsize>::max());
	size_t file_length = (size_t)(*file).gcount();
	(*file).clear(); // clear the EOF bit set by our going to the end of the file, any other errors
	(*file).seekg(0, std::fstream::beg);
	return file_length;
}

/****************************************************************************
* num_digits
*
* - Helper function
* - Gives the number of digits required to represent an integer in
* base 10
*
* Parameters :
* - input : the number in question
*
* Returns :
* - size_t : the required quantity of digits
****************************************************************************/
template<typename T>
inline size_t num_digits(const T input)
{
	// compile time check because function will only work with ints
	static_assert(!std::is_floating_point_v<T>, "Function template num_digits() only accepts integer types!");
	if (input == 0) {
		return 1;
	} else if (input > 0) {
		return (size_t)std::log10(std::abs((const int64_t)input)) + 1;
	} else {
		return (size_t)std::log10(std::abs((const int64_t)input)) + 2; // negatives need an additional character for the '-' sign
	}
}