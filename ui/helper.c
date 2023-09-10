/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>

#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "font.h"
#include "ui/helper.h"
#include "ui/inputbox.h"

void UI_GenerateChannelString(char *pString, const uint8_t Channel)
{
	unsigned int i;

	if (gInputBoxIndex == 0)
	{
		sprintf(pString, "CH-%02d", Channel + 1);
		return;
	}

	pString[0] = 'C';
	pString[1] = 'H';
	pString[2] = '-';
	for (i = 0; i < 2; i++)
		pString[i + 3] = (gInputBox[i] == 10) ? '-' : gInputBox[i] + '0';
}

void UI_GenerateChannelStringEx(char *pString, const bool bShowPrefix, const uint8_t ChannelNumber)
{
	if (gInputBoxIndex)
	{
		unsigned int i;
		for (i = 0; i < 3; i++)
			pString[i] = (gInputBox[i] == 10) ? '-' : gInputBox[i] + '0';
		return;
	}

	if (bShowPrefix)
		sprintf(pString, "CH-%03d", ChannelNumber + 1);
	else
	if (ChannelNumber == 0xFF)
		strcpy(pString, "NULL");
	else
		sprintf(pString, "%03d", ChannelNumber + 1);
}

void UI_PrintString(const char *pString, uint8_t Start, uint8_t End, uint8_t Line, uint8_t Width, bool bCentered)
{
	size_t i;
	size_t Length = strlen(pString);

	if (bCentered)
		Start += (((End - Start) - (Length * Width)) + 1) / 2;

	for (i = 0; i < Length; i++)
	{
		if (pString[i] >= ' ' && pString[i] < 127)
		{
			const unsigned int Index = pString[i] - ' ';
			const unsigned int ofs   = (unsigned int)Start + (i * Width);
			memcpy(gFrameBuffer[Line + 0] + ofs, &gFontBig[Index][0], 8);
			memcpy(gFrameBuffer[Line + 1] + ofs, &gFontBig[Index][8], 8);
		}
	}
}

void UI_PrintStringSmall(const char *pString, uint8_t Start, uint8_t End, uint8_t Line, bool bCentered)
{
	const size_t Length = strlen(pString);
	size_t       i;

	if (bCentered)
		Start += (((End - Start) - (Length * 8)) + 1) / 2;

	for (i = 0; i < Length; i++)
	{
		if (pString[i] >= 32)
		{
			const unsigned int Index = ((unsigned int)pString[i] - 32) * 5;
			if (Index < sizeof(gFont5x7))
			{
				const unsigned int ofs = (unsigned int)Start + (i * 8);
				memcpy(gFrameBuffer[Line] + ofs, &gFont5x7[Index], 5);
			}
		}
	}
}

void UI_DisplayFrequency(const char *pDigits, uint8_t X, uint8_t Y, bool bDisplayLeadingZero, bool bFlag)
{
	const unsigned int char_width  = 13;
	uint8_t           *pFb0        = gFrameBuffer[Y] + X;
	uint8_t           *pFb1        = pFb0 + 128;
	bool               bCanDisplay = false;
	unsigned int       i           = 0;
	
	// MHz
	while (i < 3)
	{
		const unsigned int Digit = pDigits[i++];
		if (bDisplayLeadingZero || bCanDisplay || Digit > 0)
		{
			bCanDisplay = true;
			memcpy(pFb0, gFontBigDigits[Digit],              char_width);
			memcpy(pFb1, gFontBigDigits[Digit] + char_width, char_width);
		}
		else
		if (bFlag)
		{
			pFb0 -= 6;
			pFb1 -= 6;
		}
		pFb0 += char_width;
		pFb1 += char_width;
	}

	// decimal point
	*pFb1 = 0x60; pFb0++; pFb1++;
	*pFb1 = 0x60; pFb0++; pFb1++;
	*pFb1 = 0x60; pFb0++; pFb1++;
	
	// kHz
	while (i < 6)
	{
		const unsigned int Digit = pDigits[i++];
		memcpy(pFb0, gFontBigDigits[Digit],              char_width);
		memcpy(pFb1, gFontBigDigits[Digit] + char_width, char_width);
		pFb0 += char_width;
		pFb1 += char_width;
	}
}

void UI_DisplayFrequencySmall(const char *pDigits, uint8_t X, uint8_t Y, bool bDisplayLeadingZero)
{
	const unsigned int char_width  = 7;
	uint8_t           *pFb         = gFrameBuffer[Y] + X;
	bool               bCanDisplay = false;
	unsigned int       i           = 0;

	// MHz
	while (i < 3)
	{
		const unsigned int Digit = pDigits[i++];
		if (bDisplayLeadingZero || bCanDisplay || Digit > 0)
		{
			bCanDisplay = true;
			memcpy(pFb, gFontSmallDigits[Digit], char_width);
			pFb += char_width;
		}
	}

	// decimal point
	pFb++;
	pFb++;
	*pFb++ = 0x60;
	*pFb++ = 0x60;
	pFb++;

	// kHz
	while (i < 8)
	{
		const unsigned int Digit = pDigits[i++];
		memcpy(pFb, gFontSmallDigits[Digit], char_width);
		pFb += char_width;
	}
}

void UI_DisplaySmallDigits(uint8_t Size, const char *pString, uint8_t X, uint8_t Y)
{
	const unsigned int char_width  = 7;
	unsigned int       x = X;
	unsigned int       i;
	for (i = 0; i < Size; i++, x += char_width)
		memcpy(gFrameBuffer[Y] + x, gFontSmallDigits[(unsigned int)pString[i]], char_width);
}