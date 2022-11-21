/*
*****************************************************************************
* Copyright by ams AG                                                       *
* All rights are reserved.                                                  *
*                                                                           *
* IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
* THE SOFTWARE.                                                             *
*                                                                           *
* THIS SOFTWARE IS PROVIDED FOR USE ONLY IN CONJUNCTION WITH AMS PRODUCTS.  *
* USE OF THE SOFTWARE IN CONJUNCTION WITH NON-AMS-PRODUCTS IS EXPLICITLY    *
* EXCLUDED.                                                                 *
*                                                                           *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
*****************************************************************************
*/

/** @file */

#ifndef __TMF882X_CONFIG_H
#define __TMF882X_CONFIG_H

#define CONFIG_TMF882X_INTELHEX_SUPPORT() 1    ///< Enable Intel HEX format support for FWDL
#define CONFIG_TMF882X_HISTOGRAM_SUPPORT() 1    ///< Enable histogram readout support
#define CONFIG_TMF882X_8X8_SUPPORT() 1     ///< Enable 8x8 support (TMF8828 only)
#define CONFIG_TMF882X_OSC_TRIM_SUPPORT() 1     ///< Enable Oscillator trimming support
#define CONFIG_TMF882X_SPAD_CFG() 1     ///< Enable read/write SPAD configuration
#define CONFIG_TMF882X_LOG_SPAD_CFG() 1     ///< Enable SPAD configuration logging
#define CONFIG_TMF882X_LOG_CFG() 1     ///< Enable Configuration logging
#define CONFIG_TMF882X_DATA_DUMP_SUPPORT() 1     ///< Enable register dumps, error dumps, debug dumps, etc. (requires snprintf)

#endif
