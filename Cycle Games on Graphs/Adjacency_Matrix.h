#pragma once
/*
 *
 * This file will mostly hold the code that deals with generating adjacency
 * listings/ matrices and writing them to files, as well as the reading of said
 * files for use by the game playing code
 *
 */

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Misc.h"

/*
 *
 * Because using '\n' as a delimiter breaks compatability for files shared
 *between Windows and Mac machines (due to the difference in endline chars),
 * we'll #define the following delimiter character below. The choice is almost
 *entirely arbitrary, it just
 *	- can't be a number
 *	- can't be '\n'
 *	- there may be other restrictions that we haven't encountered yet
 */
#define ADJ_FILE_DELIM '#' //((char)35) // ASCII character code for '#'

// Maybe a little overkill, but some defensive programming here to limit how
// many elements we'll read in
#define MAX_ELEMENTS 500 // arbitrary max value, feel free to increase if needed

// Used within Cycle_Games.h, but made sense categorically to #define these in
// here
// typedef uint_fast16_t Adjacency_Info;
// #define ADJACENT		1
// #define NOT_ADJACENT	0
enum class Adjacency_Info : uint_fast16_t { ADJACENT, NOT_ADJACENT };

/****************************************************************************
 * dos2unix_repair
 *
 * - Opens the specified file containing an adjacency listing, then creates a
 * copy of it without carriage returns ('\r') so the non-Windows world can use
 * the file
 * - Takes the existing file name and appends "_repaired" to it for the copy's
 *name
 * - Shamelessly taken from
 *https://stackoverflow.com/questions/58279627/reimplementing-dos2unix-and-unix2dos-in-c-r-and-n-not-appearing-in-hexd
 *
 * Parameters :
 * - file_path : std::filesystem::path object holding the path for the desired
 *file
 *
 * Returns :
 * - std::filesystem::path : path to the repaired copy of the file
 ****************************************************************************/
std::filesystem::path dos2unix_repair(const std::filesystem::path file_path) {
    char c;
    std::ifstream is(file_path.c_str());

    std::filesystem::path repair_file_path = file_path;
    repair_file_path.replace_extension("");
    repair_file_path.replace_filename(repair_file_path.filename().string() +
                                      "_repaired");
    repair_file_path.replace_extension(".txt");
    std::ofstream os(repair_file_path.c_str());

    while (is.get(c)) {
        if (c != '\r') [[likely]] {
            os.put(c);
        }
    }

    is.close();
    os.close();
    return repair_file_path;
}

/****************************************************************************
 * load_adjacency_info
 *
 * - Opens the specified file containing an adjacency listing, then allocates
 * the space for and fills a buffer for the adjacency matrix corresponding to
 * the listing in the file
 * - While it would be more straightforward to repeatedly call std::getline as
 * opposed to reading in the entire block all at once, having one large read
 * from the disk should be faster than a bunch of smaller reads
 *
 * Parameters :
 * - file_path : string holding the path to the desired file
 * - num_nodes_out : passed in by reference in order to tell the caller how
 * many nodes the given graph has
 * - success_out : pointer to a bool indicating whether an adjacency matrix was
 * successfully generated to the caller
 * - try_repair : bool indicating whether the function should try creating a
 * "repaired" version of the adjacency info file if certain error conditions are
 * met. Default value is true, but should be set to false for subsequent calls
 *to prevent infinite recursion
 *
 * Returns :
 * - uint_fast16_t* : pointer to a buffer of size
 * (*num_nodes_out) * (*num_nodes_out) holding the adjacency matrix
 ****************************************************************************/
