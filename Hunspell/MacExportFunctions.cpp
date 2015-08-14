#include "hunspell/hunspell.hxx"
#include "hyphen/hyphen.h"
#include <locale.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include "NHunspellExtensions.h"
#define DLLEXPORT extern "C" 

static size_t usslen(unsigned short *str) { size_t s = 0; while (*str != 0) s++, str++; return s; }

class Marshaler
{
  // The methods aren't multi threaded, reentrant or whatever so a encapsulated buffer can be used for performance reasons
	void * marshalBuffer;
	size_t marshalBufferSize;

	void * wordBuffer;
	size_t wordBufferSize;

	void * word2Buffer;
	size_t word2BufferSize;

	void * affixBuffer;
	size_t affixBufferSize;

public:

	iconv_t cd;
	iconv_t cdReverse;

	inline Marshaler(char *encoding)
	{
		marshalBuffer = 0;
		marshalBufferSize = 0;

		wordBuffer = 0;
		wordBufferSize = 0;
		
		word2Buffer = 0;
		word2BufferSize = 0;

		affixBuffer = 0;
		affixBufferSize = 0;

        cd = iconv_open(encoding, "UTF-16LE");
        cdReverse = iconv_open("UTF-16LE", encoding);
	}

	inline ~Marshaler()
	{
		if( marshalBuffer != 0 )
			free( marshalBuffer );

		if( wordBuffer != 0 )
			free( wordBuffer );

		if( word2Buffer != 0 )
			free( word2Buffer );

		if( affixBuffer != 0 )
			free( affixBuffer );

		iconv_close(cd);
		iconv_close(cdReverse);
	}	

	// Buffer for marshaling string lists back to .NET
	void * GetMarshalBuffer(size_t size) 
	{
		if(size < marshalBufferSize )
			return marshalBuffer;

		if( marshalBufferSize == 0 )
		{
			size += 128;
			marshalBuffer = malloc( size );
		}
		else
		{
			size += 256;
			marshalBuffer = realloc(marshalBuffer, size );
		}
		
		marshalBufferSize = size;
		return marshalBuffer;

	}
	
	// Buffer for the mutibyte representation of a word
	void * GetWordBuffer(size_t size) 
	{
		if(size < wordBufferSize )
			return wordBuffer;

		if( wordBufferSize == 0 )
		{
			size += 32;
			wordBuffer = malloc( size );
		}
		else
		{
			size += 64;
			wordBuffer = realloc(wordBuffer, size );
		}
		
		wordBufferSize = size;
		return wordBuffer;
	}

		// Buffer for the mutibyte representation of a word
	void * GetWord2Buffer(size_t size) 
	{
		if(size < word2BufferSize )
			return word2Buffer;

		if( word2BufferSize == 0 )
		{
			size += 32;
			word2Buffer = malloc( size );
		}
		else
		{
			size += 64;
			word2Buffer = realloc(wordBuffer, size );
		}
		
		word2BufferSize = size;
		return word2Buffer;
	}

	inline char * GetWordBuffer( unsigned short * unicodeString )
	{
		size_t inputsize = (usslen(unicodeString) + 1) * sizeof (unsigned short);
		size_t buffersize = inputsize * 4;
		char * buffer = (char *) GetWordBuffer( buffersize );
		int ic = iconv(cd, (char **)&unicodeString, &inputsize, &buffer, &buffersize);
		return (char *)wordBuffer;
	}

	inline char * GetWord2Buffer( unsigned short * unicodeString )
	{
		size_t inputsize = (usslen(unicodeString) + 1) * sizeof (unsigned short);
		size_t buffersize = inputsize * 4;
		char * buffer = (char *) GetWord2Buffer( buffersize );
		iconv(cd, (char **)&unicodeString, &inputsize, &buffer, &buffersize);
		return (char *)word2Buffer;
	}

	// Buffer for the mutibyte representation of an affix
	void * GetAffixBuffer(size_t size) 
	{
		if(size < affixBufferSize )
			return affixBuffer;

		if( affixBufferSize == 0 )
		{
			size += 32;
			affixBuffer = malloc( size );
		}
		else
		{
			size += 64;
			affixBuffer = realloc(affixBuffer, size );
		}
		
		affixBufferSize = size;
		return affixBuffer;
	}

