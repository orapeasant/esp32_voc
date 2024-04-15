#line 1 "/home/ubuntu/proj/ei-projects/esp32-examples/ei-mbed-qcbor-http-acc/src/qcbor/UsefulBuf.h"
/*============================================================================
 Copyright (c) 2016-2018, The Linux Foundation.
 Copyright (c) 2018-2019, Laurence Lundblade.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors, nor the name "Laurence Lundblade" may be used to
      endorse or promote products derived from this software without
      specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =============================================================================*/

/*============================================================================
 FILE:  UsefulBuf.h

 DESCRIPTION:  General purpose input and output buffers

 EDIT HISTORY FOR FILE:

 This section contains comments describing changes made to the module.
 Notice that changes are listed in reverse chronological order.

 when         who             what, where, why
 --------     ----            --------------------------------------------------
 5/21/2019    llundblade      #define configs for efficient endianness handling.
 5/16/2019    llundblade      Add UsefulOutBuf_IsBufferNULL().
 3/23/2019    llundblade      Big documentation & style update. No interface
                              change.
 3/6/2019     llundblade      Add UsefulBuf_IsValue()
 12/17/2018   llundblade      Remove const from UsefulBuf and UsefulBufC .len
 12/13/2018   llundblade      Documentation improvements
 09/18/2018   llundblade      Cleaner distinction between UsefulBuf and
                              UsefulBufC.
 02/02/18     llundbla        Full support for integers in and out; fix pointer
                              alignment bug. Incompatible change: integers
                              in/out are now in network byte order.
 08/12/17     llundbla        Added UsefulOutBuf_AtStart and UsefulBuf_Find
 06/27/17     llundbla        Fix UsefulBuf_Compare() bug. Only affected
                              comparison for < or > for unequal length buffers.
                              Added UsefulBuf_Set() function.
 05/30/17     llundbla        Functions for NULL UsefulBufs and const / unconst
 11/13/16     llundbla        Initial Version.

 =============================================================================*/

#ifndef _UsefulBuf_h
#define _UsefulBuf_h


/*
 Configuration Options

 This code is designed so it will work correctly and completely by
 default. No configuration is necessary to make it work. None of the
 following #defines need to be enabled. The code works and is very
 portable with them all turned off.

 All configuration options (USEFULBUF_CONFIG_XXX)
    1) Reduce code size
    2) Improve efficiency
    3) Both of the above

 The efficiency improvements are not large, so the main reason really
 is to reduce code size.

 */


/*
 Endianness Configuration

 By default, UsefulBuf does not need to know what the endianness of
 the device is. All the code will run correctly on either big or
 little endian CPUs.

 Here's the recipe for configuring the endianness-related #defines
 to use more efficient CPU/OS/compiler dependent features to reduce
 code size. Note these only affect the integer arrays (tagged
 arrays) feature of QCBOR. All other endianness handling in
 QCBOR is integrated with code that also handles alignment and
 preferred encoding.

 The first option is to not define anything. This will work fine on
 with all CPU's, OS's and compilers. The code for encoding
 integers will be a little larger and slower.

 If your CPU is big-endian then define USEFULBUF_CONFIG_BIG_ENDIAN. This
 will give the most efficient code for big-endian CPUs. It will be small
 and efficient because there will be no byte swapping.

 Try defining USEFULBUF_CONFIG_HTON. This will work on most CPU's,
 OS's and compilers, but not all. On big-endian CPUs this should give
 the most efficient code, the same as USEFULBUF_CONFIG_BIG_ENDIAN
 does. On little-endian CPUs it should call the system-defined byte
 swapping method which is presumably implemented efficiently. In some
 cases, this will be a dedicated byte swap instruction like Intel's
 bswap.

 If USEFULBUF_CONFIG_HTON works and you know your CPU is
 little-endian, it is also good to define
 USEFULBUF_CONFIG_LITTLE_ENDIAN.

 if USEFULBUF_CONFIG_HTON doesn't work and you know your system is
 little-endian, try defining both USEFULBUF_CONFIG_LITTLE_ENDIAN and
 USEFULBUF_CONFIG_BSWAP. This should call the most efficient
 system-defined byte swap method. However, note
 https://hardwarebug.org/2010/01/14/beware-the-builtins/.  Perhaps
 this is fixed now. Often hton() and ntoh() will call the built-in
 __builtin_bswapXX()() function, so this size issue could affect
 USEFULBUF_CONFIG_HTON.

 Last, run the tests. They must all pass.

 These #define config options affect the inline implementation of
 UsefulOutBuf_InsertUint64() and UsefulInputBuf_GetUint64().  They
 also affect the 16-, 32-bit, float and double versions of these
 instructions. Since they are inline, they size effect is not in the
 UsefulBuf object code, but in the calling code.

 Summary:
   USEFULBUF_CONFIG_BIG_ENDIAN -- Force configuration to big-endian.
   USEFULBUF_CONFIG_LITTLE_ENDIAN -- Force to little-endian.
   USEFULBUF_CONFIG_HTON -- Use hton(), htonl(), ntohl()... to
     handle big and little-endian with system option.
   USEFULBUF_CONFIG_BSWAP -- With USEFULBUF_CONFIG_LITTLE_ENDIAN,
     use __builtin_bswapXX().
 */

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN) && defined(USEFULBUF_CONFIG_LITTLE_ENDIAN)
#error "Cannot define both USEFULBUF_CONFIG_BIG_ENDIAN and USEFULBUF_CONFIG_LITTLE_ENDIAN"
#endif


#include <stdint.h> // for uint8_t, uint16_t....
#include <string.h> // for strlen, memcpy, memmove, memset
#include <stddef.h> // for size_t


#ifdef USEFULBUF_CONFIG_HTON
#include <arpa/inet.h> // for htons, htonl, htonll, ntohs...
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 @file UsefulBuf.h

 The goal of this code is to make buffer and pointer manipulation
 easier and safer when working with binary data.

 The @ref UsefulBuf, @ref UsefulOutBuf and @ref UsefulInputBuf
 structures are used to represent buffers rather than ad hoc pointers and
 lengths.

 With these it will often be possible to write code that does little
 or no direct pointer manipulation for copying and formatting
 data. For example, the QCBOR encoder was written using these and
 has no less pointer manipulation.

 While it is true that object code using these functions will be a
 little larger and slower than a white-knuckle clever use of pointers
 might be, but not by that much or enough to have an effect for most
 use cases. For security-oriented code this is highly
 worthwhile. Clarity, simplicity, reviewability and are more
 important.

 There are some extra sanity and double checks in this code to help
 catch coding errors and simple memory corruption. They are helpful,
 but not a substitute for proper code review, input validation and
 such.

 This code consists of a lot of inline functions and a few that are
 not.  It should not generate very much object code, especially with
 the optimizer turned up to @c -Os or @c -O3.
 */


/**
 @ref UsefulBufC and @ref UsefulBuf are simple data structures to hold
 a pointer and length for binary data.  In C99 this data structure can
 be passed on the stack making a lot of code cleaner than carrying
 around a pointer and length as two parameters.

 This is also conducive to secure coding practice as the length is
 always carried with the pointer and the convention for handling a
 pointer and a length is clear.

 While it might be possible to write buffer and pointer code more
 efficiently in some use cases, the thought is that unless there is an
 extreme need for performance (e.g., you are building a
 gigabit-per-second IP router), it is probably better to have cleaner
 code you can be most certain about the security of.

 The non-const @ref UsefulBuf is usually used to refer a buffer to be
 filled in.  The length is the size of the buffer.

 The const @ref UsefulBufC is usually used to refer to some data that
 has been filled in. The length is amount of valid data pointed to.

 A common use is to pass a @ref UsefulBuf to a function, the function
 fills it in, the function returns a @ref UsefulBufC. The pointer is
 the same in both.

 A @ref UsefulBuf is null, it has no value, when @c ptr in it is @c NULL.

 There are utility functions for the following:
  - Initializing
  - Create initialized const @ref UsefulBufC from compiler literals
  - Create initialized const @ref UsefulBufC from NULL-terminated string
  - Make an empty @ref UsefulBuf on the stack
  - Checking whether a @ref UsefulBuf is null, empty or both
  - Copying, copying with offset, copying head or tail
  - Comparing and finding substrings

 See also @ref UsefulOutBuf. It is a richer structure that has both
 the size of the valid data and the size of the buffer.

 @ref UsefulBuf is only 16 or 8 bytes on a 64- or 32-bit machine so it
 can go on the stack and be a function parameter or return value.

 Another way to look at it is this. C has the NULL-terminated string
 as a means for handling text strings, but no means or convention for
 binary strings. Other languages do have such means, Rust, an
 efficient compiled language, for example.

 @ref UsefulBuf is kind of like the Useful Pot Pooh gave Eeyore on his
 birthday.  Eeyore's balloon fits beautifully, "it goes in and out
 like anything".
*/
typedef struct q_useful_buf_c {
    const void *ptr;
    size_t      len;
} UsefulBufC;


