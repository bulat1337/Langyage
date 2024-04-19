#ifndef ASSEMBLER_ADDITIONAL
#define ASSEMBLER_ADDITIONAL

/**
 * @file assembler_additional.h
 * @brief Header file containing additional functions and structures for the assembler program.
 */

#include "assembler.h"
#include "../../../../File_parser/include/file_parser.h"
#include "utils.h"
#include "secondary.h"

/**
 * @brief Macro to log messages to a file.
 *
 * This macro simplifies logging by automatically passing file name, function name, and line number.
 * Usage: LOG("Message to log");
 */
#define LOG(...)\
	asm_write_log(__FILE__, __func__, __LINE__, __VA_ARGS__);

/**
 * @brief Logs a message to a file.
 *
 * Logs a message to a file named "log.txt" with the specified format and additional information.
 *
 * @param file_name The name of the file where the log message originates.
 * @param func_name The name of the function where the log message originates.
 * @param line The line number in the source file where the log message originates.
 * @param fmt The format string for the log message.
 * @param ... Additional arguments to be formatted according to the format string.
 */
void asm_write_log(const char *file_name, const char *func_name, int line, const char *fmt, ...);

#define FREAD_CHECK(read_elems, amount)								\
	if(read_elems != amount)										\
	{																\
		LOG("ERROR: fread read unexpected amount of elems.\n");		\
		LOG("\t expected amount: %lu.\n", amount);					\
		LOG("\t read amount: %lu.\n", read_elems);					\
																	\
		return ASM_INVALID_FREAD;									\
	}


#define FWRITE_CHECK(written_elems, amount)								\
	if(written_elems != amount)											\
	{																	\
		LOG("ERROR: fwrite wrote unexpected amount of elems.\n");		\
		LOG("\t expected amount: %lu.\n", amount);						\
		LOG("\t written amount: %lu.\n", written_elems);				\
																		\
		return ASM_INVALID_FWRITE;										\
	}

/**
 * @struct Label
 * @brief Structure representing a label with its name and position.
 */
struct Label
{
    char  *name; /**< Name of the label. */
    size_t IP_pos; /**< Position of the label in the bytecode. */
};

/**
 * @struct Labels_w_carriage
 * @brief Structure representing labels along with their carriage position.
 */
struct Labels_w_carriage
{
    Label *labels; /**< Array of labels. */
    size_t carriage; /**< Carriage position of the labels. */
};

/**
 * @struct JMP_pos
 * @brief Structure representing a jump with its name and position.
 */
struct JMP_pos
{
    char const *name; /**< Name of the jump. */
    int         IP_pos; /**< Position of the jump in the bytecode. */
};

/**
 * @struct JMP_poses_w_carriage
 * @brief Structure representing jumps along with their carriage position.
 */
struct JMP_poses_w_carriage
{
    JMP_pos *JMP_poses; /**< Array of jumps. */
    size_t   carriage; /**< Carriage position of the jumps. */
};

struct Compile_manager
{
    Strings       strings;
	Buffer_w_info        human_code_buffer; /**< Buffer with length information for human-readable code. */
	Labels_w_carriage    labels_w_carriage; /**< Structure containing labels with carriage information. */
    JMP_poses_w_carriage jmp_poses_w_carriage; /**< Structure containing jumps with carriage information. */
	Buffer_w_info        byte_code; /**< Buffer with length information for bytecode. */
	char                *byte_code_start; /**< Start of the byte code buffer. */
};

const int           POISON_JMP_POS                 = -1;
const char          IDENTIFIER_BYTE                =  1;
const int           CMD_TYPE_ALIGNMENT_VALUE       =  3;
const size_t        ALIGN_TO_INT                   =  sizeof(int)    - sizeof(char);
const size_t        ALIGN_TO_DOUBLE                =  sizeof(double) - sizeof(char);
const char * const  MAIN_JMP_NAME                  = "main";
const unsigned char SPACE_SKIP                     = 1;
const unsigned char LETTER_SKIP                    = 1;
const unsigned char SIX_BYTE_ALIGNMENT             = 6;
const unsigned char ONE_BYTE_ALIGNMENT             = 1;
const unsigned char TWO_BYTE_ALIGNMENT             = 2;
const unsigned char B_CODE_SIZE_COEFF              = 2;

/**
 * @brief Parses the human-readable assembly code and prepares it for compilation.
 *
 * This function reads the human-readable assembly code from a file,
 * allocates memory for the buffer, and arranges the tokens.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @param file_name Name of the file containing human-readable assembly code.
 * @return Error code indicating the status of the function.
 */