	inline char * GetAffixBuffer( unsigned short * unicodeString )
	{
		size_t inputsize = (usslen(unicodeString) + 1) * sizeof (unsigned short);
		size_t buffersize = inputsize * 4;
		char * buffer = (char *) GetAffixBuffer( buffersize );
		iconv(cd, (char **)&unicodeString, &inputsize, &buffer, &buffersize);
		return (char *)affixBuffer;
	}
};

class NHunspell: public Hunspell, public Marshaler
{
public:
	inline NHunspell(const char *affpath, const char *dpath, const char * key = 0) : Hunspell(affpath,dpath,key), Marshaler(get_dic_encoding()) 
	{
	}
};

class NHyphen: public Marshaler
{
	HyphenDict * dictionary;

public:
	inline NHyphen(HyphenDict * d) 
		: dictionary(d), Marshaler(d->cset)
	{
	}

	inline ~NHyphen()
	{
		if( dictionary != 0 )
			hnj_hyphen_free( dictionary );
	}
	
	inline int Hyphenate(  const char *word, int word_size, char * hyphens,
        char *hyphenated_word, char *** rep, int ** pos, int ** cut )
	{
		return hnj_hyphen_hyphenate2(dictionary,word,word_size,hyphens,hyphenated_word, rep, pos, cut); 
	}
};

inline char * AllocMultiByteBuffer( unsigned short * unicodeString )
{
	iconv_t ic = iconv_open("UTF-8", "UTF-16LE");
	size_t inputsize = (usslen(unicodeString) + 1) * sizeof (unsigned short);
	size_t buffersize = inputsize * 4;
	char * buffer = (char *) malloc( buffersize ), *bufferPtr = buffer;
	iconv(ic, (char **)&unicodeString, &inputsize, &bufferPtr, &buffersize);
	iconv_close(ic);
	return buffer;

}

/************************* Export Functions **************************************************************/

DLLEXPORT NHunspell * HunspellInit(void * affixBuffer, size_t affixBufferSize, void * dictionaryBuffer, size_t dictionaryBufferSize, unsigned short * key)
{
	char * key_buffer = 0;
	if( key != 0 )
		key_buffer = AllocMultiByteBuffer(key);

	MemoryBufferInformation affixBufferInfo;
	affixBufferInfo.magic = magicConstant;
	affixBufferInfo.buffer = affixBuffer;
	affixBufferInfo.bufferSize = affixBufferSize;

	MemoryBufferInformation dictionaryBufferInfo;
	dictionaryBufferInfo.magic = magicConstant;
	dictionaryBufferInfo.buffer = dictionaryBuffer;
	dictionaryBufferInfo.bufferSize = dictionaryBufferSize;

	NHunspell * result = new NHunspell((const char *) &affixBufferInfo, (const char *) &dictionaryBufferInfo,key_buffer); 

	if( key_buffer != 0 )
		free( key_buffer );

	return result;
}


DLLEXPORT void HunspellFree(NHunspell * handle )
{
	delete handle;
}

DLLEXPORT bool HunspellAdd(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = handle->GetWordBuffer(word);
	int success = handle->add(word_buffer);
	return success != 0;
}

DLLEXPORT bool HunspellRemove(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = handle->GetWordBuffer(word);
	int success = handle->remove(word_buffer);
	return success != 0;
}

DLLEXPORT bool HunspellAddWithAffix(NHunspell * handle, unsigned short * word, unsigned short * example )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	char * example_buffer = ((NHunspell *) handle)->GetWord2Buffer(example);
	bool success =  handle->add_with_affix(word_buffer,example_buffer) != 0;
	return success;
}

DLLEXPORT bool HunspellSpell(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	bool correct = ((NHunspell *) handle)->spell(word_buffer) != 0;
	return correct;
}

