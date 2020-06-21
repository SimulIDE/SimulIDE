#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libelf.h>
#include <gelf.h>

#include "sim_elf.h"
#include "sim_vcd_file.h"
#include "avr_eeprom.h"
#include "avr_ioport.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef struct flash_section_tag
{
    GElf_Shdr   shdr;
    Elf_Data *  data_ptr;
} flash_section_t;

static void elf_parse_mmcu_section(elf_firmware_t * firmware, uint8_t * src, uint32_t size)
{
    while (size) {
        uint8_t tag = *src++;
        uint8_t ts = *src++;
        int next = size > 2 + ts ? 2 + ts : size;
    //	printf("elf_parse_mmcu_section %d, %d / %d\n", tag, ts, size);
        switch (tag) {
            case AVR_MMCU_TAG_FREQUENCY:
                firmware->frequency =
                    src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
                break;
            case AVR_MMCU_TAG_NAME:
                strcpy(firmware->mmcu, (char*)src);
                break;
            case AVR_MMCU_TAG_VCC:
                firmware->vcc =
                    src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
                break;
            case AVR_MMCU_TAG_AVCC:
                firmware->avcc =
                    src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
                break;
            case AVR_MMCU_TAG_AREF:
                firmware->aref =
                    src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
                break;
            case AVR_MMCU_TAG_PORT_EXTERNAL_PULL: {
                for (int i = 0; i < 8; i++)
                    if (!firmware->external_state[i].port) {
                        firmware->external_state[i].port = src[2];
                        firmware->external_state[i].mask = src[1];
                        firmware->external_state[i].value = src[0];
                        AVR_LOG(NULL, LOG_TRACE, "AVR_MMCU_TAG_PORT_EXTERNAL_PULL[%d] %c:%02x:%02x\n",
                            i, firmware->external_state[i].port,
                            firmware->external_state[i].mask,
                            firmware->external_state[i].value);
                        break;
                    }
            }	break;
        case AVR_MMCU_TAG_VCD_PORTPIN:
        case AVR_MMCU_TAG_VCD_IRQ:
        case AVR_MMCU_TAG_VCD_TRACE: {
                uint8_t mask = src[0];
                uint16_t addr = src[1] | (src[2] << 8);
                char * name = (char*)src + 3;

//#if 0
                AVR_LOG(NULL, LOG_DEBUG,
                        "VCD_TRACE %d %04x:%02x - %s\n", tag,
                        addr, mask, name);
//#endif
                firmware->trace[firmware->tracecount].kind = tag;
                firmware->trace[firmware->tracecount].mask = mask;
                firmware->trace[firmware->tracecount].addr = addr;
                strncpy(firmware->trace[firmware->tracecount].name, name,
                    sizeof(firmware->trace[firmware->tracecount].name));
                firmware->tracecount++;
            }	break;
            case AVR_MMCU_TAG_VCD_FILENAME: {
                strcpy(firmware->tracename, (char*)src);
            }	break;
            case AVR_MMCU_TAG_VCD_PERIOD: {
                firmware->traceperiod =
                    src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
            }	break;
            case AVR_MMCU_TAG_SIMAVR_COMMAND: {
                firmware->command_register_addr = src[0] | (src[1] << 8);
            }	break;
            case AVR_MMCU_TAG_SIMAVR_CONSOLE: {
                firmware->console_register_addr = src[0] | (src[1] << 8);
            }	break;
        }
        size -= next;
        src += next - 2; // already incremented
    }
}

