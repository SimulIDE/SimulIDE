/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2007 Roy R Rankin

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/

// T. Scott Dattalo

// Portions of this file were obtained from:

/* intel16.c - read an intel hex file */
/* Copyright (c) 1994 Ian King */

#include <stdio.h>
#include <ctype.h>

#include "pic-processor.h"
#include "hexutils.h"


//------------------------------------------------------------------------
// IntelHexProgramFileType -- constructor
//
// When a IntelHexProgramFileType is instantiated, it will get placed
// on to a 'ProgramFileType' list. This list is then used (ultimately)
// by the 'load' command to load files of this type.

IntelHexProgramFileType::IntelHexProgramFileType()
{
   // Determine endianess of current processor
   short word = 0x4321;
   isBigEndian = (*(char *)& word) != 0x21;
}

void IntelHexProgramFileType::putachar (FILE * file, unsigned char c)
{
    checksum += c;
    fprintf(file, "%02X", c);
}

/* write big endian word */
void IntelHexProgramFileType::write_be_word(FILE * file, int w)
{
        unsigned short nw =hton16(w);
        putachar(file, nw & 0xff );
        putachar(file, (nw >> 8) & 0xff);
}

/* write little endian word */
void IntelHexProgramFileType::write_le_word(FILE * file, int w)
{
        unsigned short nw =hton16(w);
        putachar(file, (nw >> 8) & 0xff);
        putachar(file, nw & 0xff );
}

int IntelHexProgramFileType::getachar (FILE * file)
{
  int c;
  do c = fgetc (file);
  while (c == '\r');                /* strip LF out of MSDOS files */

  return c;
}

unsigned char IntelHexProgramFileType::getbyte (FILE * file)
{
  unsigned char byte;
  uint data;

  if (fscanf (file, "%02x", &data) != 1) return 0;

  byte = data & 0xff;
  checksum += byte;                /* all the bytes are used in the checksum */
  /* so here is the best place to update it */
  return byte;
}

/* read big endian word */
int IntelHexProgramFileType::read_be_word(FILE * file)
{
        int w;
        w = getbyte(file);
        w |= getbyte(file) << 8;
        return(ntoh16(w));
}

/* read little endian word */
int IntelHexProgramFileType::read_le_word(FILE * file)
{
        int w;
        w = getbyte(file) << 8;
        w |= getbyte(file);
        return(ntoh16(w));
}

uint IntelHexProgramFileType::getword(FILE *file)
{
  unsigned char lo = getbyte(file);
  return ((getbyte(file) << 8) | lo);
}

int IntelHexProgramFileType::LoadProgramFile( Processor **pProcessor, const char *pFilename,
                                           FILE *inputfile, const char *pProcessorName )
{
  if(*pProcessor == NULL) {
    // Need to determine processor from file.
    // for now return error.
    return ERR_NEED_PROCESSOR_SPECIFIED;
  }
  // assume no configuration word is in the hex file.
  (*pProcessor)->set_config_word((*pProcessor)->config_word_address(),0xffff);
  int iReturn;
  if((iReturn = readihex16( *pProcessor, inputfile)) != SUCCESS) // No errors were found in the hex file.
  {
    (*pProcessor)->set_frequency(10e6);
    (*pProcessor)->reset(POR_RESET);
    (*pProcessor)->simulation_mode = eSM_STOPPED;

    return SUCCESS;
  }
  return iReturn;
}

/*
 *         writeihexN outputs Register structures in Intel hex format
 *
 *         bytes_per_word - number of bytes of register to output
 *         fr -             pointer to array of Register structures
 *         size -           number of Register structures to output
 *         file -                 File descriptor for output
 *         out_base -         address offset in words for first register
 *
 *         If bytes_per_word == 1 this writes ihex8 format
 *            bytes_per_word == 2 this writes ihex16 format
 *                    address is big endian, data little endian
 *                    byte address (word_address*2) is written
 */
void IntelHexProgramFileType::writeihexN( int bytes_per_word, Register **fr, int32_t size, FILE *file, int32_t out_base )
{
  int32_t extended_address;
  int32_t address;
  int rec_size = 32;        // output record size in bytes
  int i;
  int j = 0;

  if (fr == NULL || file == NULL || size <= 0 || (bytes_per_word != 1 && bytes_per_word != 2))
      return;

  out_base <<= (bytes_per_word - 1); // convert word address to byte address

  extended_address = out_base >> 16;
  address = out_base & 0xffff;

  if (extended_address)
        fprintf( file, ":02000004%04X%02X\n", extended_address, ext_csum(extended_address) );

  while( j < size)
  {
        if (rec_size > (size - j) * bytes_per_word)
                rec_size = (size -j) * bytes_per_word;
        if (address & 0x10000)
        {
                extended_address++;
                address &= 0xffff;
                fprintf(file, ":02000004%04X%02X\n",
                    extended_address,
                    ext_csum(extended_address)
                );
        }
        fprintf(file, ":%02X", rec_size);
        checksum = rec_size;
        write_be_word(file, address);
        putachar(file, 0);
        for(i = 0; i < rec_size; j++, i += bytes_per_word)
        {
            if (bytes_per_word == 2)
                write_le_word(file, fr[j]->get_value());
            else
                putachar(file, fr[j]->get_value());
        }
        fprintf(file, "%02X\n", (-checksum) & 0xff);
        address += rec_size;
   }
   fprintf(file, ":00000001FF\n");
}