/**
 This non-const @ref UsefulBuf is typically used for some allocated
 memory that is to be filled in. The @c len is the amount of memory,
 not the length of the valid data in the buffer.
 */
typedef struct q_useful_buf {
   void  *ptr;
   size_t len;
} UsefulBuf;


/**
 A null @ref UsefulBufC is one that has no value in the same way a @c
 NULL pointer has no value.  A @ref UsefulBufC is @c NULL when the @c
 ptr field is @c NULL. It doesn't matter what @c len is.  See
 UsefulBuf_IsEmpty() for the distinction between null and empty.
 */
#define NULLUsefulBufC  ((UsefulBufC) {NULL, 0})


/**
 A null @ref UsefulBuf is one that has no memory associated the same
 way @c NULL points to nothing. It does not matter what @c len is.
 */
#define NULLUsefulBuf   ((UsefulBuf) {NULL, 0})


/**
 @brief Check if a @ref UsefulBuf is @ref NULLUsefulBuf or not.

 @param[in] UB The UsefulBuf to check.

 @return 1 if it is @ref NULLUsefulBuf, 0 if not.
 */
static inline int UsefulBuf_IsNULL(UsefulBuf UB);


/**
 @brief Check if a @ref UsefulBufC is @ref NULLUsefulBufC or not.

 @param[in] UB The @ref UsefulBufC to check.

 @return 1 if it is @c NULLUsefulBufC, 0 if not.
 */
static inline int UsefulBuf_IsNULLC(UsefulBufC UB);


/**
 @brief Check if a @ref UsefulBuf is empty or not.

 @param[in] UB The @ref UsefulBuf to check.

 @return 1 if it is empty, 0 if not.

 An "empty" @ref UsefulBuf is one that has a value and can be
 considered to be set, but that value is of zero length.  It is empty
 when @c len is zero. It doesn't matter what the @c ptr is.

 A lot of uses will not need to clearly distinguish a @c NULL @ref
 UsefulBuf from an empty one and can have the @c ptr @c NULL and the
 @c len 0.  However if a use of @ref UsefulBuf needs to make a
 distinction then @c ptr should not be @c NULL when the @ref UsefulBuf
 is considered empty, but not @c NULL.
 */
static inline int UsefulBuf_IsEmpty(UsefulBuf UB);


/**
 @brief Check if a @ref UsefulBufC is empty or not.

 @param[in] UB The @ref UsefulBufC to check.

 @return 1 if it is empty, 0 if not.
 */
static inline int UsefulBuf_IsEmptyC(UsefulBufC UB);


/**
 @brief Check if a @ref UsefulBuf is @ref NULLUsefulBuf or empty.

 @param[in] UB The @ref UsefulBuf to check.

 @return 1 if it is either @ref NULLUsefulBuf or empty, 0 if not.
 */
static inline int UsefulBuf_IsNULLOrEmpty(UsefulBuf UB);


/**
 @brief Check if a @ref UsefulBufC is @ref NULLUsefulBufC or empty.

 @param[in] UB The @ref UsefulBufC to check.

 @return 1 if it is either @ref NULLUsefulBufC or empty, 0 if not.
 */
static inline int UsefulBuf_IsNULLOrEmptyC(UsefulBufC UB);


/**
 @brief Convert a non-const @ref UsefulBuf to a const @ref UsefulBufC.

 @param[in] UB The @ref UsefulBuf to convert.

 @return A @ref UsefulBufC struct.
 */
static inline UsefulBufC UsefulBuf_Const(const UsefulBuf UB);


/**
 @brief Convert a const @ref UsefulBufC to a non-const @ref UsefulBuf.

 @param[in] UBC The @ref UsefulBuf to convert.

 @return A non-const @ref UsefulBuf struct.
 */
static inline UsefulBuf UsefulBuf_Unconst(const UsefulBufC UBC);


/**
 Convert a literal string to a @ref UsefulBufC.

 @c szString must be a literal string that @c sizeof() works on.  This
 is better for literal strings than UsefulBuf_FromSZ() because it
 generates less code. It will not work on non-literal strings.

 The terminating \0 (NULL) is NOT included in the length!
 */
#define UsefulBuf_FROM_SZ_LITERAL(szString) \
    ((UsefulBufC) {(szString), sizeof(szString)-1})


/**
 Convert a literal byte array to a @ref UsefulBufC.

 @c pBytes must be a literal string that @c sizeof() works on.  It
 will not work on non-literal arrays.
 */
#define UsefulBuf_FROM_BYTE_ARRAY_LITERAL(pBytes) \
    ((UsefulBufC) {(pBytes), sizeof(pBytes)})


/**
 Make an automatic variable named @c name of type @ref UsefulBuf and
 point it to a stack variable of the given @c size.
 */