// Need to add ability to read in adjacency matrices?
std::vector<Adjacency_Info>
load_adjacency_info(const std::filesystem::path file_path,
                    uint_fast16_t *__restrict num_nodes_out,
                    bool *__restrict success_out, bool try_repair = true) {
    *success_out = false;
    std::vector<Adjacency_Info> adj_matrix;
    std::fstream data_stream;

    data_stream.open(file_path.string().c_str(),
                     std::fstream::in); // open file with read permissions only
    if (!data_stream.is_open()) [[unlikely]] {
        DISPLAY_ERR(true,
                    "Failed to open the adjacency information file.\nRequested "
                    "path: %s",
                    file_path.string().c_str());
    }

    size_t file_length = get_file_length(&data_stream);

    // allocate a buffer with the length to read all of the file's contents in
    // at once
    if (!(file_length > 0)) [[unlikely]] {
        DISPLAY_ERR(true,
                    "Received an invalid file length.\nRequested path: %s",
                    file_path.string().c_str());
        return adj_matrix;
    }

    std::vector<char> data(file_length);
    data_stream.read(data.data(), file_length);
    data_stream.close();

    // Need to find the largest node number in the data so that we can allocate
    // the buffer for the adjacency matrix
    uint_fast16_t temp_label;
    uint_fast16_t max_label = 0;

    // BUGBUG will need to tweak here if we use the new delimiter after the
    // adjacency heading ?
    size_t data_start = 0;
    while (data_start < file_length &&
           data[data_start] != '\n') { // skip over the labels in the first line
        data_start++;
    }
    if (data_start > 0) {
        data_start++; // increment one more time to get past that first newline
                      // character
    } else [[unlikely]] { // otherwise there was no file heading, indicating
                          // some sort of error with the file/ how we read it,
                          // return an error
        DISPLAY_ERR(true,
                    "File parsing error. Adjacency type header not found. File "
                    "path: %s",
                    file_path.string().c_str());
        return adj_matrix;
    }

    size_t curr = data_start;

    if (!(curr < file_length)) [[unlikely]] {
        DISPLAY_ERR(true,
                    "File parsing error. End of file read into memory "
                    "unexpectedly reached. File path: %s",
                    file_path.string().c_str());
        return adj_matrix;
    }

    // loop to find the max label in the adjacency listing file
    while (curr < file_length) {
        if (std::isdigit(data[curr])) [[likely]] {
            temp_label = std::atoi(&data[curr]);
            max_label = std::max(temp_label, max_label);
            curr += num_digits(temp_label) +
                    1; // advance to the next character, and then one more to
                       // skip a comma/ newline character
            continue;  // back to the top of the loop
        } else [[unlikely]] { // something went wrong
            if (try_repair == true) [[likely]] {
                DISPLAY_ERR(false, "Error parsing the file searching for the "
                                   "largest node label...Attempting to create "
                                   "a compatible version of the file now.");
                std::filesystem::path repaired_path =
                    dos2unix_repair(file_path); // if it's a line ending issue,
                                                // we can try to fix it
                adj_matrix =
                    load_adjacency_info(repaired_path, num_nodes_out,
                                        success_out, try_repair = false);
            } else [[unlikely]] {
                DISPLAY_ERR(
                    true,
                    "Error parsing the file searching for the largest node "
                    "label...Non-recoverable. Try regenerating the file.");
            }
            return adj_matrix;
        }
    }

    max_label++; // have to account for 0 based counting with the node labels
    *num_nodes_out = max_label;

    // now that we have the max label, we know the size of the adjacency matrix
    adj_matrix.assign((size_t)max_label * (size_t)max_label,
                      Adjacency_Info::NOT_ADJACENT);

    uint_fast16_t node_1,
        node_2; // to temporarily store node values read in from the data block
    bool second_node =
        false; // to track which node (first or second on a given line) we're on

    curr = data_start;
    if (!(curr < file_length)) [[unlikely]] {
        DISPLAY_ERR(true,
                    "Issue parsing the adjacency information file loaded into "
                    "memory.\nReached the end of the file earlier than "
                    "expected.\nFile path: %s",
                    file_path.string().c_str());
        return adj_matrix;
    }
    while (curr < file_length) {
        if (std::isdigit(data[curr])) [[likely]] {
            if (!second_node) { // reading in the first digit in a pair
                node_1 = std::atoi(&data[curr]);
                curr += num_digits(node_1); // advance to the first char past
                                            // that of the number's
                // we shouldn't be at the end of the file, AND the next
                // character has to be a comma
                if (!(curr < file_length && data[curr] == ',')) {
                    DISPLAY_ERR(true,
                                "Issue parsing the adjacency information file "
                                "loaded into memory.\nDidn't encounter a comma "
                                "when one was expected, or EOF was encountered "
                                "unexpectedly.\nFile path: %s",
                                file_path.string().c_str());
                    return adj_matrix;
                }
                curr++; // advance one more character to skip the comma
                        // separating the two digits
                second_node = true;
                continue;
            } else { // reading in the second digit
                node_2 = std::atoi(&data[curr]);
                curr += num_digits(node_2); // advance to the first char past
                                            // that of the number's
                if (curr <
                    file_length) { // if we're not at the end of the file...
                    // extra OR added to allow for the continued use of old
                    // files with the '\n' delimiter, should be fine to leave
                    // this here
                    if (!(data[curr] == ADJ_FILE_DELIM ||
                          data[curr] ==
                              '\n')) { // ...there should be a newline (or our
                                       // updated delimiter) character next
                        DISPLAY_ERR(
                            true,
                            "Issue parsing the adjacency information file "
                            "loaded into memory.\nDidn't encounter a delimiter "
                            "when one was expected.\nFile path: %s",
                            file_path.string().c_str());
                        return adj_matrix;
                    }
                }
                curr++; // advance one more character to skip the newline char
                adj_matrix[index_translation(max_label, node_1, node_2)] =
                    Adjacency_Info::ADJACENT; // mark the appropriate entries in
                                              // the adjacency matrix
                adj_matrix[index_translation(max_label, node_2, node_1)] =
                    Adjacency_Info::ADJACENT; // ^
                second_node = false;
                continue;
            }
        } else [[unlikely]] { // something went wrong, return the empty vector
                              // but don't set the success_out flag
            DISPLAY_ERR(
                true,
                "Issue parsing the adjacency information file loaded into "
                "memory.\nUnspecified parsing error.\nFile path: %s",
                file_path.string().c_str());
            return adj_matrix;
        }
    }

    *success_out = true;
    return adj_matrix;
}

