#ifndef BIGLIB_H
#define BIGLIB_H

typedef unsigned long DWORD;
typedef unsigned long BIG;

/* creates a bignum and initializes it with the value InitValue
 * returns a pointer to the bignum */
extern DWORD __stdcall _BigCreate(DWORD InitValue);

/* destroys the bignum Big */
extern DWORD __stdcall _BigDestroy(DWORD BigAddr);

/* copies BigSrc to BigDest */
extern DWORD __stdcall _BigCopy(DWORD BigSrc, DWORD BigDest);

/* compares BigA and BigB
 *	returns 1 if BigA > BigB
 *	0 if BigA = BigB
 *	-1 if BigA < BigB
 */
extern DWORD __stdcall _BigCompare(DWORD BigA, DWORD BigB);

/* compares BigA and 32-bit Value
 *	returns 1 if BigA > Value
 *	0 if BigA = Value
 *	-1 if BigA < Value
 */
extern DWORD __stdcall _BigCompare32(DWORD BigA, DWORD Value);

/* fills Big with Len bytes of Data in base 256 */
extern DWORD __stdcall _BigInB256(unsigned char *Data, DWORD Len, DWORD Big);

/* fills Big with 32-bit Value */
extern DWORD __stdcall _BigIn32(DWORD Value, DWORD Big);

/* fills Big with the null-terminated string String in base Base (Base is 2-62)
 * returns 0 if no error, -1 if base not supported */
extern DWORD __stdcall _BigIn(unsigned char *string, DWORD base, DWORD big);

/* fills Big with Len bytes of data at Buffer in base Base (Base is 2-256)
	returns 0 if no error, -1 if base not supported */
extern DWORD __stdcall _BigInBytes(unsigned char *buffer, DWORD Len, DWORD base, DWORD Big);

/* fills Buffer with the value of Big in base 256
   returns the length of the string */
extern DWORD __stdcall _BigOutB256(DWORD Big, unsigned char *buffer);

/* fills Buffer with the value of Big in base 16 as a null-terminated string
   returns the length of the string not including the terminating null character */
extern DWORD __stdcall _BigOutB16(DWORD Big, unsigned char *buffer);

/* fills Buffer with the value of Big in base Base as a null-terminated string (Base is 2-62 or 256)
   returns the length of the string not including the terminating null character,
   -1 if base not supported*/
extern DWORD __stdcall _BigOut(DWORD big, DWORD base, unsigned char *buffer);

/* fills Buffer with the value of Big in base Base as data (Base is 2-256)
   returns the length of the data not including the terminating null character,
   -1 if base not supported */
extern DWORD __stdcall _BigOutBytes(DWORD Big, DWORD dtBase, unsigned char *buffer);

/* BigY = BigX + Value */
extern DWORD __stdcall _BigAdd32(DWORD BigX, DWORD Value, DWORD BigY);

/* BigZ = BigX + BigY */
extern DWORD __stdcall _BigAdd(DWORD BigX, DWORD BigY, DWORD BigZ);

/* BigY = |BigX - Value| */
extern DWORD __stdcall _BigSub32(DWORD BigX, DWORD Value, DWORD BigY);

/* BigZ = |BigX - BigY| */
extern DWORD __stdcall _BigSub(DWORD BigX, DWORD BigY, DWORD BigZ);

/* BigY = BigX >> 1
   returns the previous lower bit */
extern DWORD __stdcall _BigShr(DWORD BigX, DWORD BigY);

/* BigY = BigX << 1 */
extern DWORD __stdcall _BigShl(DWORD BigX, DWORD BigY);

/* BigY = BigX * Value */
extern DWORD __stdcall _BigMul32(DWORD BigX, DWORD Value, DWORD BigY);

/* BigZ = BigX * BigY */
extern DWORD __stdcall _BigMul(DWORD BigX, DWORD BigY, DWORD BigZ);

/* BigY = BigX / Value
	BigR = BigX mod Value
	if pointer to BigY is 0, only remainder is returned
	if pointer to BigR is 0, only quotient is returned
	returns remainder, -1 if division by zero
*/
extern DWORD __stdcall _BigDiv32(DWORD BigX, DWORD Value, DWORD BigY, DWORD BigR);

/* BigZ = BigX / BigY
	BigR = BigX mod BigY
	if pointer to BigZ is 0, only remainder is returned
	if pointer to BigR is 0, only quotient is returned
	returns 0 if no error
	returns -1 if division by zero */
extern DWORD __stdcall _BigDiv(DWORD BigX, DWORD BigY, DWORD BigZ, DWORD BigR);

/* BigZ = BigX mod BigY
	returns 0 if no error
	returns -1 if division by zero */
extern DWORD __stdcall _BigMod(DWORD BigX, DWORD BigY, DWORD BigZ);

/* BigZ = BigX * BigY mod BigN
	returns 0 if no error
	returns -1 if division by zero */
extern DWORD __stdcall _BigMulMod(DWORD BigX, DWORD BigY, DWORD BigN, DWORD BigZ);

/* BigZ = BigX ^ E mod BigN
	returns 0 if no error
	returns -1 if division by zero */
extern DWORD __stdcall _BigPowMod32(DWORD BigX, DWORD E, DWORD BigN, DWORD BigZ);

/* BigZ = BigX ^ BigY mod BigN
	returns 0 if no error
	returns -1 if division by zero */
extern DWORD __stdcall _BigPowMod(DWORD BigX, DWORD BigY, DWORD BigN, DWORD BigZ);

/* BigZ = g.c.d.(BigX, BigY) */
extern DWORD __stdcall _BigGcd(DWORD BigX, DWORD BigY, DWORD BigZ);

#endif
