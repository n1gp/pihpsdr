/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*/

/**
* @file ozyio.c
* @brief USB I/O with Ozy
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/


/*
* modified by Bob Wisdom VK4YA May 2015 to create ozymetis
* modified further Laurence Barker G8NJJ to add USB functionality to pihpsdr
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>    // tolower
#include <errno.h>
#include <string.h>
#include <sys/stat.h> // for stat
#include <unistd.h>   // for readlink, sleep, getcwd

#include <libusb-1.0/libusb.h>

#include "ozyio.h"
#include "message.h"

#define OZY_PID (0x0007)
#define OZY_VID (0xfffe)

#define VRQ_SDR1K_CTL 0x0d
#define SDR1KCTRL_READ_VERSION  0x7
#define VRT_VENDOR_IN 0xC0

#define VENDOR_REQ_TYPE_IN 0xc0
#define VENDOR_REQ_TYPE_OUT 0x40

#define VRQ_I2C_WRITE (0x08)
#define VRT_VENDOR_OUT (0x40)


#define VENDOR_REQ_SET_LED 0x01
#define VENDOR_REQ_FPGA_LOAD 0x02

#define FL_BEGIN 0
#define FL_XFER 1
#define FL_END 2

#define OZY_BUFFER_SIZE 512

#define OZY_IO_TIMEOUT 10
//#define OZY_IO_TIMEOUT 500
//#define OZY_IO_TIMEOUT 2000
#define MAX_EPO_PACKET_SIZE 64

#define USB_TIMEOUT -7

static int init = 0;
extern unsigned char penny_fw, mercury_fw;
extern unsigned short penny_fp, penny_rp, penny_alc;
extern int adc_overflow;
void writepenny(unsigned char mode);
static libusb_device_handle* ozy_handle;

static char ozy_firmware[64] = {0};
static char ozy_fpga[64] = {0};
static unsigned char ozy_firmware_version[9];

// variables accessed via ozy_i2c_readvars:
unsigned char penny_fw = 0, mercury_fw = 0;

// variables accessed via ozy_i2c_readpwr
unsigned short penny_fp = 0, penny_rp = 0, penny_alc = 0;
int adc_overflow;





int ozy_open() {
  int rc;

  if (init == 0) {
    rc = libusb_init(NULL);

    if (rc < 0) {
      t_print("libusb_init failed: %d\n", rc);
      return rc;
    }

    init = 1;
  }

  ozy_handle = libusb_open_device_with_vid_pid(NULL, OZY_VID, OZY_PID);

  if (ozy_handle == NULL) {
    t_print("libusbio: cannot find ozy device\n");
    return -1;
  }

  rc = libusb_detach_kernel_driver(ozy_handle, 0);

  if (rc < 0) {
    //        t_print("libusb_detach_kernel_driver failed: %d\n",rc);
  }

  rc = libusb_claim_interface(ozy_handle, 0);

  if (rc < 0) {
    t_print("libusb_claim_interface failed: %d\n", rc);
    return rc;
  }

  return 0;
}

int ozy_close() {
  int rc;
  rc = libusb_attach_kernel_driver(ozy_handle, 0);

  if (rc < 0) {
    //        t_print("libusb_attach_kernel_driver failed: %d\n",rc);
  }

  libusb_close(ozy_handle);
  return 0;
}

int ozy_get_firmware_string(unsigned char* buffer, int buffer_size) {
  int rc;
  rc = libusb_control_transfer(ozy_handle, VRT_VENDOR_IN, VRQ_SDR1K_CTL, SDR1KCTRL_READ_VERSION, 0, buffer, buffer_size,
                               OZY_IO_TIMEOUT);

  if (rc < 0) {
    t_print("ozy__get_firmware_string failed: %d\n", rc);
    return rc;
  }

  buffer[rc] = '\0';
  return 0;
}

int ozy_write(int ep, unsigned char* buffer, int buffer_size) {
  int rc;
  int bytes;
  bytes = 0;
  rc = libusb_bulk_transfer(ozy_handle, (unsigned char)ep, buffer, buffer_size, &bytes, OZY_IO_TIMEOUT);

  if (rc == USB_TIMEOUT) {
    t_print("%s: timeout bytes=%d ep=%d\n", __FUNCTION__, bytes, ep);
    libusb_clear_halt(ozy_handle, (unsigned char)ep);
  }

  // this returns OK in all cases (?)
  return buffer_size;
}

int ozy_read(int ep, unsigned char* buffer, int buffer_size) {
  int rc;
  int bytes;
  rc = libusb_bulk_transfer(ozy_handle, (unsigned char)ep, buffer, buffer_size, &bytes, OZY_IO_TIMEOUT);

  if (rc == 0) {
    rc = bytes;
  }

  return rc;
}

int ozy_write_ram(int fx2_start_addr, unsigned char *bufp, int count) {
  int pkt_size = MAX_EPO_PACKET_SIZE;
  int len = count;
  int bytes_written = 0;
  int addr;

  for ( addr = fx2_start_addr; addr < fx2_start_addr + len; addr += pkt_size, bufp += pkt_size ) {
    int nsize = len + fx2_start_addr - addr;

    if ( nsize > pkt_size ) { nsize = pkt_size; }

    int bytes_written_this_write = libusb_control_transfer(ozy_handle, 0x40, 0xa0, addr, 0, bufp, nsize, OZY_IO_TIMEOUT);

    if ( bytes_written_this_write >= 0  ) {
      bytes_written += bytes_written_this_write;
    } else {
      return bytes_written_this_write;
    }
  }

  return bytes_written;
}

int ozy_reset_cpu(int reset) {
  unsigned char write_buf;

  if ( reset ) { write_buf = 1; }
  else { write_buf = 0; }

  if ( ozy_write_ram(0xe600, &write_buf, 1) != 1 ) { return 0; }
  else { return 1; }
}

static unsigned int hexitToUInt(char c) {
  c = tolower(c);

  if ( c >= '0' && c <= '9' ) {
    return c - '0';
  } else if ( c >= 'a' && c <= 'f' ) {
    return 10 + (c - 'a');
  }

  return 0;
}

static int ishexit(unsigned char c) {
  c = tolower(c);

  if ( c >= '0' && c <= '9' ) { return 1; }

  if ( c >= 'a' && c <= 'f' ) { return 1; }

  return 0;
}

static int hexitsToUInt(char *p, int count) {
  unsigned int result = 0;

  for (int  i = 0; i < count; i++ ) {
    char c = *p;
    ++p;

    if ( !ishexit(c) ) {
      return -1;
    }

    unsigned int this_hex = hexitToUInt(c);
    result *= 16;
    result += this_hex;
  }

  return result;
}

int ozy_load_firmware(char *fnamep) {
  FILE *ifile;
  int linecount = 0;
  int length;
  int addr;
  int type;
  char readbuf[1030];
  unsigned char wbuf[256];
  unsigned char my_cksum;
  unsigned char cksum;
  int this_val;
  int i;
  t_print("loading ozy firmware: %s\n", fnamep);
  ifile = fopen(fnamep, "r");

  if ( ifile == NULL ) {
    t_print( "Could not open: \'%s\'\n", fnamep);
    return 0;
  }

  while (  fgets(readbuf, sizeof(readbuf), ifile) != NULL ) {
    ++linecount;

    if ( readbuf[0] != ':' ) {
      t_print( "ozy_upload_firmware: bad record\n");
      return 0;
    }

    length = hexitsToUInt(readbuf + 1, 2);
    addr = hexitsToUInt(readbuf + 3, 4);
    type = hexitsToUInt(readbuf + 7, 2);

    if ( length < 0 || addr < 0 || type < 0 ) {
      t_print( "ozy_upload_firmware: bad length, addr or type\n");
      return 0;
    }

    switch ( type ) {
    case 0: /* record */
      my_cksum = (unsigned char)(length + (addr & 0xff) + ((addr >> 8) + type));

      for ( i = 0; i < length; i++ ) {
        this_val = hexitsToUInt(readbuf + 9 + (i * 2), 2);
#if 0
        t_print("i: %d val: 0x%02x\n", i, this_val);
#endif

        if ( this_val < 0 ) {
          t_print( "ozy_upload_firmware: bad record data\n");
          return 0;
        }

        wbuf[i] = (unsigned char)this_val;
        my_cksum += wbuf[i];
      }

      this_val = hexitsToUInt(readbuf + 9 + (length * 2), 2);

      if ( this_val < 0 ) {
        t_print( "ozy_upload_firmware: bad checksum data\n");
        return 0;
      }

      cksum = (unsigned char)this_val;
