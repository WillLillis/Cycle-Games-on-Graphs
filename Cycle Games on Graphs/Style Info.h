#pragma once

/*
*
* While it's not super important, I figured it might be helpful to put some 
* kind of style information in a header as I go to keep the project somewhat
* consistent
* 
*/

/*
*
* All functions should have the following comment "header" directly above their definitions
* If there are no parameters or return type, the respective entries can be filled with " - none"
* 
*/ 
/****************************************************************************
* <Function Name>
*
* - <Brief description of what it does>
* - <Brief description of how it operates if you feel that would be helpful
* and/ or needed>
* - <Comments on edge behavior, error checking/ reporting, etc.>
*
* Parameters :
* - <param1> : <description of what param1 is/ what it's used for>
* - <param2> : "                                                 "
* - etc.
*
* Returns :
* - <return type> : <description of what the function returns
****************************************************************************/


/*
*  Going to try to stick with snake_case for variable and function names, although 
* #define's will get SCREAMING_SNAKE_CASE because it looks cool
*/

/*
*
* Throughout the code I make use of the cstdint types, i.e. uint16_t
* In digging through the documentation a bit, I also found "fast" types, such as
* uint_fast16_t, which provides an unsigned integer of width at least 16 bits, that is
* the fastest on the specified architecture
* 
* Although it looks a bit odd to my eyes (to start at least), it seems like the right choice
* to use these types, as we may get a bit of a performance boost out of them
* 
* However some uses of uint_fastx_t types generate some warnings on some compilers, so casts
* may be necessary to maintain a quiet-ish compilation
*
*/