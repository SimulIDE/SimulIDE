/*
   Copyright (C) 2017        Roy R Rankin

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

#include "config.h"
#include "14bit-processors.h"
#include "14bit-registers.h"
#include "a2d_v2.h"


#include "lcd_module.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


LCD_MODULE::LCD_MODULE(Processor *pCpu, bool p16f917)
   : cpu(pCpu), IntSrc(0)
{
    char lcdsex[] = "lcdsex";
    char lcddataX[10];
    int i;

    Vlcd1 = Vlcd2 = Vlcd3 = 0;
    Vlcd1_on = Vlcd2_on = Vlcd3_on = false;
    bias_now = 0;
    future_cycle = 0;
    is_sleeping = false;

    lcdcon = new LCDCON(pCpu, "lcdcon", "LCD control register", this);
    lcdps = new LCDPS(pCpu, "lcdps", "LCD prescaler select register", this, 0xcf);
    for (i = 0; i < 3 ; i++)
    {
        lcdsex[5] = '0' + i;
        if ( i < 2 || p16f917)
            lcdSEn[i] = new LCDSEn(pCpu, (const char *)lcdsex, "LCD Segment register", this, i);
        else
            lcdSEn[i] = 0;
    }
    printf("\n");
    for (i=0; i < 12; i++)
    {
        snprintf(lcddataX, sizeof(lcddataX), "lcddata%d", i);
         if (((i+1)%3 != 0) || p16f917)
         {
            lcddatax[i] = new LCDDATAx(pCpu, (const char *)lcddataX, "LCD Data register", this, i);
        }
        else
        {
            lcddatax[i] = 0;
        }
    }
    for(i=0; i < 24; i++)
        LCDsegn[i] = 0;
    for(i=0; i<4; i++)
        LCDcom[i] = 0;
}
// set LCD bias pins
void LCD_MODULE::set_Vlcd(PinModule *_Vlcd1, PinModule *_Vlcd2, PinModule *_Vlcd3)
{
    Vlcd1 = _Vlcd1;
    Vlcd2 = _Vlcd2;
    Vlcd3 = _Vlcd3;
}
// set LCD common pins
void LCD_MODULE::set_LCDcom(PinModule *c0, PinModule *c1, PinModule *c2, PinModule *c3)
{
        LCDcom[0] = c0;
        LCDcom[1] = c1;
        LCDcom[2] = c2;
        LCDcom[3] = c3;
}

// set 4 LCD segment pins (at a time)
void LCD_MODULE::set_LCDsegn(uint i, PinModule *c0, PinModule *c1, PinModule *c2, PinModule *c3)
{
        assert(i <= 20);
        LCDsegn[i+0] = c0;
        LCDsegn[i+1] = c1;
        LCDsegn[i+2] = c2;
        LCDsegn[i+3] = c3;
}

void LCD_MODULE::clear_bias()
{
    Dprintf(("LCD_MODULE::clear_bias()\n"));
    bias_now = 0;
    if (Vlcd1_on)
    {
        Vlcd1->AnalogReq(lcdps, false, Vlcd1->getPin().name().c_str());
               Vlcd1_on = false;
    }
    if (Vlcd2_on)
    {
        Vlcd2->AnalogReq(lcdps, false, Vlcd2->getPin().name().c_str());
               Vlcd2_on = false;
    }
    if (Vlcd3_on)
    {
        Vlcd3->AnalogReq(lcdps, false, Vlcd3->getPin().name().c_str());
               Vlcd3_on = false;
    }
}

void LCD_MODULE::set_bias(uint lmux)
{
    bool biasmode = (lcdps->value.get() & LCDPS::BIASMD);
    unsigned char bias = 0;

    Dprintf(("LCD_MODULE::set_bias Vlcd1=%p\n", Vlcd1));

    switch(lmux)
    {
    case 0:
        bias = 1;
        break;

    case 1:
        bias = biasmode ? 2 : 3;
        break;

    case 2:
        bias = biasmode ? 2 : 3;
        break;

    case 3:
        bias = 3;
        break;
    }
    if (bias == bias_now)
        return;

    switch(bias)
    {
    case 1:        //Static bias
        if (lcdcon->value.get() & LCDCON::VLCDEN)
        {
                if (Vlcd1_on)
            {
                    Vlcd1->AnalogReq(lcdps, false, Vlcd1->getPin().name().c_str());
                   Vlcd1_on = false;
            }
                if (Vlcd2_on)
            {
                    Vlcd2->AnalogReq(lcdps, false, Vlcd2->getPin().name().c_str());
                Vlcd2_on = false;
                }
                if (!Vlcd3_on)
            {
                    Vlcd3->AnalogReq(lcdps, true, "vlcd3");
                Vlcd3_on = true;
                }

        }
        break;

    case 2:        // 1/2 bias
    case 3:        // 1>/3 bias
        if (!Vlcd1_on)
        {
           Vlcd1->AnalogReq(lcdps, true, "vlcd1");
                  Vlcd1_on = true;
        }
        if (!Vlcd2_on)
        {
            Vlcd2->AnalogReq(lcdps, true, "vlcd2");
            Vlcd2_on = true;
        }
        if (!Vlcd3_on)
        {
            Vlcd3->AnalogReq(lcdps, true, "vlcd3");
            Vlcd3_on = true;
        }
        break;
    }


    bias_now = bias;
}


void LCD_MODULE::lcd_on_off(bool lcdOn)
{
    uint i;
    if (lcdOn)
    {
        for (i=0; i < 3; i++)
        {
            if (lcdSEn[i])
                lcd_set_segPins(i, lcdSEn[i]->value.get(), lcdSEn[i]->value.get()^0);
        }
        lcd_set_com(lcdOn, lcdcon->value.get() & (LCDCON::LMUX0| LCDCON::LMUX1));
        start_clock();
    }
    else
    {
    }
}
void LCD_MODULE::lcd_set_com(bool lcdOn, uint lmux)
{
    uint i;

    Dprintf(("LCD_MODULE::lcd_set_com on %d lmux %u\n", lcdOn, lmux));
    if (lcdOn)
    {
        for(i=0; i < 4; i++)
        {
            mux_now = lmux;
            if (i <= lmux)
            {
                char name[5];
                snprintf(name, sizeof(name), "COM%u", i);

                if (LCDcom[i]->getPin().get_direction())
                    LCDcomDirection |= (1<<i);
                else
                    LCDcomDirection &= ~(1<<i);
                LCDcom[i]->getPin().update_direction(1,true);
            }
            else
            {
                LCDcom[i]->getPin().update_direction(LCDcomDirection & (1<<i),true);
            }
        }
    }
    else
    {
        for(i= 0; i < 4; i++)
        {
            LCDcom[i]->getPin().update_direction(LCDcomDirection & (1<<i),true);
        }
    }
}

void LCD_MODULE::sleep()
{
    uint con_reg = lcdcon->value.get();
    Dprintf(("LCD_MODULE::sleep()\n"));
    if (!(lcdps->value.get() & LCDPS::LCDA))
        return;

    // Stop during sleep
    if ((con_reg & LCDCON::SLPEN) || !(con_reg & (LCDCON::CS0 | LCDCON::CS1)))
    {
        Dprintf(("LCD_MODULE::sleep() stop during sleep fc=%" PRINTF_GINT64_MODIFIER "d now=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle, get_cycles().get()));
        if (future_cycle >= get_cycles().get())
        {
            get_cycles().clear_break(future_cycle);
            future_cycle = 0;
            phase = 0;
        }
        is_sleeping = true;
        // Set all LCD outputs to zero
        for(int l=0; l <= mux_now; l++) // scan across com related output
        {
          LCDcom[l]->getPin().putState(0.);
        }
        for(int k = 0; (k < 3) && lcdSEn[k]; k++)
        {
            uint enable = lcdSEn[k]->value.get();
            if (enable)
            {
                for(int i=0; i< 8; i++)
                {
                    if (enable & (1<<i)) LCDsegn[i]->getPin().putState(0.);
                }
            }
        }

    }
}
void LCD_MODULE::wake()
{
    uint con_reg = lcdcon->value.get();
    if (!(lcdps->value.get() & LCDPS::LCDA) || !is_sleeping)
        return;

    is_sleeping = false;

    Dprintf(("LCD_MODULE::wake() fc=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle));
    // Stop during sleep
    if ((con_reg & LCDCON::SLPEN) || !(con_reg & (LCDCON::CS0 | LCDCON::CS1)))
    {
        Dprintf(("LCD_MODULE::wake() restart after  sleep\n"));
        start_clock();
    }
}

void LCD_MODULE::lcd_set_segPins(uint regno, uint new_value, uint diff)
{
    unsigned char *pt = &LCDsegDirection[regno];

    for (int i = 0; i < 8; i++)
    {
        uint mask = 1<<i;
        PinModule *port = LCDsegn[regno*8+i];
        
        if (diff & mask)
        {
            if (new_value & mask)
            {
                char name[6];
                snprintf(name, sizeof(name), "SEG%u", regno * 8 + i);

                if (port->getPin().get_direction()) *pt |= mask;
                else                                *pt &= ~mask;

                port->getPin().update_direction(1,true);
            }
            else port->getPin().update_direction(*pt&mask, true);
        }
    }
}

void LCD_MODULE::start_clock()
{
    uint prescale = (lcdps->value.get() & (LCDPS::LPMASK)) +1;
    uint clock_source, frame_rate;
    double freq;

    clock_source = 0;
    Dprintf(("LCD_MODULE::start_clock() mux_now %x lmux %x\n", mux_now, lcdcon->value.get() & 0x3));

    switch((lcdcon->value.get() & (LCDCON::CS0 | LCDCON::CS1)) >> 2)
    {
    case 0:        // Fosc/8102 or instruction/sec / 2048;
        clock_source = 2048;
        break;

    case 1:        //T1OSC(32kHz) (Timer1)/32
        freq = t1con->t1osc();
        if (freq > 1.)
            clock_source = get_cycles().instruction_cps() * 32 /freq;
        else
        {
            fprintf(stderr, "LCD_MODULE::start_clock() t1osc not enabled\n");
            return;
        }
        break;

    case 2:        //LFINTOSC (31 kHz) /32
    case 3:
        clock_source = get_cycles().instruction_cps() * 32 /31e3;
        Dprintf(("LFINTOSC %u \n", clock_source));
        break;
    }
    if (mux_now != 3)
        frame_rate = clock_source * (4 * prescale);
    else
        frame_rate = clock_source * (3 * prescale);

    num_phases = 2 * (mux_now + 1);
    phase = 0;

    if (typeB()) // Type B wave form
    {
        clock_tick = frame_rate / (mux_now + 1);
        start_typeB();
    }
    else
    {
        clock_tick = frame_rate / num_phases;
        start_typeA();
    }
    Dprintf(("frame rate %u clock_tick %u %.1f\n", frame_rate, clock_tick, get_cycles().instruction_cps()/frame_rate));
    if (future_cycle >= get_cycles().get())
    {
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    save_hold_data();
    lcdps->value.put(lcdps->value.get() | LCDPS::LCDA);
    if ((lcdps->value.get() & LCDPS::WFT) == 0)
        lcdps->value.put(lcdps->value.get() | LCDPS::WA);
    callback();
}

void LCD_MODULE::start_typeA()
{
    switch(mux_now)
    {
    case 0:                // static
        map_com[0] = 003;
        map_on     = 030;
        map_off    = 003;
        break;

    case 1:                // 1/2
        map_com[0] = 00321;
        map_com[1] = 02103;
        map_on     = 03030;
        map_off    = 01212;
        break;

    case 2:                // 1/3
        map_com[0] = 0032121;
        map_com[1] = 0210321;
        map_com[2] = 0212103;
        map_on     = 0303030;
        map_off    = 0121212;
        break;

    case 3:                // 1/4
        map_com[0] = 003212121;
        map_com[1] = 021032121;
        map_com[2] = 021210321;
        map_com[3] = 021212103;
        map_on     = 030303030;
        map_off    = 012121212;;
        break;

    };
}
void LCD_MODULE::start_typeB()
{
    switch(mux_now)
    {
    case 0:                // static - use type A for this
        break;

    case 1:                // 1/2
        map_com[0] = 00231;
        map_com[1] = 02013;
        map_on     = 030;
        map_off    = 012;
        break;

    case 2:                // 1/3
        map_com[0] = 0122311;
        map_com[1] = 0202131;
        map_com[2] = 0220113;
        map_on     = 003;
        map_off    = 021;
        break;

    case 3:                // 1/4
        map_com[0] = 002223111;
        map_com[1] = 020221311;
        map_com[2] = 022021131;
        map_com[3] = 022201113;
        map_on     = 033330000;
        map_off    = 011112222;;
        break;

    };
}
// shutdown LCD
void LCD_MODULE::stop_clock()
{
        for (int i=0; i < 3; i++)
        {
            if (lcdSEn[i])
                lcd_set_segPins(i, 0, lcdSEn[i]->value.get());
        }
        lcd_set_com(false, lcdcon->value.get() & (LCDCON::LMUX0| LCDCON::LMUX1));

    lcdps->value.put(lcdps->value.get() & ~LCDPS::LCDA);
}
void LCD_MODULE::callback()
{
   Dprintf(("LCD_MODULE::callback() %" PRINTF_GINT64_MODIFIER "d phase=%d bias_now=%d\n", future_cycle, phase,  bias_now));

    drive_lcd();

    if (typeB() && (phase == (mux_now + 1)))
    {
        IntSrc->Trigger();
        lcdps->value.put(lcdps->value.get() | LCDPS::WA);
    }
    phase++;
    if (phase == num_phases)
    {
        phase = 0;
        save_hold_data();
        if (!(lcdcon->value.get() & LCDCON::LCDEN))
            stop_clock();
        if (typeB())
            lcdps->value.put(lcdps->value.get() & ~LCDPS::WA);
    }
    if (lcdps->value.get() & LCDPS::LCDA)
    {
        future_cycle = get_cycles().get() + clock_tick;
        get_cycles().set_break(future_cycle, this);
    }
}
void LCD_MODULE::save_hold_data()
{
    for(int i = 0; i < 12; i++)
    {
        if (lcddatax[i])
            hold_data[i] = lcddatax[i]->value.get();
    }
}
void LCD_MODULE::drive_lcd()
{

    double vlcd[4];
    double com_volt[4];
    uint subphase;
    uint shift = 3 * (num_phases - phase - 1);
    uint64_t mask = 07 << shift;

    vlcd[0] = 0;
    vlcd[3] = Vlcd3->getPin().get_nodeVoltage();
    if (bias_now != 1)
    {
        vlcd[1] = Vlcd1->getPin().get_nodeVoltage();
        vlcd[2] = Vlcd2->getPin().get_nodeVoltage();
    }

    for(int l=0; l <= mux_now; l++) // scan across com related output
    {

        uint index= (map_com[l] & mask)>> shift;
        com_volt[l] = vlcd[index];
        Dprintf(("com%d mask %" PRINTF_GINT64_MODIFIER "o index %u %.1f\n", l, mask, index, com_volt[l]));
        LCDcom[l]->getPin().putState(com_volt[l]);
    }

    if (typeB())
        subphase = phase % (mux_now + 1);
    else
        subphase =  phase / 2;
    double Von = vlcd[(map_on & mask) >> shift];
    double Voff = vlcd[(map_off & mask) >> shift];
    Dprintf(("phase %d mask %" PRINTF_GINT64_MODIFIER "o subphase %u\n",phase, mask, subphase));
    for(int k = 0; (k < 3) && lcdSEn[k]; k++)
    {
        uint enable = lcdSEn[k]->value.get();
        uint data = hold_data[k+3*subphase];

        if (enable)
        {
#ifdef DEBUG
            printf("\t0x%x", data);
#endif
            for(int i=0; i< 8; i++)
            {
                bool bit = (1<<i) & data;        // segment data
                if (enable & (1<<i))                // segment active
                {
                   double seg_volt;
                   seg_volt = bit ? Von : Voff;
#ifdef DEBUG
                    printf(" %d(%.0f %.0f) ", bit, seg_volt , com_volt[0]-seg_volt);
#endif
                    LCDsegn[i]->getPin().putState(seg_volt);
                }
            }
#ifdef DEBUG
            printf("\n");
#endif
        }

    }
}

LCDCON::LCDCON(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *_lcd_module) :
    sfr_register(pCpu, pName, pDesc)
{
    lcd_module = _lcd_module;
}
void LCDCON::put_value(uint new_value)
{
    uint diff = value.get() ^ new_value;
    Dprintf(("LCDCON::put_value new=0x%x old=0x%x \n", new_value, value.get()));
    value.put(new_value);

    // Are LCD Bias Voltage Pins Enabled
    if (new_value & VLCDEN)
    {
        lcd_module->set_bias(new_value & (LMUX0 | LMUX1));
    }
    else if (diff & VLCDEN)  // disable Vlcd
    {
        lcd_module->clear_bias();
    }
    // LCD on/off
    if (diff & LCDEN)
        lcd_module->lcd_on_off(new_value & LCDEN);
}
void LCDCON::put(uint new_value)
{
    Dprintf(("LCDCON::put 0x%x\n", new_value));

    put_value(new_value);
}


LCDPS::LCDPS(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *_lcd_module, uint bitmask) :
    sfr_register(pCpu, pName, pDesc), lcd_module(_lcd_module),
        mask_writeable(bitmask)
{
}
void LCDPS::put(uint new_value)
{
    put_value(new_value & mask_writeable);
}
LCDSEn::LCDSEn(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *_lcd_module, uint _n) :
    sfr_register(pCpu, pName, pDesc)
{
    lcd_module = _lcd_module;
    n = _n;
}
void LCDSEn::put(uint new_value)
{
    uint diff = new_value ^ value.get();

    put_value(new_value);

    if (lcd_module->get_lcdcon_lcden())
        lcd_module->lcd_set_segPins(n, new_value, diff);

}
LCDDATAx::LCDDATAx(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *_lcd_module, uint _n) :
    sfr_register(pCpu, pName, pDesc)
{
    lcd_module = _lcd_module;
    n =_n;
}
void LCDDATAx::put(uint new_value)
{
    // set error if lcdps:WA not set
    if (!lcd_module->get_lcdps_wa())
    {
        fprintf(stderr, "%s ERROR write with WA == 0\n", name().c_str());
        lcd_module->set_lcdcon_werr();
        return;
    }

    put_value(new_value);
}