#if 0
      t_print("\n%s", readbuf);
      t_print("len: %d (0x%02x) addr: 0x%04x mychk: 0x%02x chk: 0x%02x",
              length, length, addr, my_cksum, cksum);
#endif

      if ( ((cksum + my_cksum) & 0xff) != 0 ) {
        t_print( "ozy_upload_firmware: bad checksum\n");
        return 0;
      }

      if ( ozy_write_ram(addr, wbuf, length) < 1 ) {
        t_print( "ozy_upload_firmware: bad write\n");
        return 0;
      }

      break;

    case 1: /* EOF */
      break;

    default: /* invalid */
      t_print( "ozy_upload_firmware: invalid type\n");
      return 0;
    }
  }

  //        t_print( "ozy_upload_firmware: Processed %d lines.\n", linecount);
  return linecount;
}

int ozy_set_led(int which, int on) {
  int rc;
  int val;

  if ( on ) {
    val = 1;
  } else {
    val = 0;
  }

  rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_SET_LED,
                               val, which, NULL, 0, OZY_IO_TIMEOUT);

  if ( rc < 0 ) {
    return 0;
  }

  return 1;
}

int ozy_load_fpga(char *rbf_fnamep) {
  FILE *rbffile;
  unsigned char buf[MAX_EPO_PACKET_SIZE];
  int bytes_read;
  int total_bytes_xferd = 0;
  int rc;
  t_print("loading ozy fpga: %s\n", rbf_fnamep);
  rbffile = fopen(rbf_fnamep, "rb");

  if ( rbffile == NULL ) {
    t_print( "Failed to open: \'%s\'\n", rbf_fnamep);
    return 0;
  }

  rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                               0, FL_BEGIN, NULL, 0, OZY_IO_TIMEOUT);

  if ( rc < 0 ) {
    t_print( "ozy_load_fpga: failed @ FL_BEGIN rc=%d\n", rc);
    fclose(rbffile);
    return 0;
  }

  /*
       *  read the rbf and send it over the wire, 64 bytes at a time
       */
  while ( (bytes_read = fread(buf, 1, sizeof(buf), rbffile)) > 0 ) {
    rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                                 0, FL_XFER, buf, bytes_read, OZY_IO_TIMEOUT);
    total_bytes_xferd += bytes_read;

    if ( rc < 0 ) {
      t_print( "ozy_load_fpga: failed @ FL_XFER\n");
      fclose(rbffile);
      return 0;
    }
  }

  t_print("%d bytes transferred.\n", total_bytes_xferd);
  fclose(rbffile);
  rc = libusb_control_transfer(ozy_handle, VENDOR_REQ_TYPE_OUT, VENDOR_REQ_FPGA_LOAD,
                               0, FL_END, NULL, 0, OZY_IO_TIMEOUT);

  if ( rc < 0 ) {
    t_print( "ozy_load_fpga: failed @ FL_END\n");
    return 0;
  }

  return 1;
}