void *MarshalList(NHunspell * handle, char **wordList, int wordCount)
{
	// Cacculation of the Marshalling Buffer. 
    // Layout: {Pointers- zero terminated}{Wide Strings - zero terminated}
	size_t bufferSize = (wordCount + 1) * sizeof (unsigned short **);
	for( int i = 0; i < wordCount; ++ i )
		bufferSize += (strlen(wordList[i]) + 1) * 2 * sizeof( unsigned short);		
		
	char * marshalBuffer = (char *) ((NHunspell *) handle)->GetMarshalBuffer( bufferSize );
	unsigned short ** pointer = (unsigned short ** ) marshalBuffer;
	unsigned short * buffer = (unsigned short * ) (marshalBuffer + (wordCount + 1) * sizeof (unsigned short **));
	for( int i = 0; i < wordCount; ++ i )
	{
		*pointer = buffer;

		char *inputbuffer = wordList[i];
		size_t inputsize = strlen(wordList[i]) + 1;
		iconv(handle->cdReverse, &inputbuffer, &inputsize, (char**)&buffer, &bufferSize);
		
		// Prepare pointers for the next string
		++pointer;
	}

	// Zero terminate the pointer list
	*pointer = 0;

	((NHunspell *) handle)->free_list(&wordList, wordCount);	

	return marshalBuffer;
}

DLLEXPORT void * HunspellSuggest(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	char ** wordList;
	int  wordCount = ((NHunspell *) handle)->suggest(&wordList, word_buffer);
	return MarshalList(handle, wordList, wordCount);
}


DLLEXPORT void * HunspellAnalyze(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	char ** morphList;
	int  morphCount = ((NHunspell *) handle)->analyze(&morphList, word_buffer);
	return MarshalList(handle, morphList, morphCount);
}

DLLEXPORT void * HunspellStem(NHunspell * handle, unsigned short * word )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	char ** stemList;
	int  stemCount = ((NHunspell *) handle)->stem(&stemList, word_buffer);
	return MarshalList(handle, stemList, stemCount);
}

DLLEXPORT void * HunspellGenerate(NHunspell * handle, unsigned short * word, unsigned short * word2 )
{
	char * word_buffer = ((NHunspell *) handle)->GetWordBuffer(word);
	char * word2_buffer = ((NHunspell *) handle)->GetWord2Buffer(word2);
	char ** stemList;
	int  stemCount = ((NHunspell *) handle)->generate(&stemList, word_buffer, word2_buffer);
	return MarshalList(handle, stemList, stemCount);
}

DLLEXPORT void * HyphenInit(void * dictionaryBuffer, size_t dictionaryBufferSize)
{

	MemoryBufferInformation dictionaryBufferInfo;
	dictionaryBufferInfo.magic = magicConstant;
	dictionaryBufferInfo.buffer = dictionaryBuffer;
	dictionaryBufferInfo.bufferSize = dictionaryBufferSize;

	void * result = new NHyphen(hnj_hyphen_load((const char *) &dictionaryBufferInfo)); 
	return result;
}

DLLEXPORT void HyphenFree(NHyphen * handle )
{
	delete handle;
}