int IntelHexProgramFileType::readihex16( Processor *pProcessor, FILE * file )
{
  int extended_address = 0;
  int address;
  int linetype = 0;
  int bytesthisline;
  int i;
  int lineCount = 1;
  int csby;
  Processor *& cpu = pProcessor;

  while (1) {
      if (getachar (file) != ':') {
        printf ("Need a colon as first character in each line\n");
        printf ("Colon missing in line %d\n", lineCount);
        //exit (1);
        return ERR_BAD_FILE;
      }

      checksum = 0;
      bytesthisline = getbyte (file);
      address = read_be_word(file) / 2;

      /* wierdness of INHX16! address different */
      /* endian-ness and 2x too big */

/*        The address is big endian and is a byte address
 *        which is why gpsim which uses word addresses considers
 *         them 2x to big. This file assumes the data is little endian.
 *         RRR
 */
      linetype = getbyte (file);        /* 0 for data, 1 for end  */

      switch (linetype ) 
      {
          case 0:      // Data record
            {
              unsigned char buff[256];
              bytesthisline &= 0xff;
              for (i = 0; i < bytesthisline; i++)
                buff[i] = getbyte(file);

              cpu->init_program_memory_at_index(address|extended_address,
                                                buff, bytesthisline);
            }
            break;
          case 1:      // End of hex file
            return SUCCESS;
          case 4:      // Extended address
            {
              extended_address = read_be_word(file) << 15;
              printf ("Extended linear address %x %x\n",
                      address, extended_address);

            }
            break;
          default:
            printf ("Error! Unknown record type! %d\n", linetype);
            return ERR_BAD_FILE;
      }
      csby = getbyte (file);        /* get the checksum byte */
      /* this should make the checksum zero */
      /* due to side effect of getbyte */

      if (checksum)
      {
        printf ("Checksum error in input file.\n");
        printf ("Got 0x%02x want 0x%02x at line %d\n",
          csby, (0 - checksum) & 0xff, lineCount);
        return ERR_BAD_FILE;
      }

      (void) getachar( file );        /* lose <return> */

      lineCount++;
    }
  return SUCCESS;
}
/*
 *         readihexN loads Register structures from file in Intel hex format
 *
 *         bytes_per_word - number of bytes of register to output (1 & 2 supported)
 *         fr -             pointer to array of Register structures
 *         size -           number of Register structures to output
 *         file -                 File descriptor for output
 *         offset -         address offset in words for first register
 *
 *         If bytes_per_word == 1 this reads ihex8 format
 *            bytes_per_word == 2 this reads ihex16 format
 *                    address is big endian, data little endian
 *                    byte address (word address*2) is read
 */
int IntelHexProgramFileType::readihexN (int bytes_per_word, Register **fr, int32_t size, FILE * file, int32_t offset)
{
  int extended_address = 0;
  int address;
  int linetype = 0;
  int wordsthisline, bytesthisline;
  int i;
  int lineCount = 1;
  int csby;

  while (1) {
      if (getachar (file) != ':') {
        printf ("Need a colon as first character in each line\n");
        printf ("Colon missing in line %d\n", lineCount);
        return ERR_BAD_FILE;
      }

      checksum = 0;
      bytesthisline = getbyte (file);
      wordsthisline = bytesthisline  / bytes_per_word;
      address = read_be_word(file);

      linetype = getbyte (file);        /* 0 for data, 1 for end  */

      switch (linetype ) {
      case 0:      // Data record
        {
          int data;
          int32_t index = (extended_address | address) / bytes_per_word - offset;

          if (index < 0)
          {
                printf("Address 0x%x less than offset 0x%x line %d\n",
                        (extended_address | address) / bytes_per_word,
                        offset,
                        lineCount );
                return ERR_BAD_FILE;
          }
          if (index + wordsthisline > size)
          {
                printf("Index %d exceeds size %d at line %d\n",
                        index + wordsthisline,
                        size,
                        lineCount );
                return ERR_BAD_FILE;
          }
          for (i = 0; i < wordsthisline; i++)
          {
              if (bytes_per_word == 1)
                data = getbyte(file);
              else
                data = read_le_word(file);

              fr[index+i]->put_value(data);
          }
        }
        break;
      case 1:      // End of hex file
        return SUCCESS;
      case 4:      // Extended address
        {

          extended_address = read_be_word(file) << 16;
          printf ("Extended linear address %x %x\n",
                  address, extended_address);

        }
        break;
      default:
        printf ("Error! Unknown record type! %d\n", linetype);
        return ERR_BAD_FILE;
      }

      csby = getbyte (file);        /* get the checksum byte */
      /* this should make the checksum zero */
      /* due to side effect of getbyte */

      if (checksum)        {
        printf ("Checksum error in input file.\n");
        printf ("Got 0x%02x want 0x%02x at line %d\n",
          csby, (0 - checksum) & 0xff, lineCount);
        return ERR_BAD_FILE;
      }
      (void) getachar (file);        /* lose <return> */

      lineCount++;
    }
  return SUCCESS;
}

/* ... The End ... */