int elf_read_firmware_ext(const char * file, elf_firmware_t * firmware)
{
	Elf32_Ehdr elf_header;			/* ELF header */
	Elf *elf = NULL;                       /* Our Elf pointer for libelf */
	int fd; // File Descriptor

	if ((fd = open(file, O_RDONLY | O_BINARY)) == -1 ||
			(read(fd, &elf_header, sizeof(elf_header))) < sizeof(elf_header)) {
		AVR_LOG(NULL, LOG_ERROR, "could not read %s\n", file);
		perror(file);
		close(fd);
		return -1;
	}

	Elf_Data *data_data = NULL,
		*data_text = NULL,
		*data_ee = NULL;                /* Data Descriptor */

	memset(firmware, 0, sizeof(*firmware));
#if ELF_SYMBOLS
	firmware->symbolcount = 0;
	firmware->symbol = NULL;
#endif

	/* this is actually mandatory !! otherwise elf_begin() fails */
	if (elf_version(EV_CURRENT) == EV_NONE) {
			/* library out of date - recover from error */
	}
	// Iterate through section headers again this time well stop when we find symbols
	elf = elf_begin(fd, ELF_C_READ, NULL);
	//printf("Loading elf %s : %p\n", file, elf);
    if (!elf)
    {
            AVR_LOG(NULL, LOG_ERROR, "could not read %s\n", file);
            perror(file);
            close(fd);
            return -1;
    }

    size_t num_of_sections = 0, section_no = 0;
    elf_getshdrnum(elf, &num_of_sections);

    flash_section_t flash_sections[num_of_sections];

	Elf_Scn *scn = NULL;                   /* Section Descriptor */

	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		GElf_Shdr shdr;                 /* Section Header */
		gelf_getshdr(scn, &shdr);
		char * name = elf_strptr(elf, elf_header.e_shstrndx, shdr.sh_name);
    //	printf("Walking elf section '%s'\n", name);
        AVR_LOG(((avr_t*)0), LOG_TRACE, "%s: section %s\n", __func__, name);

        if (!strcmp(name, ".fuse"))
            continue;

		if (!strcmp(name, ".text"))
			data_text = elf_getdata(scn, NULL);
		else if (!strcmp(name, ".data"))
			data_data = elf_getdata(scn, NULL);
		else if (!strcmp(name, ".eeprom"))
			data_ee = elf_getdata(scn, NULL);
		else if (!strcmp(name, ".bss")) {
			Elf_Data *s = elf_getdata(scn, NULL);
			firmware->bsssize = s->d_size;
		} else if (!strcmp(name, ".mmcu")) {
			Elf_Data *s = elf_getdata(scn, NULL);
			elf_parse_mmcu_section(firmware, s->d_buf, s->d_size);
			//printf("%s: avr_mcu_t size %ld / read %ld\n", __FUNCTION__, sizeof(struct avr_mcu_t), s->d_size);
		//	avr->frequency = f_cpu;
            continue;
		}
        if ((shdr.sh_type == SHT_PROGBITS) && (shdr.sh_flags & SHF_ALLOC) != 0)
        {
            flash_sections[section_no].data_ptr = elf_getdata(scn, NULL);
            gelf_getshdr(scn, &flash_sections[section_no++].shdr);
        }

#if ELF_SYMBOLS
		// When we find a section header marked SHT_SYMTAB stop and get symbols
		if (shdr.sh_type == SHT_SYMTAB) {
			// edata points to our symbol table
			Elf_Data *edata = elf_getdata(scn, NULL);

			// how many symbols are there? this number comes from the size of
			// the section divided by the entry size
			int symbol_count = shdr.sh_size / shdr.sh_entsize;

			// loop through to grab all symbols
			for (int i = 0; i < symbol_count; i++) {
				GElf_Sym sym;			/* Symbol */
				// libelf grabs the symbol data using gelf_getsym()
				gelf_getsym(edata, i, &sym);

				// print out the value and size
				if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL ||
						ELF32_ST_TYPE(sym.st_info) == STT_FUNC ||
						ELF32_ST_TYPE(sym.st_info) == STT_OBJECT) {
					const char * name = elf_strptr(elf, shdr.sh_link, sym.st_name);

					// if its a bootloader, this symbol will be the entry point we need
					if (!strcmp(name, "__vectors"))
						firmware->flashbase = sym.st_value;
					avr_symbol_t * s = malloc(sizeof(avr_symbol_t) + strlen(name) + 1);
					strcpy((char*)s->symbol, name);
					s->addr = sym.st_value;
					if (!(firmware->symbolcount % 8))
						firmware->symbol = realloc(
							firmware->symbol,
							(firmware->symbolcount + 8) * sizeof(firmware->symbol[0]));

					// insert new element, keep the array sorted
					int insert = -1;
					for (int si = 0; si < firmware->symbolcount && insert == -1; si++)
						if (firmware->symbol[si]->addr >= s->addr)
							insert = si;
					if (insert == -1)
						insert = firmware->symbolcount;
					else
						memmove(firmware->symbol + insert + 1,
								firmware->symbol + insert,
								(firmware->symbolcount - insert) * sizeof(firmware->symbol[0]));
					firmware->symbol[insert] = s;
					firmware->symbolcount++;
				}
			}
		}