asm_err_t parse_human_code(Compile_manager *manager, const char *file_name);

/**
 * @brief Processes the assembly commands and generates the byte code.
 *
 * This function processes the human-readable assembly commands
 * and generates the corresponding byte code.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Error code indicating the status of the function.
 */
asm_err_t cmds_process(Compile_manager *manager);

/**
 * @brief Writes main jump instruction.
 *
 * Writes the main jump instruction to the bytecode.
 *
 * @param byte_code Pointer to the bytecode buffer and length.
 * @param jmp_poses_w_carriage Pointer to the structure containing jump positions.
 * @return Error code indicating the success or failure of the operation.
 */
asm_err_t write_main_jmp(struct Buffer_w_info *byte_code,
					   JMP_poses_w_carriage *jmp_poses_w_carriage);

/**
 * @brief Writes data of a certain size to the buffer.
 *
 * @param byte_code Pointer to Buffer_w_infon structure representing buffer and its length.
 * @param value Pointer to the data to be written.
 * @param size Size of the data in bytes.
 * @return asm_err_t Error code indicating the success of the operation.
 */
asm_err_t write_to_buf(struct Buffer_w_info *byte_code,
                     const void *value, size_t size);

/**
 * @brief Adds alignment to a buffer by adding "void" bytes equivalent to zero.
 *
 * @param buf Pointer to Buffer_w_info structure representing buffer and its length.
 * @param amount_of_bytes Number of bytes to add.
 * @return asm_err_t Error code indicating the success of the operation.
 */
asm_err_t align_buffer(struct Buffer_w_info *buf, size_t amount_of_bytes);

/**
 * @brief Writes a character to the buffer with alignment.
 *
 * @param byte_code Pointer to Buffer_w_info structure representing buffer with its length.
 * @param value Character to be written.
 * @param alignment_space Number of alignment bytes.
 * @return asm_err_t Error code indicating the success of the operation.
 */
asm_err_t write_char_w_alignment(struct Buffer_w_info *byte_code,
                               char value, size_t alignment_space);

/**
 * @brief Logs labels.
 *
 * Logs the labels along with their IDs and positions.
 *
 * @param labels_w_carriage Pointer to the structure containing labels.
 * @return Error code indicating the success or failure of the operation.
 */
asm_err_t log_labels(struct Labels_w_carriage *labels_w_carriage);

/**
 * @brief Logs jumps.
 *
 * Logs the jumps along with their IDs and positions.
 *
 * @param jmp_poses_w_carriage Pointer to the structure containing jumps.
 * @return Error code indicating the success or failure of the operation.
 */
asm_err_t log_jmps(struct JMP_poses_w_carriage *jmp_poses_w_carriage);

/**
 * @brief Arranges the labels in the byte code.
 *
 * This function arranges the labels in the byte code based on the jump positions.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Error code indicating the status of the function.
 */
asm_err_t arrange_labels(Compile_manager *manager);

/**
 * @brief Reduces the size of the byte code buffer.
 *
 * This function reduces the size of the byte code buffer by removing trailing zeros.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Error code indicating the status of the function.
 */
asm_err_t reduce_buffer_size(Compile_manager *manager);

/**
 * @brief Creates a binary file containing the byte code.
 *
 * This function creates a binary file containing the generated byte code.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @param file_name Name of the binary file to be created.
 * @return Error code indicating the status of the function.
 */
asm_err_t create_bin(Compile_manager *manager, const char *file_name);                                              //  хуй 

/**
 * @brief Destructor for the Compile_manager structure.
 *
 * This function frees the allocated memory in the Compile_manager structure.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Error code indicating the status of the function.
 */
asm_err_t manager_dtor(Compile_manager *manager);

/**
 * @brief Masks the byte code buffer.
 *
 * This function masks the byte code buffer with the specified mask.
 *
 * @param byte_code Pointer to the byte code buffer.
 * @param mask Mask value to be applied.
 * @return Error code indicating the status of the function.
 */
asm_err_t mask_buffer(Buffer_w_info *byte_code, const char mask);

/**
 * @brief Initializes the Compile_manager structure.
 *
 * This function initializes the Compile_manager structure.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Error code indicating the status of the function.
 */
asm_err_t init_manager(Compile_manager *manager);

/**
 * @brief Gets the current instruction pointer position.
 *
 * This function gets the current instruction pointer position based on the byte code buffer.
 *
 * @param manager Pointer to the Compile_manager structure.
 * @return Current instruction pointer position.
 */
size_t get_ip_pos(Compile_manager *manager);

#endif