int ozy_i2c_write(unsigned char* buffer, int buffer_size, unsigned char cmd) {
  int rc;
  rc = libusb_control_transfer(ozy_handle, VRT_VENDOR_OUT, VRQ_I2C_WRITE, cmd, 0, buffer, buffer_size, OZY_IO_TIMEOUT);

  if (rc < 0) {
    t_print("ozy_i2c_write failed: %d\n", rc);
    return rc;
  }

  return rc;
}

int ozy_i2c_read(unsigned char* buffer, int buffer_size, unsigned char cmd) {
  int rc;
  rc = libusb_control_transfer(ozy_handle, VRT_VENDOR_IN, VRQ_I2C_READ, cmd, 0, buffer, buffer_size, OZY_IO_TIMEOUT);

  if (rc < 0) {
    t_print("ozy_i2c_read failed: %d\n", rc);
    return rc;
  }

  return rc;
}


void ozy_i2c_readpwr(int addr) {
  int rc = 0;
  unsigned char buffer[8];

  switch (addr) {
  case I2C_PENNY_ALC:
    rc = ozy_i2c_read(buffer, 2, I2C_PENNY_ALC);

    if (rc < 0) {
      t_perror("ozy_i2c_readpwr alc: failed");
    }

    penny_alc = (buffer[0] << 8) + buffer[1];
    break;

  case I2C_PENNY_FWD:
    rc = ozy_i2c_read(buffer, 2, I2C_PENNY_FWD);

    if (rc < 0) {
      t_perror("ozy_i2c_readpwr fwd: failed");
    }

    penny_fp = (buffer[0] << 8) + buffer[1];
    break;

  case I2C_PENNY_REV:
    rc = ozy_i2c_read(buffer, 2, I2C_PENNY_REV);

    if (rc < 0) {
      t_perror("ozy_i2c_readpwr rev: failed");
    }

    penny_rp = (buffer[0] << 8) + buffer[1];
    break;

  case I2C_MERC1_ADC_OFS:
    // adc overload
    rc = ozy_i2c_read(buffer, 2, I2C_MERC1_ADC_OFS); // adc1 overflow status

    if (rc < 0) {
      t_perror("ozy_i2c_readpwr adc: failed");
    }

    if (buffer[0] == 0) {       // its overloaded
      adc_overflow = 2000;    // counts down to give hold time to client
    }

    break;

  default:
    break;
  }
}