#endif
	}
    uint32_t offset = 0;
    firmware->flashsize =
            (data_text ? data_text->d_size : 0) +
            (data_data ? data_data->d_size : 0);
    /*
	firmware->flash = malloc(firmware->flashsize);

	// using unsigned int for output, since there is no AVR with 4GB
	if (data_text) {
	//	hdump("code", data_text->d_buf, data_text->d_size);
		memcpy(firmware->flash + offset, data_text->d_buf, data_text->d_size);
		AVR_LOG(NULL, LOG_TRACE, "Loaded %u .text at address 0x%x\n",
				(unsigned int)data_text->d_size, firmware->flashbase);
		offset += data_text->d_size;
	}
	if (data_data) {
	//	hdump("data", data_data->d_buf, data_data->d_size);
		memcpy(firmware->flash + offset, data_data->d_buf, data_data->d_size);
		AVR_LOG(NULL, LOG_TRACE, "Loaded %u .data\n", (unsigned int)data_data->d_size);
		offset += data_data->d_size;
		firmware->datasize = data_data->d_size;
	}
    */

    for (int ii = 0; ii < section_no; ii++)
    {
        if (((uint32_t)flash_sections[ii].shdr.sh_addr & 0x800000) != 0)
        {
            // 0x800000 bit mean this is a "Harvard architecture" data section: skip it as it is already calculated.
            continue;
        }
        uint32_t size = (uint32_t)flash_sections[ii].shdr.sh_addr + (uint32_t)flash_sections[ii].shdr.sh_size;
        if (size > firmware->flashsize)
            firmware->flashsize = size;
    }
    firmware->flash = malloc(firmware->flashsize);
    memset(firmware->flash, 0xFF, firmware->flashsize);
    // using unsigned int for output, since there is no AVR with 4GB
    if (data_text) {
    //	hdump("code", data_text->d_buf, data_text->d_size);
        memcpy(firmware->flash + offset, data_text->d_buf, data_text->d_size);
        AVR_LOG(NULL, LOG_TRACE, "Loaded %u .text at address 0x%x\n",
                (unsigned int)data_text->d_size, firmware->flashbase);
        offset += data_text->d_size;
    }
    if (data_data) {
    //	hdump("data", data_data->d_buf, data_data->d_size);
        memcpy(firmware->flash + offset, data_data->d_buf, data_data->d_size);
        AVR_LOG(NULL, LOG_TRACE, "Loaded %u .data\n", (unsigned int)data_data->d_size);
        offset += data_data->d_size;
        firmware->datasize = data_data->d_size;
    }
    for (int ii = 0; ii < section_no; ii++)
    {
        if (flash_sections[ii].data_ptr == data_text)
        {
            /*memcpy(firmware->flash + offset, data_text->d_buf, data_text->d_size);
            AVR_LOG(NULL, LOG_TRACE, "Loaded %u .text at address 0x%x\n",
                    (unsigned int)data_text->d_size, firmware->flashbase);
            offset += data_text->d_size;*/
            continue;
        }
        else if (flash_sections[ii].data_ptr == data_data)
        {
            /*memcpy(firmware->flash + offset, data_data->d_buf, data_data->d_size);
            AVR_LOG(NULL, LOG_TRACE, "Loaded %u .data\n", (unsigned int)data_data->d_size);
            offset += data_data->d_size;
            firmware->datasize = data_data->d_size;*/
            continue;
        }
        else {
            // This is a custom flash section. The shdr.sh_addr should contain its actual address.
            memcpy(firmware->flash + (uint32_t)flash_sections[ii].shdr.sh_addr,
                    flash_sections[ii].data_ptr->d_buf, (uint32_t)flash_sections[ii].shdr.sh_size);
        }
    }

	if (data_ee) {
	//	hdump("eeprom", data_ee->d_buf, data_ee->d_size);
		firmware->eeprom = malloc(data_ee->d_size);
		memcpy(firmware->eeprom, data_ee->d_buf, data_ee->d_size);
		AVR_LOG(NULL, LOG_TRACE, "Loaded %u .eeprom\n", (unsigned int)data_ee->d_size);
		firmware->eesize = data_ee->d_size;
	}
//	hdump("flash", avr->flash, offset);
	elf_end(elf);
	close(fd);
	return 0;
}

