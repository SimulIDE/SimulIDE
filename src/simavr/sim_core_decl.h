/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
 
#ifndef __SIM_CORE_DECL_H__
#define __SIM_CORE_DECL_H__

extern avr_kind_t mega164;
extern avr_kind_t tiny45;
extern avr_kind_t tiny85;
extern avr_kind_t mega324;
extern avr_kind_t mega1284;
extern avr_kind_t mega1280;
extern avr_kind_t tiny84;
extern avr_kind_t mega168;
extern avr_kind_t mega1281;
extern avr_kind_t tiny24;
extern avr_kind_t mega169p;
extern avr_kind_t mega128rfr2;
extern avr_kind_t tiny2313a;
extern avr_kind_t mega88;
extern avr_kind_t tiny4313;
extern avr_kind_t usb162;
extern avr_kind_t mega128rfa1;
extern avr_kind_t mega8;
extern avr_kind_t tiny25;
extern avr_kind_t mega2560;
extern avr_kind_t mega16;
extern avr_kind_t mega64;
extern avr_kind_t mega328;
extern avr_kind_t tiny2313;
extern avr_kind_t tiny13;
extern avr_kind_t mega16m1;
extern avr_kind_t mega324a;
extern avr_kind_t mega32u4;
extern avr_kind_t mega644;
extern avr_kind_t usb162;
extern avr_kind_t tiny44;
extern avr_kind_t mega128;
extern avr_kind_t mega48;
extern avr_kind_t mega32;
extern avr_kind_t * avr_kind[];

avr_kind_t * avr_kind[] = {
	&mega164,
	&tiny45,
	&tiny85,
	&mega324,
	&mega1284,
	&mega1280,
	&tiny84,
	&mega168,
	&mega1281,
	&tiny24,
	&mega169p,
	&mega128rfr2,
	&tiny2313a,
	&mega88,
	&tiny4313,
	&usb162,
	&mega128rfa1,
	&mega8,
	&tiny25,
	&mega2560,
	&mega16,
	&mega64,
	&mega328,
	&tiny2313,
	&tiny13,
	&mega16m1,
	&mega324a,
	&mega32u4,
	&mega644,
	&usb162,
	&tiny44,
	&mega128,
	&mega48,
	&mega32,
	NULL
};

#endif