void ozy_i2c_readvars() {
  int rc = 0;
  unsigned char buffer[8];
  t_print("ozy_i2c_init: starting\n");
  rc = ozy_i2c_read(buffer, 2, I2C_MERC1_FW);

  if (rc < 0) {
    t_perror("ozy_i2c_readvars MercFW: failed");
    //
    // quickly return: if this fails, probably the I2C jumpers are not set
    // correctly and it is not worth to continue
    //
    return;
  }

  mercury_fw = buffer[1];
  t_print("mercury firmware=%d\n", (int)buffer[1]);
  rc = ozy_i2c_read(buffer, 2, I2C_PENNY_FW);

  if (rc < 0) {
    t_perror("ozy_i2c_readvars PennyFW: failed");
    //
    // If this fails, writing the TLV320 data to Penny
    // need not be attempted
    //
    return;
  }

  penny_fw = buffer[1];
  t_print("penny firmware=%d\n", (int)buffer[1]);
  writepenny((unsigned char)1);
}

void writepenny(unsigned char mode) {
  unsigned char Penny_TLV320[2];
  unsigned char Penny_TLV320_data[] = { 0x1e, 0x00, 0x12, 0x01, 0x08, 0x15, 0x0c, 0x00, 0x0e, 0x02, 0x10, 0x00, 0x0a, 0x00, 0x00, 0x00 }; // 16 byte
  int x;
  // This is used to set the MicGain and Line in when Ozy/Magister is used
  // The I2C settings are as follows:
  //
  //    1E 00 - Reset chip
  //    12 01 - set digital interface active
  //    08 XX - D/A on. See below for mic settings
  //    08 14 - ditto but no mic boost
  //    0C 00 - All chip power on
  //    0E 02 - Slave, 16 bit, I2S
  //    10 00 - 48k, Normal mode
  //    0A 00 - turn D/A mute off
  //    00 00 - set Line in gain to 0
  //
  //    Microphone settings (6th byte):
  //
  //    XX=0x15: Use Mic in, apply 20dB Mic boost
  //    XX=0x14: Use Mic in, no Mic boost
  //    XX=0x10: Use Line in
  t_print("write Penny\n");

  //
  // update mic gain on Penny or PennyLane TLV320
  //
  if (mode == 0x01) {
    Penny_TLV320_data[5]  = 0x15;  // mic in, mic boost 20db
  } else if (mode & 2) {
    Penny_TLV320_data[5]  = 0x10;  // line in
  } else {
    Penny_TLV320_data[5]  = 0x14;  // mic in, no mic boost
  }

  //      // set the I2C interface speed to 400kHZ
  //      if (!(OZY.Set_I2C_Speed(hdev, 1)))
  //      {
  //          t_print("Unable to set I2C speed to 400kHz", "System Error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
  //          return;

  // send the configuration data to the TLV320 on Penelope or PennyLane
  for (x = 0; x < 16; x += 2) {
    // copy two bytes to buffer and send via I2C
    Penny_TLV320[0] = Penny_TLV320_data[x];
    Penny_TLV320[1] = Penny_TLV320_data[x + 1];

    if (ozy_i2c_write(Penny_TLV320, 2, I2C_PENNY_TLV320) < 0) {
      t_print("Unable to configure TLV320 on Penelope via I2C\n");
      // break out of the configuration loop
      break;
    }
  }

  t_print("write Penny done..\n");
}



