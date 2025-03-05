/*
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
	email: paul.alfille@gmail.com
	Released under the GPL
	See the header file: ow.h for full attribution
	1wire/iButton system from Dallas Semiconductor
*/

/* General Device File format:
    This device file corresponds to a specific 1wire/iButton chip type
	( or a closely related family of chips )

	The connection to the larger program is through the "device" data structure,
	  which must be declared in the acompanying header file.

	The device structure holds the
	  family code,
	  name,
	  device type (chip, interface or pseudo)
	  number of properties,
	  list of property structures, called "filetype".

	Each filetype structure holds the
	  name,
	  estimated length (in bytes),
	  aggregate structure pointer,
	  data format,
	  read function,
	  write funtion,
	  generic data pointer

	The aggregate structure, is present for properties that several members
	(e.g. pages of memory or entries in a temperature log. It holds:
	  number of elements
	  whether the members are lettered or numbered
	  whether the elements are stored together and split, or separately and joined
*/

/* DS2401 Simple ID device */
/* Also needed for all others */

/* ------- Prototypes ----------- */

	/* All from ow_xxxx.c file */

#include <config.h>
#include "owfs_config.h"
#include "ow_2401.h"

READ_FUNCTION(FS_humid_TSH202);
READ_FUNCTION(FS_temp_TSH202);
READ_FUNCTION(FS_r_functional_register);
READ_FUNCTION(FS_r_firmware_version);
READ_FUNCTION(FS_r_sub_family_code);

/* ------- Structures ----------- */

/* Copy-pasted from ow_1820.c.c */
#define SCRATCHPAD_LENGTH 9
#define _1W_WRITE_SCRATCHPAD      0x4E
#define _1W_READ_SCRATCHPAD       0xBE

/* read 9 bytes, includes CRC8 which is checked */
static GOOD_OR_BAD OW_r_scratchpad(BYTE * data, const struct parsedname *pn)
{
	/* data is 9 bytes long */
	BYTE be[] = { _1W_READ_SCRATCHPAD, };
	struct transaction_log tread[] = {
		TRXN_START,
		TRXN_WRITE1(be),
		TRXN_READ(data, SCRATCHPAD_LENGTH),
		TRXN_CRC8(data, SCRATCHPAD_LENGTH),
		TRXN_END,
	};
	return BUS_transaction(tread, pn);
}

/* write 2 bytes (byte4,5 of register) */
/* Write the scratchpad but not back to static */
static GOOD_OR_BAD OW_w_scratchpad(const BYTE * data, const struct parsedname *pn)
{
	/* data is 2 bytes long */
	BYTE d[3] = { _1W_WRITE_SCRATCHPAD, data[0], data[1], };

	struct transaction_log twrite[] = {
		TRXN_START,
		TRXN_WRITE(d, sizeof(d)),
		TRXN_END,
	};

	return BUS_transaction(twrite, pn);
}

static ZERO_OR_ERROR FS_r_scratchpad(struct one_wire_query *owq)
{
	BYTE s[SCRATCHPAD_LENGTH] ;

	if ( BAD(OW_r_scratchpad( s, PN(owq) ) ) ) {
		return -EINVAL ;
	}

	return OWQ_format_output_offset_and_size( (const ASCII *) s, SCRATCHPAD_LENGTH, owq);
}

// TODO: Should I accept the whole scratchpad here?
// Or as now, just the two bytes we're going to write.
static ZERO_OR_ERROR FS_w_scratchpad(struct one_wire_query *owq)
{
	FS_del_sibling( "TSH202/scratchpad", owq ) ;
	return GB_to_Z_OR_E(OW_w_scratchpad( (BYTE *)OWQ_buffer(owq), PN(owq)));
}

#define _TSH202_READ_FUNCTIONAL_REGISTER 0x93
#define _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH 4
#define _TSH202_FIRMWARE_VERSION_LENGTH 8
#define _TSH202_SUB_FAMILY_CODE_LENGTH 5