// Generating functions
// do we want to change these so that they dump all their info to a buffer,
// then write it all at once?-> won't matter as much as the reads because write
// buffering is a thing but may still be the best practice would need to develop
// means to calculating the required buffer size beforehand.... this isn't
// something that's getting called repeatedly, delay likely not to be an issue
/****************************************************************************
 * generalized_petersen_gen
 *
 * - Writes an adjacency listing for a generalized petersen graph of the
 * specified parameters to the file stream
 * - https://mathworld.wolfram.com/GeneralizedPetersenGraph.html
 *
 * Parameters :
 * - output : file stream to write the adjacency listing to
 * - n : graph parameter
 *	- number of nodes on a ring in the graph
 * - k : graph parameter
 *	- how many nodes away the inner rings are connected
 *
 * Returns :
 * - none
 ****************************************************************************/
void generalized_petersen_gen(FILE *__restrict output, const uint_fast16_t n,
                              const uint_fast16_t k) {
    if (output == NULL) [[unlikely]] {
        DISPLAY_ERR(true, "Invalid file stream.");
        return;
    }
    fprintf(output, "Adjacency_Listing\n"); // label at the top of the file so
                                            // we know what it is

    // outer ring connections to adjacent nodes around the ring
    for (uint_fast16_t i = 0; i < n; i++) {
        fprintf(output, "%hu,%hu%c", (uint16_t)i, (uint16_t)((i + 1) % n),
                ADJ_FILE_DELIM);
    }
    // outer ring to inner ring "spokes"
    for (uint_fast16_t i = 0; i < n; i++) {
        fprintf(output, "%hu,%hu%c", (uint16_t)i, (uint16_t)(i + n),
                ADJ_FILE_DELIM);
    }
    // inner ring connections (ew)
    for (uint_fast16_t i = n; i < (2 * n) - 1; i++) {
        fprintf(output, "%hu,%hu%c", (uint16_t)i, (uint16_t)(((i + k) % n) + n),
                ADJ_FILE_DELIM);
    }
    // also an inner ring connection
    fprintf(output, "%hu,%hu", (uint16_t)((2 * n) - 1),
            (uint16_t)(((((2 * n) - 1) + k) % n) +
                       n)); // last entry does NOT get a newline char after it
}

/****************************************************************************
 * stacked_prism_gen
 *
 * - Writes an adjacency listing for a stacked prism graph of the
 * specified parameters to the file stream
 * - https://mathworld.wolfram.com/StackedPrismGraph.html
 *
 * Parameters :
 * - output : file stream to write the adjacency listing to
 * - n : graph parameter
 *	- number of nodes on a ring in the graph
 * - m : graph parameter
 *	- how many stacked prisms the graph has
 *
 * Returns :
 * - none
 ****************************************************************************/