static int file_exists (const char * fileName) {
  struct stat buf;
  int i = stat ( fileName, &buf );
  return ( i == 0 ) ? 1 : 0 ;
}

#if defined(__linux__) || defined(__APPLE__)
int filePath (char *sOut, const char *sIn) {
  int rc = 0;

  if ((rc = file_exists (sIn))) {
    strcpy (sOut, sIn);
    rc = 1;
  } else {
    char xPath [PATH_MAX] = {0};
    rc = readlink ("/proc/self/exe", xPath, sizeof(xPath));

    // try to detect the directory from which the executable has been loaded
    if (rc >= 0) {
      char s[PATH_MAX];
      char *p;

      if ( (p = strrchr (xPath, '/')) ) { *(p + 1) = '\0'; }

      t_print( "%d, Path of executable: [%s]\n", rc, xPath);
      strcpy (s, xPath);
      strcat (s, sIn);

      if ((rc = file_exists (s))) {
        // found in the same dir of executable
        t_print( "File: [%s]\n", s);
        strcpy(sOut, s);
      } else {
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
          t_print( "Current working dir: %s\n", cwd);
          strcpy (s, cwd);
          strcat (s, "/");
          strcat (s, sIn);

          if ((rc = file_exists (s))) {
            t_print( "File: [%s]\n", s);
            strcpy(sOut, s);
          }
        }
      }
    } else {
      t_print( "%d: %s\n", errno, strerror(errno));
    }
  }

  return rc;
}
#endif



//
// initialise a USB ozy device.
// renamed as "initialise" and combined with the "ozyinit" code
//
int ozy_initialise() {
  int rc;

  if (strlen(ozy_firmware) == 0) { filePath (ozy_firmware, "ozyfw-sdr1k.hex"); }

  if (strlen(ozy_fpga) == 0) { filePath (ozy_fpga, "Ozy_Janus.rbf"); }

  // open ozy
  rc = ozy_open();

  if (rc != 0) {
    t_print("Cannot locate Ozy\n");
  }

  // load Ozy FW
  ozy_reset_cpu(1);
  ozy_load_firmware(ozy_firmware);
  ozy_reset_cpu(0);
  ozy_close();
  sleep(4);
  ozy_open();
  ozy_set_led(1, 1);
  ozy_load_fpga(ozy_fpga);
  ozy_set_led(1, 0);
  ozy_close();
  ozy_open();
  ozy_get_firmware_string(ozy_firmware_version, 8);
  t_print("Ozy FX2 version: %s\n", ozy_firmware_version);
  //
  // NOTE (thanks Rick): For I2C to work you need to place jumpers on SCL and SDA on
  // the OZY/Magister board. This enables firmware detection on adjacent cards
  // and microphone settings on Penny.
  //
  ozy_i2c_readvars();
  ozy_close();
  sleep(1);
  ozy_open();
  return 0;
}



//
// modified from "ozy_open" code: just finds out if there is a USB
// ozy on the bus. Closes the connection after discovering.
// returns 1 if a device found on USB
//
//
int ozy_discover() {
  int success = 0;            // function return code

  if (init == 0) {
    int rc = libusb_init(NULL);

    if (rc < 0) {
      t_print("libusb_init failed: %d\n", rc);
      return success;
    }

    init = 1;
  }

  //
  // do a trial open with thr PID and VID of ozy
  //
  ozy_handle = libusb_open_device_with_vid_pid(NULL, OZY_VID, OZY_PID);

  if (ozy_handle == NULL) {
    t_print("libusbio: cannot find ozy device\n");
    return success;
  } else {
    success = 1;
    t_print("libusbio: ozy device found on USB port\n");
  }

  //
  // if we get this far, we have an ozy on the bus so discover successful.
  // we don't know that it will be selected for use, so close it again
  // for now; re-open if the user selects Ozy
  //
  libusb_close(ozy_handle);
  return success;
}