#define  UsefulBuf_MAKE_STACK_UB(name, size) \
    uint8_t    __pBuf##name[(size)];\
    UsefulBuf  name = {__pBuf##name , sizeof( __pBuf##name )}


/**
 Make a byte array in to a @ref UsefulBuf. This is usually used on
 stack variables or static variables.  Also see @ref
 UsefulBuf_MAKE_STACK_UB.
 */
#define UsefulBuf_FROM_BYTE_ARRAY(pBytes) \
    ((UsefulBuf) {(pBytes), sizeof(pBytes)})


/**
 @brief Convert a NULL-terminated string to a @ref UsefulBufC.

 @param[in] szString The string to convert.

 @return A @ref UsefulBufC struct.

 @c UsefulBufC.ptr points to the string so its lifetime must be
 maintained.

 The terminating \0 (NULL) is NOT included in the length.
 */
static inline UsefulBufC UsefulBuf_FromSZ(const char *szString);


/**
 @brief Copy one @ref UsefulBuf into another at an offset.

 @param[in] Dest     Destination buffer to copy into.
 @param[in] uOffset  The byte offset in @c Dest at which to copy to.
 @param[in] Src      The bytes to copy.

 @return Pointer and length of the copy or @ref NULLUsefulBufC.

 This fails and returns @ref NULLUsefulBufC if @c offset is beyond the
 size of @c Dest.

 This fails and returns @ref NULLUsefulBufC if the @c Src length plus
 @c uOffset is greater than the length of @c Dest.

 The results are undefined if @c Dest and @c Src overlap.

 This assumes that there is valid data in @c Dest up to @c
 uOffset. The @ref UsefulBufC returned starts at the beginning of @c
 Dest and goes to @c Src.len @c + @c uOffset.
 */
UsefulBufC UsefulBuf_CopyOffset(UsefulBuf Dest, size_t uOffset, const UsefulBufC Src);


/**
 @brief Copy one @ref UsefulBuf into another.

 @param[in] Dest  The destination buffer to copy into.
 @param[out] Src  The source to copy from.

 @return Filled in @ref UsefulBufC on success, @ref NULLUsefulBufC
         on failure.

 This fails if @c Src.len is greater than @c Dest.len.

 Note that like @c memcpy(), the pointers are not checked and this
 will crash rather than return @ref NULLUsefulBufC if they are @c
 NULL or invalid.

 The results are undefined if @c Dest and @c Src overlap.
 */
static inline UsefulBufC UsefulBuf_Copy(UsefulBuf Dest, const UsefulBufC Src);


/**
 @brief Set all bytes in a @ref UsefulBuf to a value, for example to 0.

 @param[in] pDest  The destination buffer to copy into.
 @param[in] value  The value to set the bytes to.

 Note that like @c memset(), the pointer in @c pDest is not checked
 and this will crash if @c NULL or invalid.
 */
static inline UsefulBufC UsefulBuf_Set(UsefulBuf pDest, uint8_t value);


/**
 @brief Copy a pointer into a @ref UsefulBuf.

 @param[in,out] Dest  The destination buffer to copy into.
 @param[in] ptr       The source to copy from.
 @param[in] uLen      Length of the source; amount to copy.

 @return 0 on success, 1 on failure.

 This fails and returns @ref NULLUsefulBufC if @c uLen is greater than
 @c pDest->len.

 Note that like @c memcpy(), the pointers are not checked and this
 will crash, rather than return 1 if they are @c NULL or invalid.
 */
static inline UsefulBufC UsefulBuf_CopyPtr(UsefulBuf Dest,
                                           const void *ptr,
                                           size_t uLen);


/**
  @brief Returns a truncation of a @ref UsefulBufC.

  @param[in] UB       The buffer to get the head of.
  @param[in] uAmount  The number of bytes in the head.

  @return A @ref UsefulBufC that is the head of UB.
 */
static inline UsefulBufC UsefulBuf_Head(UsefulBufC UB, size_t uAmount);


/**
 @brief  Returns bytes from the end of a @ref UsefulBufC.

 @param[in] UB       The buffer to get the tail of.
 @param[in] uAmount  The offset from the start where the tail is to begin.

 @return A @ref UsefulBufC that is the tail of @c UB or @ref NULLUsefulBufC
         if @c uAmount is greater than the length of the @ref UsefulBufC.

 If @c UB.ptr is @c NULL, but @c UB.len is not zero, then the result will
 be a @ref UsefulBufC with a @c NULL @c ptr and @c len with the length
 of the tail.
 */
static inline UsefulBufC UsefulBuf_Tail(UsefulBufC UB, size_t uAmount);


/**
 @brief Compare one @ref UsefulBufC to another.

 @param[in] UB1  The first buffer to compare.
 @param[in] UB2  The second buffer to compare.

 @return 0, positive or negative value.

 Returns a negative value if @c UB1 if is less than @c UB2. @c UB1 is
 less than @c UB2 if it is shorter or the first byte that is not the
 same is less.

 Returns 0 if the inputs are the same.

 Returns a positive value if @c UB2 is less than @c UB1.

 All that is of significance is that the result is positive, negative
 or 0. (This doesn't return the difference between the first
 non-matching byte like @c memcmp() ).
 */
int UsefulBuf_Compare(const UsefulBufC UB1, const UsefulBufC UB2);


/**
 @brief Find first byte that is not a particular byte value.

 @param[in] UB     The destination buffer for byte comparison.
 @param[in] uValue The byte value to compare to.

 @return  Offset of first byte that isn't @c uValue or
          @c SIZE_MAX if all bytes are @c uValue.

 Note that unlike most comparison functions, 0
 does not indicate a successful comparison, so the
 test for match is:

      UsefulBuf_IsValue(...) == SIZE_MAX

 If @c UB is null or empty, there is no match
 and 0 is returned.
 */
size_t UsefulBuf_IsValue(const UsefulBufC UB, uint8_t uValue);


/**
 @brief Find one @ref UsefulBufC in another.

 @param[in] BytesToSearch  Buffer to search through.
 @param[in] BytesToFind    Buffer with bytes to be found.

 @return Position of found bytes or @c SIZE_MAX if not found.
 */
size_t UsefulBuf_FindBytes(UsefulBufC BytesToSearch, UsefulBufC BytesToFind);


#if 1 // NOT_DEPRECATED
/** Deprecated macro; use @ref UsefulBuf_FROM_SZ_LITERAL instead */
#define SZLiteralToUsefulBufC(szString) \
    ((UsefulBufC) {(szString), sizeof(szString)-1})

/** Deprecated macro; use UsefulBuf_MAKE_STACK_UB instead */
#define  MakeUsefulBufOnStack(name, size) \
    uint8_t    __pBuf##name[(size)];\
    UsefulBuf  name = {__pBuf##name , sizeof( __pBuf##name )}

/** Deprecated macro; use @ref UsefulBuf_FROM_BYTE_ARRAY_LITERAL instead */
#define ByteArrayLiteralToUsefulBufC(pBytes) \
    ((UsefulBufC) {(pBytes), sizeof(pBytes)})

/** Deprecated function; use UsefulBuf_Unconst() instead */
static inline UsefulBuf UsefulBufC_Unconst(const UsefulBufC UBC)
{
    return (UsefulBuf){(void *)UBC.ptr, UBC.len};
}
#endif




/**
 @brief Copy a @c float to a @c uint32_t.

 @param[in] f  Float value to copy.

 @return  A @c uint32_t with the float bits.

 Convenience function to avoid type punning, compiler warnings and
 such. The optimizer usually reduces this to a simple assignment.  This
 is a crusty corner of C.
 */
static inline uint32_t UsefulBufUtil_CopyFloatToUint32(float f);


/**
 @brief Copy a @c double to a @c uint64_t.

 @param[in] d  Double value to copy.

 @return  A @c uint64_t with the double bits.

 Convenience function to avoid type punning, compiler warnings and
 such. The optimizer usually reduces this to a simple assignment.  This
 is a crusty corner of C.
 */
static inline uint64_t UsefulBufUtil_CopyDoubleToUint64(double d);


/**
 @brief Copy a @c uint32_t to a @c float.

 @param[in] u32  Integer value to copy.

 @return  The value as a @c float.

 Convenience function to avoid type punning, compiler warnings and
 such. The optimizer usually reduces this to a simple assignment.  This
 is a crusty corner of C.
 */
static inline float UsefulBufUtil_CopyUint32ToFloat(uint32_t u32);


/**
 @brief Copy a @c uint64_t to a @c double.

 @param[in] u64  Integer value to copy.

 @return  The value as a @c double.

 Convenience function to avoid type punning, compiler warnings and
 such. The optimizer usually reduces this to a simple assignment.  This
 is a crusty corner of C.
 */
static inline double UsefulBufUtil_CopyUint64ToDouble(uint64_t u64);




/**
 UsefulOutBuf is a structure and functions (an object) for serializing
 data into a buffer when encoding a network protocol or writing data
 to file.

 The main idea is that all the pointer manipulation is performed by
 @ref UsefulOutBuf functions so the caller doesn't have to do any
 pointer manipulation.  The pointer manipulation is centralized.  This
 code will have been reviewed and written carefully so it spares the
 caller of much of this work and results in safer code with less work.

 The @ref UsefulOutBuf methods that add data to the output buffer
 always check the length and will never write off the end of the
 output buffer. If an attempt to add data that will not fit is made,
 an internal error flag will be set and further attempts to add data
 will not do anything.

 There is no way to ever write off the end of that buffer when calling
 the @c UsefulOutBuf_AddXxx() and @c UsefulOutBuf_InsertXxx()
 functions.

 The functions to add data do not return an error. The working model
 is that all calls to add data are made without an error check. Errors
 are just checked for once after all the data has been added before the
 and before serialized data is to be used. This makes the calling code
 cleaner.

 There is a utility function to get the error status anytime along the
 way for a special circumstance. There are functions to see how much
 room is left and see if some data will fit too, but their use is
 generally not necessary.

 The general call flow is:

    - Initialize by calling @ref UsefulOutBuf_Init(). The output
      buffer given to it can be from the heap, stack or
      otherwise. @ref UsefulOutBuf_MakeOnStack is a convenience macro
      that makes a buffer on the stack and initializes it.

    - Call methods like UsefulOutBuf_InsertString(),
      UsefulOutBuf_AppendUint32() and UsefulOutBuf_InsertUsefulBuf()
      to output data. The append calls add data to the end of the
      valid data. The insert calls take a position argument.

    - Call UsefulOutBuf_OutUBuf() or UsefulOutBuf_CopyOut() to see
      there were no errors and to get the serialized output bytes.

 @ref UsefulOutBuf can be used in a size calculation mode to calculate
 the size of output that would be generated. This is useful to
 calculate the size of a buffer that is to be allocated to hold the
 output. To use @ref UsefulOutBuf in this mode, call
 UsefulOutBuf_Init() with the @c Storage @ref UsefulBuf as
 @c (UsefulBuf){NULL,MAX_UINT32}. Then call all the Insert and Add
 functions. No attempt will made to actually copy data, so only the
 lengths have to be valid for these calls.

 Methods like UsefulOutBuf_InsertUint64() always output in network
 bytes order (big endian).

 The possible errors are:
  - The @ref UsefulOutBuf was not initialized or was corrupted.

  - An attempt was made to add data that will not fit.

  - An attempt was made to insert data at a position beyond the end of
    the buffer.

  - An attempt was made to insert data at a position beyond the valid
    data in the buffer.

 Some inexpensive simple sanity checks are performed before every data
 addition to guard against use of an uninitialized or corrupted
 UsefulOutBuf.

 This has been used to create a CBOR encoder. The CBOR encoder has
 almost no pointer manipulation in it, is easier to read, and easier
 to review.

 A @ref UsefulOutBuf is small and can go on the stack:
   - 32 bytes (27 bytes plus alignment padding) on a 64-bit machine
   - 16 bytes (15 bytes plus alignment padding) on a 32-bit machines
 */
typedef struct useful_out_buf {
   // PRIVATE DATA STRUCTURE
   UsefulBuf  UB;       // Memory that is being output to
   size_t     data_len; // length of the data
   uint16_t   magic;    // Used to detect corruption and lack of initialization
   uint8_t    err;
} UsefulOutBuf;


/**
 @brief Initialize and supply the actual output buffer.

 @param[out] pUOutBuf  The @ref UsefulOutBuf to initialize.
 @param[in] Storage    Buffer to output into.

 Initializes  the @ref UsefulOutBuf with storage. Sets the current
 position to the beginning of the buffer clears the error.

 This must be called before the @ref UsefulOutBuf is used.
 */
void UsefulOutBuf_Init(UsefulOutBuf *pUOutBuf, UsefulBuf Storage);


/**
 Convenience macro to make a @ref UsefulOutBuf on the stack and
 initialize it with a stack buffer of the given size. The variable
 will be named @c name.
 */
#define  UsefulOutBuf_MakeOnStack(name, size) \
   uint8_t       __pBuf##name[(size)];\
   UsefulOutBuf  name;\
   UsefulOutBuf_Init(&(name), (UsefulBuf){__pBuf##name, (size)});


/**
 @brief Reset a @ref UsefulOutBuf for re use

 @param[in] pUOutBuf Pointer to the @ref UsefulOutBuf

 This sets the amount of data in the output buffer to none and clears
 the error state.

 The output buffer is still the same one and size as from the
 UsefulOutBuf_Init() call.

 This doesn't zero the data, just resets to 0 bytes of valid data.
 */
static inline void UsefulOutBuf_Reset(UsefulOutBuf *pUOutBuf);


/**
 @brief Returns position of end of data in the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.

 @return position of end of data.

 On a freshly initialized @ref UsefulOutBuf with no data added, this
 will return 0. After 10 bytes have been added, it will return 10 and
 so on.

 Generally callers will not need this function for most uses of @ref
 UsefulOutBuf.
 */
static inline size_t UsefulOutBuf_GetEndPosition(UsefulOutBuf *pUOutBuf);


/**
 @brief Returns whether any data has been added to the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.

 @return 1 if output position is at start.
 */
static inline int UsefulOutBuf_AtStart(UsefulOutBuf *pUOutBuf);


/**
 @brief Inserts bytes into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] NewData   The bytes to insert.
 @param[in] uPos      Index in output buffer at which to insert.

 @c NewData is the pointer and length for the bytes to be added to the
 output buffer. There must be room in the output buffer for all of @c
 NewData or an error will occur.

 The insertion point must be between 0 and the current valid data. If
 not, an error will occur. Appending data to the output buffer is
 achieved by inserting at the end of the valid data. This can be
 retrieved by calling UsefulOutBuf_GetEndPosition().

 When insertion is performed, the bytes between the insertion point
 and the end of data previously added to the output buffer are slid to
 the right to make room for the new data.

 Overlapping buffers are OK. @c NewData can point to data in the
 output buffer.

 If an error occurs an error state is set in the @ref UsefulOutBuf. No
 error is returned.  All subsequent attempts to add data will do
 nothing.

 The intended use is that all additions are made without checking for
 an error. The error will be taken into account when
 UsefulOutBuf_OutUBuf() returns @c NullUsefulBufC.
 UsefulOutBuf_GetError() can also be called to check for an error.
 */
void UsefulOutBuf_InsertUsefulBuf(UsefulOutBuf *pUOutBuf,
                                  UsefulBufC NewData,
                                  size_t uPos);


/**
 @brief Insert a data buffer into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] pBytes    Pointer to the bytes to insert
 @param[in] uLen      Length of the bytes to insert
 @param[in] uPos      Index in output buffer at which to insert

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a pointer and length is passed in rather than an
 @ref UsefulBufC.
 */
static inline void UsefulOutBuf_InsertData(UsefulOutBuf *pUOutBuf,
                                           const void *pBytes,
                                           size_t uLen,
                                           size_t uPos);


/**
 @brief Insert a NULL-terminated string into the UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] szString  NULL-terminated string to insert.
 @param[in] uPos      Index in output buffer at which to insert.
 */
static inline void UsefulOutBuf_InsertString(UsefulOutBuf *pUOutBuf,
                                             const char *szString,
                                             size_t uPos);


/**
 @brief Insert a byte into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the UsefulOutBuf.
 @param[in] byte      Bytes to insert.
 @param[in] uPos      Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a single byte is to be inserted.
 */
static inline void UsefulOutBuf_InsertByte(UsefulOutBuf *pUOutBuf,
                                           uint8_t byte,
                                           size_t uPos);


/**
 @brief Insert a 16-bit integer into the @ref UsefulOutBuf.

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger16  Integer to insert.
 @param[in] uPos        Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a two-byte integer is to be inserted.

 The integer will be inserted in network byte order (big endian).
 */
static inline void UsefulOutBuf_InsertUint16(UsefulOutBuf *pUOutBuf,
                                             uint16_t uInteger16,
                                             size_t uPos);


/**
 @brief Insert a 32-bit integer into the @ref UsefulOutBuf.

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger32  Integer to insert.
 @param[in] uPos        Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a four-byte integer is to be inserted.

 The integer will be inserted in network byte order (big endian).
 */
static inline void UsefulOutBuf_InsertUint32(UsefulOutBuf *pUOutBuf,
                                             uint32_t uInteger32,
                                             size_t uPos);


/**
 @brief Insert a 64-bit integer into the @ref UsefulOutBuf.

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger64  Integer to insert.
 @param[in] uPos        Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being an eight-byte integer is to be inserted.

 The integer will be inserted in network byte order (big endian).
 */
static inline void UsefulOutBuf_InsertUint64(UsefulOutBuf *pUOutBuf,
                                             uint64_t uInteger64,
                                             size_t uPos);


/**
 @brief Insert a @c float into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] f         @c float to insert.
 @param[in] uPos      Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a @c float is to be inserted.

 The @c float will be inserted in network byte order (big endian).
 */
static inline void UsefulOutBuf_InsertFloat(UsefulOutBuf *pUOutBuf,
                                            float f,
                                            size_t uPos);


/**
 @brief Insert a @c double into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] d         @c double  to insert.
 @param[in] uPos      Index in output buffer at which to insert.

 See UsefulOutBuf_InsertUsefulBuf() for details. This is the same with
 the difference being a @c double is to be inserted.

 The @c double will be inserted in network byte order (big endian).
 */
static inline void UsefulOutBuf_InsertDouble(UsefulOutBuf *pUOutBuf,
                                             double d,
                                             size_t uPos);


/**
 @brief Append a @ref UsefulBuf into the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] NewData   The @ref UsefulBuf with the bytes to append.

 See UsefulOutBuf_InsertUsefulBuf() for details. This does the same
 with the insertion point at the end of the valid data.
*/
static inline void UsefulOutBuf_AppendUsefulBuf(UsefulOutBuf *pUOutBuf,
                                                UsefulBufC NewData);


/**
 @brief Append bytes to the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] pBytes    Pointer to bytes to append.
 @param[in] uLen      Length of @c pBytes to append.

 See UsefulOutBuf_InsertData() for details. This does the same
 with the insertion point at the end of the valid data.
 */
static inline void UsefulOutBuf_AppendData(UsefulOutBuf *pUOutBuf,
                                           const void *pBytes,
                                           size_t uLen);


/**
 @brief Append a NULL-terminated string to the @ref UsefulOutBuf

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] szString  NULL-terminated string to append.
 */
static inline void UsefulOutBuf_AppendString(UsefulOutBuf *pUOutBuf,
                                             const char *szString);


/**
 @brief Append a byte to the @ref UsefulOutBuf

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] byte      Bytes to append.

 See UsefulOutBuf_InsertByte() for details. This does the same
 with the insertion point at the end of the valid data.
 */
static inline void UsefulOutBuf_AppendByte(UsefulOutBuf *pUOutBuf,
                                           uint8_t byte);


/**
 @brief Append an integer to the @ref UsefulOutBuf

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger16  Integer to append.

 See UsefulOutBuf_InsertUint16() for details. This does the same
 with the insertion point at the end of the valid data.

 The integer will be appended in network byte order (big endian).
 */
static inline void UsefulOutBuf_AppendUint16(UsefulOutBuf *pUOutBuf,
                                             uint16_t uInteger16);


/**
 @brief Append an integer to the @ref UsefulOutBuf

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger32  Integer to append.

 See UsefulOutBuf_InsertUint32() for details. This does the same
 with the insertion point at the end of the valid data.

 The integer will be appended in network byte order (big endian).
 */
static inline void UsefulOutBuf_AppendUint32(UsefulOutBuf *pUOutBuf,
                                             uint32_t uInteger32);


/**
 @brief Append an integer to the @ref UsefulOutBuf

 @param[in] pUOutBuf    Pointer to the @ref UsefulOutBuf.
 @param[in] uInteger64  Integer to append.

 See UsefulOutBuf_InsertUint64() for details. This does the same
 with the insertion point at the end of the valid data.

 The integer will be appended in network byte order (big endian).
 */
static inline void UsefulOutBuf_AppendUint64(UsefulOutBuf *pUOutBuf,
                                             uint64_t uInteger64);


/**
 @brief Append a @c float to the @ref UsefulOutBuf

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] f         @c float to append.

 See UsefulOutBuf_InsertFloat() for details. This does the same
 with the insertion point at the end of the valid data.

 The float will be appended in network byte order (big endian).
 */
static inline void UsefulOutBuf_AppendFloat(UsefulOutBuf *pUOutBuf,
                                            float f);


/**
 @brief Append a @c double to the @ref UsefulOutBuf

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[in] d         @c double to append.

 See UsefulOutBuf_InsertDouble() for details. This does the same
 with the insertion point at the end of the valid data.

 The double will be appended in network byte order (big endian).
 */
static inline void UsefulOutBuf_AppendDouble(UsefulOutBuf *pUOutBuf,
                                             double d);


/**
 @brief Returns the current error status.

 @param[in] pUOutBuf Pointer to the @ref UsefulOutBuf.

 @return 0 if all OK, 1 on error.

 This is the error status since the call to either
 UsefulOutBuf_Reset() of UsefulOutBuf_Init().  Once it goes into error
 state it will stay until one of those functions is called.

 Possible error conditions are:
   - bytes to be inserted will not fit
   - insertion point is out of buffer or past valid data
   - current position is off end of buffer (probably corrupted or uninitialized)
   - detect corruption / uninitialized by bad magic number
 */
static inline int UsefulOutBuf_GetError(UsefulOutBuf *pUOutBuf);


/**
 @brief Returns number of bytes unused used in the output buffer.

 @param[in] pUOutBuf Pointer to the @ref UsefulOutBuf.

 @return Number of unused bytes or zero.

 Because of the error handling strategy and checks in
 UsefulOutBuf_InsertUsefulBuf() it is usually not necessary to use
 this.
 */
static inline size_t UsefulOutBuf_RoomLeft(UsefulOutBuf *pUOutBuf);


/**
 @brief Returns 1 if some number of bytes will fit in the @ref UsefulOutBuf.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf
 @param[in] uLen      Number of bytes for which to check

 @return 1 if @c uLen bytes will fit, 0 if not.

 Because of the error handling strategy and checks in
 UsefulOutBuf_InsertUsefulBuf() it is usually not necessary to use
 this.
 */
static inline int UsefulOutBuf_WillItFit(UsefulOutBuf *pUOutBuf, size_t uLen);


 /**
 @brief Returns 1 if buffer given to UsefulOutBuf_Init() was @c NULL.

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf

 @return 1 if buffer given to UsefulOutBuf_Init() was @c NULL.

 Giving a @c NULL output buffer to UsefulOutBuf_Init() is used
 when just calculating the length of the encoded data.
 */
static inline int UsefulOutBuf_IsBufferNULL(UsefulOutBuf *pUOutBuf);


/**
   @brief Returns the resulting valid data in a UsefulOutBuf

   @param[in] pUOutBuf Pointer to the @ref UsefulOutBuf.

   @return The valid data in @ref UsefulOutBuf or
           @ref NULLUsefulBufC if there was an error adding data.

   The storage for the returned data is the @c Storage parameter passed
   to UsefulOutBuf_Init(). See also UsefulOutBuf_CopyOut().

   This can be called anytime and many times to get intermediate
   results. It doesn't change the data or reset the current position
   so you can keep adding data.
 */
UsefulBufC UsefulOutBuf_OutUBuf(UsefulOutBuf *pUOutBuf);


/**
 @brief Copies the valid data into a supplied buffer

 @param[in] pUOutBuf  Pointer to the @ref UsefulOutBuf.
 @param[out] Dest     The destination buffer to copy into.

 @return Pointer and length of copied data or @c NULLUsefulBufC
         if it will not fit in the @c Dest buffer.

 This is the same as UsefulOutBuf_OutUBuf() except it copies the data
 to @c Dest.
*/
UsefulBufC UsefulOutBuf_CopyOut(UsefulOutBuf *pUOutBuf, UsefulBuf Dest);




/**
 @ref UsefulInputBuf is the counterpart to @ref UsefulOutBuf and is
 for parsing data read or received.  Initialize it with the data from
 the network. Then use the functions here to get data chunks of
 various types. A position cursor is maintained internally.

 As long as the functions here are used, there will never be a
 reference off the end of the given buffer. This is true even if they
 care called incorrectly, an attempt is made to seek of the end of the
 buffer, etc. This makes it easier to write safe and correct code.
 For example, the QCBOR decoder implementation is safer and easier to
 review through its use of @ref UsefulInputBuf.

 @ref UsefulInputBuf maintains an internal error state.  The
 intended use is that data chunks can be fetched without error
 checking until the end.  Once data has been requested off the end of
 the buffer, the error state is entered. In the error state the
 @c UsefulInputBuf_GetXxxx() functions return 0, or @c NULL or
 @ref NULLUsefulBufC. As long as null are not dereferenced, the
 error check can be put off until the end, simplifying the calling
 code.

 The integer and float parsing expects network byte order (big
 endian).  Network byte order is what is used by TCP/IP, CBOR and most
 internet protocols.

 Lots of inline functions are used to keep code size down. The code
 optimizer, particularly with the @c -Os or @c -O3, also reduces code
 size a lot. The only non-inline code is UsefulInputBuf_GetBytes()
 which is less than 100 bytes so use of @ref UsefulInputBuf doesn't
 add much code for all the messy hard-to-get right issues with parsing
 in C that is solves.

 The parse context size is:
   - 64-bit machine: 16 + 8 + 2 + 1 (5 bytes padding to align) = 32 bytes
   - 32-bit machine: 8 + 4 + 2 + 1 (1 byte padding to align) = 16 bytes
 */
typedef struct useful_input_buf {
   // PRIVATE DATA STRUCTURE
   UsefulBufC UB;     // Data being parsed
   size_t     cursor; // Current offset in data being parse
   uint16_t   magic;  // Check for corrupted or uninitialized UsefulInputBuf
   uint8_t    err;    // Set request goes off end or magic number is bad
} UsefulInputBuf;

#define UIB_MAGIC (0xB00F)


/**
 @brief Initialize the UsefulInputBuf structure before use.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf instance.
 @param[in] UB       The data to parse.
 */
static inline void UsefulInputBuf_Init(UsefulInputBuf *pUInBuf, UsefulBufC UB);


/**
 @brief Returns current position in input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return Integer position of the cursor.

 The position that the next bytes will be returned from.
 */
static size_t UsefulInputBuf_Tell(UsefulInputBuf *pUInBuf);


/**
 @brief Sets the current position in input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.
 @param[in] uPos     Position to set to.

 If the position is off the end of the input buffer, the error state
 is entered, and all functions will do nothing.

 Seeking to a valid position in the buffer will not reset the error
 state. Only re initialization will do that.
 */
static void UsefulInputBuf_Seek(UsefulInputBuf *pUInBuf, size_t uPos);


/**
 @brief Returns the number of bytes from the cursor to the end of the buffer,
 the unconsumed bytes.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return Number of bytes unconsumed or 0 on error.

 This is a critical function for input length validation.

 Returns 0 if the cursor it invalid or corruption of the structure is
 detected.
 */
static size_t UsefulInputBuf_BytesUnconsumed(UsefulInputBuf *pUInBuf);


/**
 @brief Check if there are any unconsumed bytes.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.
 @param[in] uLen     Number of bytes to check availability for.

 @return 1 if @c uLen bytes are available after the cursor, and 0 if not.
 */
static int UsefulInputBuf_BytesAvailable(UsefulInputBuf *pUInBuf, size_t uLen);


/**
 @brief Get pointer to bytes out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.
 @param[in] uNum     Number of bytes to get.

 @return Pointer to bytes.

 This consumes @c uNum bytes from the input buffer. It returns a
 pointer to the start of the @c uNum bytes.

 If there are not @c uNum bytes in the input buffer, @c NULL will be
 returned and an error will be set.

 It advances the current position by @c uNum bytes.
 */
const void * UsefulInputBuf_GetBytes(UsefulInputBuf *pUInBuf, size_t uNum);


/**
 @brief Get @ref UsefulBuf out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.
 @param[in] uNum     Number of bytes to get.

 @return A @ref UsefulBufC with ptr and length of bytes consumed.

 This consumes @c uNum bytes from the input buffer and returns the
 pointer and length for them as a @ref UsefulBufC. The length returned
 will always be @c uNum.

 If there are not @c uNum bytes in the input buffer, @ref NULLUsefulBufC
 will be returned and the error state is set.

 It advances the current position by @c uNum bytes.
 */
static inline UsefulBufC UsefulInputBuf_GetUsefulBuf(UsefulInputBuf *pUInBuf, size_t uNum);


/**
 @brief Get a byte out of the input buffer.

 @param[in] pUInBuf  Pointer to the @ref UsefulInputBuf.

 @return The byte.

 This consumes 1 byte from the input buffer. It returns the byte.

 If there is not 1 byte in the buffer, 0 will be returned for the byte
 and an error set internally.  You must check the error at some point
 to know whether the 0 was the real value or just returned in error,
 but you may not have to do that right away.  Check the error state
 with UsefulInputBuf_GetError().  You can also know you are in the
 error state if UsefulInputBuf_GetBytes() returns @c NULL or the @c
 ptr from UsefulInputBuf_GetUsefulBuf() is @c NULL.

 It advances the current position by 1 byte.
 */
static inline uint8_t UsefulInputBuf_GetByte(UsefulInputBuf *pUInBuf);


/**
 @brief Get a @c uint16_t out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return The @c uint16_t.

 See UsefulInputBuf_GetByte(). This works the same, except it returns
 a @c uint16_t and two bytes are consumed.

 The input bytes must be in network order (big endian).
 */
static inline uint16_t UsefulInputBuf_GetUint16(UsefulInputBuf *pUInBuf);


/**
 @brief Get a uint32_t out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return The @c uint32_t.

 See UsefulInputBuf_GetByte(). This works the same, except it returns
 a @c uint32_t and four bytes are consumed.

 The input bytes must be in network order (big endian).
 */
static uint32_t UsefulInputBuf_GetUint32(UsefulInputBuf *pUInBuf);


/**
 @brief Get a uint64_t out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return The uint64_t.

 See UsefulInputBuf_GetByte(). This works the same, except it returns
 a @c uint64_t and eight bytes are consumed.

 The input bytes must be in network order (big endian).
 */
static uint64_t UsefulInputBuf_GetUint64(UsefulInputBuf *pUInBuf);


/**
 @brief Get a float out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return The float.

 See UsefulInputBuf_GetByte(). This works the same, except it returns
 a float and four bytes are consumed.

 The input bytes must be in network order (big endian).
 */
static float UsefulInputBuf_GetFloat(UsefulInputBuf *pUInBuf);


/**
 @brief Get a double out of the input buffer.

 @param[in] pUInBuf  Pointer to the UsefulInputBuf.

 @return The double.

 See UsefulInputBuf_GetByte(). This works the same, except it returns
 a double and eight bytes are consumed.

 The input bytes must be in network order (big endian).
 */
static double UsefulInputBuf_GetDouble(UsefulInputBuf *pUInBuf);


/**
 @brief Get the error status.

 @param[in] pUInBuf  Pointer to the @ref UsefulInputBuf.

 @return 0 if there is no error, 1 if there is.

 The error state is entered for one of these reasons:
 - Attempt to fetch data past the end of the buffer
 - Attempt to seek to a position past the end of the buffer
 - Attempt to get data from an uninitialized  or corrupt instance
   of @ref UsefulInputBuf

 Once in the error state, it can only be cleared by calling
 UsefulInputBuf_Init().

 You may be able to only check the error state at the end after all
 the UsefulInputBuf_GetXxxx() calls have been made, but if what you
 get later depends on what you get sooner you cannot. For example,
 if you get a length or count of following items you will have to
 check the error.
 */
static int UsefulInputBuf_GetError(UsefulInputBuf *pUInBuf);




/*----------------------------------------------------------
 Inline implementations.
 */
static inline int UsefulBuf_IsNULL(UsefulBuf UB)
{
   return !UB.ptr;
}


static inline int UsefulBuf_IsNULLC(UsefulBufC UB)
{
   return !UB.ptr;
}


static inline int UsefulBuf_IsEmpty(UsefulBuf UB)
{
   return !UB.len;
}


static inline int UsefulBuf_IsEmptyC(UsefulBufC UB)
{
   return !UB.len;
}


static inline int UsefulBuf_IsNULLOrEmpty(UsefulBuf UB)
{
   return UsefulBuf_IsEmpty(UB) || UsefulBuf_IsNULL(UB);
}


static inline int UsefulBuf_IsNULLOrEmptyC(UsefulBufC UB)
{
   return UsefulBuf_IsEmptyC(UB) || UsefulBuf_IsNULLC(UB);
}


static inline UsefulBufC UsefulBuf_Const(const UsefulBuf UB)
{
   return (UsefulBufC){UB.ptr, UB.len};
}


static inline UsefulBuf UsefulBuf_Unconst(const UsefulBufC UBC)
{
   return (UsefulBuf){(void *)UBC.ptr, UBC.len};
}


static inline UsefulBufC UsefulBuf_FromSZ(const char *szString)
{
   return ((UsefulBufC) {szString, strlen(szString)});
}


static inline UsefulBufC UsefulBuf_Copy(UsefulBuf Dest, const UsefulBufC Src)
{
   return UsefulBuf_CopyOffset(Dest, 0, Src);
}


static inline UsefulBufC UsefulBuf_Set(UsefulBuf pDest, uint8_t value)
{
   memset(pDest.ptr, value, pDest.len);
   return (UsefulBufC){pDest.ptr, pDest.len};
}


static inline UsefulBufC UsefulBuf_CopyPtr(UsefulBuf Dest, const void *ptr, size_t len)
{
   return UsefulBuf_Copy(Dest, (UsefulBufC){ptr, len});
}


static inline UsefulBufC UsefulBuf_Head(UsefulBufC UB, size_t uAmount)
{
   if(uAmount > UB.len) {
      return NULLUsefulBufC;
   }
   return (UsefulBufC){UB.ptr, uAmount};
}


static inline UsefulBufC UsefulBuf_Tail(UsefulBufC UB, size_t uAmount)
{
   UsefulBufC ReturnValue;

   if(uAmount > UB.len) {
      ReturnValue = NULLUsefulBufC;
   } else if(UB.ptr == NULL) {
      ReturnValue = (UsefulBufC){NULL, UB.len - uAmount};
   } else {
      ReturnValue = (UsefulBufC){(uint8_t *)UB.ptr + uAmount, UB.len - uAmount};
   }

   return ReturnValue;
}



static inline uint32_t UsefulBufUtil_CopyFloatToUint32(float f)
{
   uint32_t u32;
   memcpy(&u32, &f, sizeof(uint32_t));
   return u32;
}

static inline uint64_t UsefulBufUtil_CopyDoubleToUint64(double d)
{
   uint64_t u64;
   memcpy(&u64, &d, sizeof(uint64_t));
   return u64;
}

static inline double UsefulBufUtil_CopyUint64ToDouble(uint64_t u64)
{
   double d;
   memcpy(&d, &u64, sizeof(uint64_t));
   return d;
}

static inline float UsefulBufUtil_CopyUint32ToFloat(uint32_t u32)
{
   float f;
   memcpy(&f, &u32, sizeof(uint32_t));
   return f;
}




static inline void UsefulOutBuf_Reset(UsefulOutBuf *pMe)
{
   pMe->data_len = 0;
   pMe->err      = 0;
}


static inline size_t UsefulOutBuf_GetEndPosition(UsefulOutBuf *pMe)
{
   return pMe->data_len;
}


static inline int UsefulOutBuf_AtStart(UsefulOutBuf *pMe)
{
   return 0 == pMe->data_len;
}


static inline void UsefulOutBuf_InsertData(UsefulOutBuf *pMe,
                                           const void *pBytes,
                                           size_t uLen,
                                           size_t uPos)
{
   UsefulBufC Data = {pBytes, uLen};
   UsefulOutBuf_InsertUsefulBuf(pMe, Data, uPos);
}


static inline void UsefulOutBuf_InsertString(UsefulOutBuf *pMe,
                                             const char *szString,
                                             size_t uPos)
{
   UsefulOutBuf_InsertUsefulBuf(pMe,
                                (UsefulBufC){szString, strlen(szString)},
                                uPos);
}


static inline void UsefulOutBuf_InsertByte(UsefulOutBuf *me,
                                           uint8_t byte,
                                           size_t uPos)
{
   UsefulOutBuf_InsertData(me, &byte, 1, uPos);
}


static inline void UsefulOutBuf_InsertUint16(UsefulOutBuf *me,
                                             uint16_t uInteger16,
                                             size_t uPos)
{
   // See UsefulOutBuf_InsertUint64() for comments on this code

   const void *pBytes;

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   pBytes = &uInteger16;

#elif defined(USEFULBUF_CONFIG_HTON)
   uint16_t uTmp = htons(uInteger16);
   pBytes        = &uTmp;

#elif defined(USEFULBUF_CONFIG_LITTLE_ENDIAN) && defined(USEFULBUF_CONFIG_BSWAP)
   uint16_t uTmp = __builtin_bswap16(uInteger16);
   pBytes = &uTmp;

#else
   uint8_t aTmp[2];

   aTmp[0] = (uInteger16 & 0xff00) >> 8;
   aTmp[1] = (uInteger16 & 0xff);

   pBytes = aTmp;
#endif

   UsefulOutBuf_InsertData(me, pBytes, 2, uPos);
}


static inline void UsefulOutBuf_InsertUint32(UsefulOutBuf *pMe,
                                             uint32_t uInteger32,
                                             size_t uPos)
{
   // See UsefulOutBuf_InsertUint64() for comments on this code

   const void *pBytes;

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   pBytes = &uInteger32;

#elif defined(USEFULBUF_CONFIG_HTON)
   uint32_t uTmp = htonl(uInteger32);
   pBytes = &uTmp;

#elif defined(USEFULBUF_CONFIG_LITTLE_ENDIAN) && defined(USEFULBUF_CONFIG_BSWAP)
   uint32_t uTmp = __builtin_bswap32(uInteger32);

   pBytes = &uTmp;

#else
   uint8_t aTmp[4];

   aTmp[0] = (uInteger32 & 0xff000000) >> 24;
   aTmp[1] = (uInteger32 & 0xff0000) >> 16;
   aTmp[2] = (uInteger32 & 0xff00) >> 8;
   aTmp[3] = (uInteger32 & 0xff);

   pBytes = aTmp;
#endif

   UsefulOutBuf_InsertData(pMe, pBytes, 4, uPos);
}

static inline void UsefulOutBuf_InsertUint64(UsefulOutBuf *pMe,
                                             uint64_t uInteger64,
                                             size_t uPos)
{
   const void *pBytes;

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   // We have been told explicitly we are running on a big-endian
   // machine. Network byte order is big endian, so just copy.  There
   // is no issue with alignment here because uInter64 is always
   // aligned (and it doesn't matter if pBytes is aligned).
   pBytes = &uInteger64;

#elif defined(USEFULBUF_CONFIG_HTON)
   // Use system function to handle big- and little-endian. This works
   // on both big- and little-endian machines, but hton() is not
   // always available or in a standard place so it is not used by
   // default. With some compilers and CPUs the code for this is very
   // compact through use of a special swap instruction and on
   // big-endian machines hton() will reduce to nothing.
   uint64_t uTmp = htonll(uInteger64);

   pBytes = &uTmp;

#elif defined(USEFULBUF_CONFIG_LITTLE_ENDIAN) && defined(USEFULBUF_CONFIG_BSWAP)
   // Use built-in function for byte swapping. This usually compiles
   // to an efficient special byte swap instruction. Unlike hton() it
   // does not do this conditionally on the CPU endianness, so this
   // code is also conditional on USEFULBUF_CONFIG_LITTLE_ENDIAN
   uint64_t uTmp = __builtin_bswap64(uInteger64);

   pBytes = &uTmp;

#else
   // Default which works on every CPU with no dependency on anything
   // from the CPU, compiler, libraries or OS.  This always works, but
   // it is usually a little larger and slower than hton().
   uint8_t aTmp[8];

   aTmp[0] = (uInteger64 & 0xff00000000000000) >> 56;
   aTmp[1] = (uInteger64 & 0xff000000000000) >> 48;
   aTmp[2] = (uInteger64 & 0xff0000000000) >> 40;
   aTmp[3] = (uInteger64 & 0xff00000000) >> 32;
   aTmp[4] = (uInteger64 & 0xff000000) >> 24;
   aTmp[5] = (uInteger64 & 0xff0000) >> 16;
   aTmp[6] = (uInteger64 & 0xff00) >> 8;
   aTmp[7] = (uInteger64 & 0xff);

   pBytes = aTmp;
#endif

   // Do the insert
   UsefulOutBuf_InsertData(pMe, pBytes, sizeof(uint64_t), uPos);
}


static inline void UsefulOutBuf_InsertFloat(UsefulOutBuf *pMe,
                                            float f,
                                            size_t uPos)
{
   UsefulOutBuf_InsertUint32(pMe, UsefulBufUtil_CopyFloatToUint32(f), uPos);
}


static inline void UsefulOutBuf_InsertDouble(UsefulOutBuf *pMe,
                                             double d,
                                             size_t uPos)
{
   UsefulOutBuf_InsertUint64(pMe, UsefulBufUtil_CopyDoubleToUint64(d), uPos);
}


static inline void UsefulOutBuf_AppendUsefulBuf(UsefulOutBuf *pMe,
                                                UsefulBufC NewData)
{
   // An append is just a insert at the end
   UsefulOutBuf_InsertUsefulBuf(pMe, NewData, UsefulOutBuf_GetEndPosition(pMe));
}


static inline void UsefulOutBuf_AppendData(UsefulOutBuf *pMe,
                                           const void *pBytes,
                                           size_t uLen)
{
   UsefulBufC Data = {pBytes, uLen};
   UsefulOutBuf_AppendUsefulBuf(pMe, Data);
}


static inline void UsefulOutBuf_AppendString(UsefulOutBuf *pMe,
                                             const char *szString)
{
   UsefulOutBuf_AppendUsefulBuf(pMe, (UsefulBufC){szString, strlen(szString)});
}


static inline void UsefulOutBuf_AppendByte(UsefulOutBuf *pMe,
                                           uint8_t byte)
{
   UsefulOutBuf_AppendData(pMe, &byte, 1);
}


static inline void UsefulOutBuf_AppendUint16(UsefulOutBuf *pMe,
                                             uint16_t uInteger16)
{
   UsefulOutBuf_InsertUint16(pMe, uInteger16, UsefulOutBuf_GetEndPosition(pMe));
}

static inline void UsefulOutBuf_AppendUint32(UsefulOutBuf *pMe,
                                             uint32_t uInteger32)
{
   UsefulOutBuf_InsertUint32(pMe, uInteger32, UsefulOutBuf_GetEndPosition(pMe));
}


static inline void UsefulOutBuf_AppendUint64(UsefulOutBuf *pMe,
                                             uint64_t uInteger64)
{
   UsefulOutBuf_InsertUint64(pMe, uInteger64, UsefulOutBuf_GetEndPosition(pMe));
}


static inline void UsefulOutBuf_AppendFloat(UsefulOutBuf *pMe,
                                            float f)
{
   UsefulOutBuf_InsertFloat(pMe, f, UsefulOutBuf_GetEndPosition(pMe));
}


static inline void UsefulOutBuf_AppendDouble(UsefulOutBuf *pMe,
                                             double d)
{
   UsefulOutBuf_InsertDouble(pMe, d, UsefulOutBuf_GetEndPosition(pMe));
}


static inline int UsefulOutBuf_GetError(UsefulOutBuf *pMe)
{
   return pMe->err;
}


static inline size_t UsefulOutBuf_RoomLeft(UsefulOutBuf *pMe)
{
   return pMe->UB.len - pMe->data_len;
}


static inline int UsefulOutBuf_WillItFit(UsefulOutBuf *pMe, size_t uLen)
{
   return uLen <= UsefulOutBuf_RoomLeft(pMe);
}


static inline int UsefulOutBuf_IsBufferNULL(UsefulOutBuf *pMe)
{
   return pMe->UB.ptr == NULL;
}



static inline void UsefulInputBuf_Init(UsefulInputBuf *pMe, UsefulBufC UB)
{
   pMe->cursor = 0;
   pMe->err    = 0;
   pMe->magic  = UIB_MAGIC;
   pMe->UB     = UB;
}

static inline size_t UsefulInputBuf_Tell(UsefulInputBuf *pMe)
{
   return pMe->cursor;
}


static inline void UsefulInputBuf_Seek(UsefulInputBuf *pMe, size_t uPos)
{
   if(uPos > pMe->UB.len) {
      pMe->err = 1;
   } else {
      pMe->cursor = uPos;
   }
}


static inline size_t UsefulInputBuf_BytesUnconsumed(UsefulInputBuf *pMe)
{
   // Code Reviewers: THIS FUNCTION DOES POINTER MATH

   // Magic number is messed up. Either the structure got overwritten
   // or was never initialized.
   if(pMe->magic != UIB_MAGIC) {
      return 0;
   }

   // The cursor is off the end of the input buffer given.
   // Presuming there are no bugs in this code, this should never happen.
   // If it so, the struct was corrupted. The check is retained as
   // as a defense in case there is a bug in this code or the struct is
   // corrupted.
   if(pMe->cursor > pMe->UB.len) {
      return 0;
   }

   // subtraction can't go neative because of check above
   return pMe->UB.len - pMe->cursor;
}


static inline int UsefulInputBuf_BytesAvailable(UsefulInputBuf *pMe, size_t uLen)
{
   return UsefulInputBuf_BytesUnconsumed(pMe) >= uLen ? 1 : 0;
}


static inline UsefulBufC UsefulInputBuf_GetUsefulBuf(UsefulInputBuf *pMe, size_t uNum)
{
   const void *pResult = UsefulInputBuf_GetBytes(pMe, uNum);
   if(!pResult) {
      return NULLUsefulBufC;
   } else {
      return (UsefulBufC){pResult, uNum};
   }
}


static inline uint8_t UsefulInputBuf_GetByte(UsefulInputBuf *pMe)
{
   const void *pResult = UsefulInputBuf_GetBytes(pMe, sizeof(uint8_t));

   return pResult ? *(uint8_t *)pResult : 0;
}

static inline uint16_t UsefulInputBuf_GetUint16(UsefulInputBuf *pMe)
{
   const uint8_t *pResult = (const uint8_t *)UsefulInputBuf_GetBytes(pMe, sizeof(uint16_t));

   if(!pResult) {
      return 0;
   }

   // See UsefulInputBuf_GetUint64() for comments on this code
#if defined(USEFULBUF_CONFIG_BIG_ENDIAN) || defined(USEFULBUF_CONFIG_HTON) || defined(USEFULBUF_CONFIG_BSWAP)
   uint16_t uTmp;
   memcpy(&uTmp, pResult, sizeof(uint16_t));

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   return uTmp;

#elif defined(USEFULBUF_CONFIG_HTON)
   return ntohs(uTmp);

#else
   return __builtin_bswap16(uTmp);

#endif

#else
   return  ((uint16_t)pResult[0] << 8) + (uint16_t)pResult[1];

#endif
}


static inline uint32_t UsefulInputBuf_GetUint32(UsefulInputBuf *pMe)
{
   const uint8_t *pResult = (const uint8_t *)UsefulInputBuf_GetBytes(pMe, sizeof(uint32_t));

   if(!pResult) {
      return 0;
   }

   // See UsefulInputBuf_GetUint64() for comments on this code
#if defined(USEFULBUF_CONFIG_BIG_ENDIAN) || defined(USEFULBUF_CONFIG_HTON) || defined(USEFULBUF_CONFIG_BSWAP)
   uint32_t uTmp;
   memcpy(&uTmp, pResult, sizeof(uint32_t));

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   return uTmp;

#elif defined(USEFULBUF_CONFIG_HTON)
   return ntohl(uTmp);

#else
   return __builtin_bswap32(uTmp);

#endif

#else
   return ((uint32_t)pResult[0]<<24) +
          ((uint32_t)pResult[1]<<16) +
          ((uint32_t)pResult[2]<<8)  +
           (uint32_t)pResult[3];
#endif
}


static inline uint64_t UsefulInputBuf_GetUint64(UsefulInputBuf *pMe)
{
   const uint8_t *pResult = (const uint8_t *)UsefulInputBuf_GetBytes(pMe, sizeof(uint64_t));

   if(!pResult) {
      return 0;
   }

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN) || defined(USEFULBUF_CONFIG_HTON) || defined(USEFULBUF_CONFIG_BSWAP)
   // pResult will probably not be aligned.  This memcpy() moves the
   // bytes into a temp variable safely for CPUs that can or can't do
   // unaligned memory access. Many compilers will optimize the
   // memcpy() into a simple move instruction.
   uint64_t uTmp;
   memcpy(&uTmp, pResult, sizeof(uint64_t));

#if defined(USEFULBUF_CONFIG_BIG_ENDIAN)
   // We have been told expliclity this is a big-endian CPU.  Since
   // network byte order is big-endian, there is nothing to do.

   return uTmp;

#elif defined(USEFULBUF_CONFIG_HTON)
   // We have been told to use ntoh(), the system function to handle
   // big- and little-endian. This works on both big- and
   // little-endian machines, but ntoh() is not always available or in
   // a standard place so it is not used by default. On some CPUs the
   // code for this is very compact through use of a special swap
   // instruction.

   return ntohll(uTmp);

#else
   // Little-endian (since it is not USEFULBUF_CONFIG_BIG_ENDIAN) and
   // USEFULBUF_CONFIG_BSWAP (since it is not USEFULBUF_CONFIG_HTON).
   // __builtin_bswap64() and friends are not conditional on CPU
   // endianness so this must only be used on little-endian machines.

   return __builtin_bswap64(uTmp);


#endif

#else
   // This is the default code that works on every CPU and every
   // endianness with no dependency on ntoh().  This works on CPUs
   // that either allow or do not allow unaligned access. It will
   // always work, but usually is a little less efficient than ntoh().

   return   ((uint64_t)pResult[0]<<56) +
            ((uint64_t)pResult[1]<<48) +
            ((uint64_t)pResult[2]<<40) +
            ((uint64_t)pResult[3]<<32) +
            ((uint64_t)pResult[4]<<24) +
            ((uint64_t)pResult[5]<<16) +
            ((uint64_t)pResult[6]<<8)  +
            (uint64_t)pResult[7];
#endif
}


static inline float UsefulInputBuf_GetFloat(UsefulInputBuf *pMe)
{
   uint32_t uResult = UsefulInputBuf_GetUint32(pMe);

   return uResult ? UsefulBufUtil_CopyUint32ToFloat(uResult) : 0;
}


static inline double UsefulInputBuf_GetDouble(UsefulInputBuf *pMe)
{
   uint64_t uResult = UsefulInputBuf_GetUint64(pMe);

   return uResult ? UsefulBufUtil_CopyUint64ToDouble(uResult) : 0;
}


static inline int UsefulInputBuf_GetError(UsefulInputBuf *pMe)
{
   return pMe->err;
}

#ifdef __cplusplus
}
#endif

#endif  // _UsefulBuf_h