void stacked_prism_gen(FILE *__restrict output, const uint_fast16_t m,
                       const uint_fast16_t n) {
    if (output == NULL) [[unlikely]] {
        DISPLAY_ERR(true, "Invalid file stream.");
        return;
    }
    fprintf(output, "Adjacency_Listing\n"); // label at the top of the file so
                                            // we know what it is

    for (uint_fast16_t i = 0; i < n; i++) {
        for (uint_fast16_t j = 0; j < m - 1; j++) {
            fprintf(output, "%hu,%hu%c", (uint16_t)(j + (i * m)),
                    (uint16_t)(j + 1 + (i * m)), ADJ_FILE_DELIM);
        }
        fprintf(output, "%hu,%hu", (uint16_t)(m - 1 + (i * m)),
                (uint16_t)(i * m));
        if (i != (n - 1)) { // doing this to avoid placing a newline char at the
                            // end of the file
            fprintf(output, "%c", ADJ_FILE_DELIM);
        }
        if (i < n - 1) {
            for (uint_fast16_t k = 0; k < m; k++) {
                fprintf(output, "%hu,%hu%c", (uint16_t)(k + (i * m)),
                        (uint16_t)(k + ((i + 1) * m)), ADJ_FILE_DELIM);
            }
        }
    }
}

/*
 *
 * - A brief discussion before the Z_m^n generation code
 * - This is a bit gross. I remember talking with Dr. Gates
 * regarding a good way to generate all the members of a given Z_m^n
 * group, and the recursive solution is what we came up with
 * - An iterative solution would likely be much clearer and vastly preferred,
 *but for now this works and we're moving on
 * - The original implementation of this used std::vectors and a crappy little
 * tuple template class I wrote
 *	- While it may make sense to switch back to using the template class later,
 *for now I'll just go with the approach below
 *
 * - I decided to do it with a single dimensional array, where the tuples are
 *just laid out next to eachother, and it's more reliant on the function using
 *this stored info to know what's what
 *
 */

/****************************************************************************
 * tuple_to_index
 *
 * - Given an array holding some number of tuples as adjacent entries,
 * calculates the index in said array for the specified entry within the
 * specified tuple
 *
 * Parameters :
 * - n : number of entries per tuple
 * - tuple_num : which tuple we're accessing within the array of tuples
 * - tuple_entry : which entry we're accessing within said tuple
 *
 * Returns :
 * - uint_fast16_t : the index in the array for said entry within the specified
 * tuple
 ****************************************************************************/
inline size_t tuple_to_index(const uint_fast32_t n,
                             const uint_fast32_t tuple_num,
                             const uint_fast32_t tuple_entry) {
    return (size_t)((n * tuple_num) + tuple_entry);
}

/****************************************************************************
 * tupls_adj
 *
 * - Determines whether two tuples are adjacent
 *   - tuples are adjacent IFF they differ by 1 mod M in exactly one coordiante
 * - The two tuples MUST be of the same length
 *
 * Parameters :
 * - tuple_holder : pointer to an array holding the values of all tuples
 * - start_index_1 : index of the first entry of the first tuple in tuple_holder
 * - start_index_2 : index of the first entry of the second tuple in
 *tuple_holder
 * - m : graph parameter
 *	- corresponds to m in Z_m^n
 * - num_entries : the number of entries in the two tuples
 *	- corresponds to n in Z_m^n
 *
 * Returns :
 * - bool : whether tuple_1 and tuple_2 are adjacent
 ****************************************************************************/
bool tuples_adj(const std::vector<uint_fast16_t> &__restrict tuple_holder,
                const size_t start_index_1, const size_t start_index_2,
                const uint_fast16_t m, const uint_fast16_t num_entries) {
    bool found_coord = false;

    for (uint_fast16_t entry = 0; entry < num_entries; entry++) {
        uint_fast16_t entry_1 = tuple_holder[start_index_1 + entry];
        uint_fast16_t entry_2 = tuple_holder[start_index_2 + entry];

        int32_t diff = (int32_t)entry_1 - (int32_t)entry_2;
        if (diff == 0) { // coordinates match, continue iterating
            continue;
        } else if (std::abs((diff) % (int32_t)m) == 1 ||
                   (entry_1 + 1) % m == entry_2 ||
                   (entry_2 + 1) % m == entry_1) { // coordinates differ by one
            if (found_coord) { // more than one coordinate differs by one, not
                               // adjacent
                return false;
            }
            found_coord =
                true; // first coordinate to differ by one, keep iterating
        } else { // distance in coordinates is greater than one, not adjacent
            return false;
        }
    }

    return found_coord;
}