// TODO: We should add some detection magic here, so if a DS2401 is detected to actually be a TSH202, add the relevant bits.
static struct filetype DS2401[] = {
	F_STANDARD,
	{"TSH202", PROPERTY_LENGTH_SUBDIR, NON_AGGREGATE, ft_subdir, fc_subdir, NO_READ_FUNCTION, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/scratchpad", SCRATCHPAD_LENGTH, NON_AGGREGATE, ft_binary, fc_volatile, FS_r_scratchpad, FS_w_scratchpad, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/humidity", PROPERTY_LENGTH_FLOAT, NON_AGGREGATE, ft_float, fc_volatile, FS_humid_TSH202, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/temperature", PROPERTY_LENGTH_TEMP, NON_AGGREGATE, ft_temperature, fc_simultaneous_temperature, FS_temp_TSH202, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/functional_register", _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH, NON_AGGREGATE, ft_binary, fc_volatile, FS_r_functional_register, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/firmware_version", _TSH202_FIRMWARE_VERSION_LENGTH, NON_AGGREGATE, ft_ascii, fc_static, FS_r_firmware_version, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
	{"TSH202/sub_family_code", _TSH202_SUB_FAMILY_CODE_LENGTH, NON_AGGREGATE, ft_ascii, fc_static, FS_r_sub_family_code, NO_WRITE_FUNCTION, VISIBLE, NO_FILETYPE_DATA, },
};

/* read 4 bytes, includes CRC8 which is checked */
static GOOD_OR_BAD OW_r_functional_register(BYTE * data, const struct parsedname *pn)
{
	/* data is 4 bytes long */
	BYTE be[] = { _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH, };
	struct transaction_log tread[] = {
		TRXN_START,
		TRXN_WRITE1(be),
		TRXN_READ(data, _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH),
		// TODO: My device just has FFFFFFFF in this register... and that maks no good checksum.
		//TRXN_CRC8(data, _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH),
		TRXN_END,
	};
	return BUS_transaction(tread, pn);
}

static ZERO_OR_ERROR FS_r_functional_register(struct one_wire_query *owq)
{
	BYTE s[_TSH202_READ_FUNCTIONAL_REGISTER_LENGTH] = {};

	if ( BAD(OW_r_functional_register( s, PN(owq) ) ) ) {
		return -EINVAL ;
	}

	return OWQ_format_output_offset_and_size( (const ASCII *) s, _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH, owq);
}

static ZERO_OR_ERROR FS_temp_TSH202(struct one_wire_query *owq)
{
	size_t scr_leng = SCRATCHPAD_LENGTH ;
	BYTE data[scr_leng];

	RETURN_ERROR_IF_BAD(FS_r_sibling_binary( data, &scr_leng, "TSH202/scratchpad", owq )) ;

	OWQ_F(owq) = (_FLOAT) ((int16_t) ((data[1] << 8) | data[0] )) * .0625 ;

	return 0;
}

static ZERO_OR_ERROR FS_humid_TSH202(struct one_wire_query *owq)
{
	size_t scr_leng = SCRATCHPAD_LENGTH ;
	BYTE data[scr_leng];

	RETURN_ERROR_IF_BAD(FS_r_sibling_binary( data, &scr_leng, "TSH202/scratchpad", owq )) ;

	OWQ_F(owq) = (_FLOAT) ((int16_t) ((data[3] << 8) | data[2] )) * .0625 ;

	return 0;
}

static ZERO_OR_ERROR FS_r_firmware_version(struct one_wire_query *owq)
{
	size_t fr_leng = _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH;
	BYTE data[fr_leng];

	RETURN_ERROR_IF_BAD(FS_r_sibling_binary( data, &fr_leng, "TSH202/functional_register", owq )) ;

	char res[_TSH202_FIRMWARE_VERSION_LENGTH] = {};
	snprintf(res, sizeof(res), "%3d.%3d", data[0], data[1]);

	return OWQ_format_output_offset_and_size_z(res, owq);
}

static ZERO_OR_ERROR FS_r_sub_family_code(struct one_wire_query *owq)
{
	size_t fr_leng = _TSH202_READ_FUNCTIONAL_REGISTER_LENGTH;
	BYTE data[fr_leng];

	RETURN_ERROR_IF_BAD(FS_r_sibling_binary( data, &fr_leng, "TSH202/functional_register", owq )) ;

	char res[_TSH202_SUB_FAMILY_CODE_LENGTH] = {};
	snprintf(res, sizeof(res), "0x%X", data[3]);

	return OWQ_format_output_offset_and_size_z(res, owq);
}

DeviceEntry(01, DS2401, NO_GENERIC_READ, NO_GENERIC_WRITE);

static struct filetype DS1420[] = {
	F_STANDARD,
};

DeviceEntry(81, DS1420, NO_GENERIC_READ, NO_GENERIC_WRITE);

/* ------- Functions ------------ */