DLLEXPORT void * HyphenHyphenate(NHyphen * handle, unsigned short * word )
{
	size_t wordChars = usslen( word );
	char * word_buffer = handle->GetWordBuffer(word);
	int wordBufferSize = strlen( word_buffer );

	char * hyphenPoints = new char[wordBufferSize + 5];
	char * hyphenatedWord = new char[wordBufferSize * 2];

	char **rep = 0;
	int *pos = 0;
	int *cut = 0;

	handle->Hyphenate(word_buffer,wordBufferSize,hyphenPoints,hyphenatedWord,&rep,&pos,&cut);

	size_t hyphenatedWordWChars= (strlen(hyphenatedWord) + 1) * 2; // MultiByteToWideChar(handle->CodePage,0,hyphenatedWord,-1,0,0);

	// Marshal Buffer Layout:
	// Pointer to hyphenated word (in Data)
	// Pointer to hyphenation points array (in Data)
	// Pointer to REP Data (wchar_t ptr Array)
	// Pointer to POS Data (int Array)
	// Pointer to CUT Data (int Array)
	// Data - Hyphenated Word
	// Data - Hyphenation Points


	size_t bufferSize = sizeof (unsigned short *); // ptr to hyphenated word
	bufferSize += sizeof(unsigned char **); // pointer to hyphenation points array
	bufferSize += sizeof(unsigned short **); // pointer to REP Data
	bufferSize += sizeof(int *); // pointer to POS Data
	bufferSize += sizeof(int *); // pointer to CUT Data

	bufferSize += hyphenatedWordWChars * sizeof( unsigned short); // hyphenated word
	bufferSize += wordChars * sizeof( unsigned char); // hyphenation points;
	if( rep != 0 )
	{
		bufferSize += wordChars * sizeof( unsigned short *); // REP Data
		bufferSize += wordChars * sizeof( int); // POS Data
		bufferSize += wordChars * sizeof( int); // CUT Data

		for( int index = 0; index < wordChars; ++index )
		{
			if( rep[index] != 0 )
			{
  				size_t repTextChars=(strlen(rep[index]) + 1) * 2; //MultiByteToWideChar(handle->CodePage,0,rep[index],-1,0,0);
				bufferSize += repTextChars * sizeof(unsigned short);
			}
		}
	}


	unsigned char * marshalBuffer = (unsigned char *) handle->GetMarshalBuffer( bufferSize );
	unsigned char * currentBufferPos = marshalBuffer;

	unsigned short ** hyphenatedWordPtr = (unsigned short ** ) currentBufferPos;
    currentBufferPos += sizeof( unsigned short * );

	unsigned char ** hyphenationPointsPtr = (unsigned char ** ) currentBufferPos;
    currentBufferPos += sizeof( unsigned char ** );

	unsigned short *** repPtr = (unsigned short *** ) currentBufferPos;
    currentBufferPos += sizeof( unsigned short *** );

	int ** posPtr = (int ** ) currentBufferPos;
    currentBufferPos += sizeof( int ** );

	int ** cutPtr = (int ** ) currentBufferPos;
    currentBufferPos += sizeof( int ** );

	// Hyphenated Word
	*hyphenatedWordPtr = (unsigned short *) currentBufferPos;
	size_t inputsize = strlen(hyphenatedWord) + 1;
	iconv(handle->cdReverse, &hyphenatedWord, &inputsize, (char**)&currentBufferPos, &bufferSize);
	//MultiByteToWideChar(handle->CodePage,0,hyphenatedWord,-1,*hyphenatedWordPtr,hyphenatedWordWChars);
    //currentBufferPos += sizeof( unsigned short ) * hyphenatedWordWChars;

	// Hyphenation Points
	*hyphenationPointsPtr = (unsigned char *) currentBufferPos;
	currentBufferPos += wordChars * sizeof( unsigned char);

	if (rep)
	{
		*repPtr = (unsigned short ** ) currentBufferPos;
		currentBufferPos += wordChars * sizeof(unsigned short ** );

		*posPtr = (int *) currentBufferPos;
		currentBufferPos += wordChars * sizeof(int *);

		*cutPtr = (int *) currentBufferPos;
		currentBufferPos += wordChars * sizeof(int *);
	}
	else
	{
		*repPtr = 0;
		*posPtr = 0;
		*cutPtr = 0;
	}

	int wideCharIndex = 0;
	for( int multByteIndex = 1; multByteIndex < wordBufferSize; ++ multByteIndex)
	{
		// Multibyte Start Bytes 0XXXXXXX oder 11XXXXXX ( Follow Bytes : 10XXXXXX )
		if( (unsigned char)( word_buffer[multByteIndex] ) < 0x80 || (unsigned char)( word_buffer[multByteIndex] ) >= 0xC0 )
		{
			(*hyphenationPointsPtr)[wideCharIndex] = hyphenPoints[multByteIndex -1] - '0';

			if( rep )
			{
				if( rep[multByteIndex] )
				{
					size_t inputsize = strlen(rep[multByteIndex]) + 1;
					char *inputbuffer = rep[multByteIndex];
					(*repPtr)[wideCharIndex] = (unsigned short *)currentBufferPos;
					iconv(handle->cdReverse, &inputbuffer, &inputsize, (char**)&currentBufferPos, &bufferSize);

				}
				else 
					(*repPtr)[wideCharIndex] = 0;

				(*posPtr)[wideCharIndex] = pos[multByteIndex];
				(*cutPtr)[wideCharIndex] = cut[multByteIndex];

			}
			++wideCharIndex;
		}
	}

    if (rep)
	{
		for (int i = 0; i < wordChars - 1; i++) 
		{
			if (rep[i]) 
				free(rep[i]);
		}

		free(rep);
		free(pos);
		free(cut);
	}

	return marshalBuffer;
}