/****************************************************************************
 * z_mn_group_gen
 *
 * - Helper function for z_mn_gen
 * - Generates all the entries of a given Z_m^n group and places them in the
 * passed in array
 *
 * Parameters :
 * - member_list : array of size n * m^n that holds every member of a given
 * Z_m^n group
 *	- passed in by reference, so once it's filled, the caller has access
 * - value_holder : temporary array that holds the "current value" of the
 * tuple being generated at various points within the tuple
 *	- passed by reference so its value persists throughout recursive calls,
 *	contents mean nothing to the caller
 * - place_in_tuple : the current point in the n-tuple being filled
 * - place_in_member_list : holds the current entry the the member_list array
 * for the current tuple to be entered
 *	 - holds the tuple number (in terms of the first tuple being generated,
 * the second, third, and so on....), not an actual address/ offset in bytes
 *	- passed by reference so its value persists throughout recursive calls,
 *	contents mean nothing to the caller
 * - m : graph parameter
 *	- each tuple entry can range from 0 to m-1
 * - n : graph parameter
 *	- the number of entries in each tuple
 *
 * Returns :
 * - none
 ****************************************************************************/
void z_mn_group_gen(std::vector<uint_fast16_t> &__restrict member_list,
                    std::vector<uint_fast16_t> &__restrict value_holder,
                    const uint_fast16_t place_in_tuple,
                    uint_fast16_t *__restrict place_in_member_list,
                    const uint_fast16_t m, const uint_fast16_t n) {
    if (place_in_tuple < (n - 1)) {
        for (uint_fast16_t i = 0; i < m; i++) {
            value_holder[place_in_tuple] = i;
            z_mn_group_gen(member_list, value_holder, place_in_tuple + 1,
                           place_in_member_list, m, n);
        }
    } else {
        for (uint_fast16_t i = 0; i < m; i++) {
            value_holder[place_in_tuple] =
                i; // replace place_in_tuple with n-1 here?

            for (uint_fast16_t j = 0; j < n; j++) {
                // set the jth entry of the place_in_member_list tuple with the
                // value of value_holder[j]
                member_list[tuple_to_index(n, *place_in_member_list, j)] =
                    value_holder[j];
            }
            *place_in_member_list +=
                1; // move to the next tuple in our member_list array
        }
    }
}

/****************************************************************************
 * z_mn_gen
 *
 * - Calls z_mn_group_gen() to generate all members of a given Z_m^n group,
 * and then writes said group's adjacenecy listing to the supplied file stream
 *
 * Parameters :
 * - output : the file stream to write the adjacency listing to
 *	- passed in by reference, so once it's filled, the caller has access
 * - m : graph parameter
 *	- each tuple entry can range from 0 to m-1
 * - n : graph parameter
 *	- the number of entries in each tuple
 *
 * Returns :
 * - bool : true to indicate success, false to indicate failure
 ****************************************************************************/
// check if casts are needed here...
bool z_mn_gen(FILE *__restrict output, const uint_fast16_t m,
              const uint_fast16_t n) {
    if (output == NULL) [[unlikely]] {
        DISPLAY_ERR(true, "Invalid file stream.");
        return false;
    }

    uint_fast16_t num_tuples = (uint16_t)std::pow(m, n);
    std::vector<uint_fast16_t> value_holder(n);
    std::vector<uint_fast16_t> member_list(num_tuples * n);
    uint_fast16_t place_in_member_list = 0;
    bool has_entry = false;

    // common sense checks here to make sure vector allocated all of the
    // memory...
    if (num_tuples == 0 ||
        (member_list.size() != ((size_t)num_tuples * (size_t)n))) [[unlikely]] {
        DISPLAY_ERR(true,
                    "Failed to allocate the necessary memory to generate the "
                    "adjacency listing! Attempted to allocatte %zu bytes.",
                    (size_t)(std::pow(m, n) * n));
        return false;
    }

    z_mn_group_gen(member_list, value_holder, 0, &place_in_member_list, m, n);

    // output to file
    fprintf(output, "Adjacency_Listing\n");
    for (uint_fast16_t i = 0; i < num_tuples; i++) {
        for (uint_fast16_t j = i; j < num_tuples; j++) {
            if (tuples_adj(member_list, (size_t)i * (size_t)n,
                           (size_t)j * (size_t)n, m, n)) {
                if (has_entry == true) { // doing this to avoid a newline
                                         // character at the end of the file
                    fprintf(output, "%c%hu,%hu", ADJ_FILE_DELIM, (uint16_t)i,
                            (uint16_t)j);
                } else {
                    fprintf(output, "%hu,%hu", (uint16_t)i, (uint16_t)j);
                }
                has_entry = true;
            }
        }
    }

    return true;
}
