#include "dsm_module.h"


class minSink : public SignalSink
{
public:
    minSink(DSM_MODULE *dsm) : m_dsm(dsm) {}
    virtual void setSinkState(char new3State)
                { m_dsm->minEdge(new3State);}
    virtual void release() {}
private:
    DSM_MODULE *m_dsm;
};

class carhSink : public SignalSink
{
public:
    carhSink(DSM_MODULE *dsm) : m_dsm(dsm) {}
    virtual void setSinkState(char new3State)
                { m_dsm->carhEdge(new3State);}
    virtual void release() { delete this;}
private:
    DSM_MODULE *m_dsm;
};

class carlSink : public SignalSink
{
public:
    carlSink(DSM_MODULE *dsm) : m_dsm(dsm) {}
    virtual void setSinkState(char new3State)
                { m_dsm->carlEdge(new3State);}
    virtual void release() { delete this;}
private:
    DSM_MODULE *m_dsm;
};

class MDoutSignalSource : public SignalControl
{
public:
    MDoutSignalSource(DSM_MODULE *dsm) : m_dsm(dsm) {}
    virtual char getState() { return m_dsm->mdout; }
    virtual void release() {}
private:
    DSM_MODULE *m_dsm;

};


DSM_MODULE::DSM_MODULE(Processor *pCpu) :
    mdcon(pCpu, "mdcon", "Modulation Control Register", this),
    mdsrc(pCpu, "mdsrc", "Modulation Source Control Register", this),
    mdcarh(pCpu, "mdcarh", "Modulation High Carrier Control Register", this),
    mdcarl(pCpu, "mdcarl", "Modulation Low Carrier Control Register", this),
    m_mdout(0), m_mdmin(0), m_minSink(0),
    m_mdcin1(0), cin1Sink_cnt(0), m_carlSink(0),
    m_mdcin2(0), m_carhSink(0), out_source(0), mdout('?'),
    usart_mod(0), ssp_mod1(0), ssp_mod2(0),
    mdmin_state(false), mdcarl_state(false), mdcarh_state(false),
    dflipflopH(false), dflipflopL(false), dsmSrc_pin(0), monitor_pin(0),
    monitor_mod(0)
{
}


DSM_MODULE::~DSM_MODULE()
{
  if (monitor_pin)
  {
        delete monitor_mod;
        delete monitor_pin;
        monitor_pin = 0;
        if (m_carhSink)
        {
/*RRR
             if (m_mdcin1)
             {
                 m_mdcin1->removeSink(m_carhSink);
             }
             if (m_mdcin2)
             {
                 m_mdcin2->removeSink(m_carhSink);
             }
RRR */
             delete m_carhSink;
        }
  }
}
void DSM_MODULE::dsm_logic(bool carl_neg_edge, bool carh_neg_edge)
{
    bool out, outh, outl;
    uint con_reg = mdcon.get_value();

    if (carl_neg_edge && carl_neg_edge)
    {
        dflipflopL = !mdmin_state & !mdcarh_state;
        dflipflopH = mdmin_state & !mdcarl_state;
    }
    else if (carl_neg_edge)
    {
        dflipflopL = !mdmin_state & !dflipflopH;
    }
    else if (carh_neg_edge)
    {
        dflipflopH = mdmin_state & !dflipflopL;
    }

    if (mdcarl.get_value() & MDCLSYNC)
    {
       outl = mdcarl_state && dflipflopL;
    }
    else
    {
       outl = !mdmin_state && mdcarl_state;
    }
    if (mdcarh.get_value() & MDCHSYNC)
    {
       outh = mdcarh_state && dflipflopH;
    }
    else
    {
       outh = mdmin_state && mdcarh_state;
    }

    out = outl || outh;
    out = (con_reg & MDOPOL)? !out : out;
    if (out)
    {
        con_reg |= MDOUT;
    }
    else
    {
        con_reg &= ~MDOUT;
    }
    mdcon.put_value(con_reg);
    putMDout(out);
}

void DSM_MODULE::putMDout(bool out)
{
        mdout = out? '1' : '0';
        m_mdout->updatePinModule();
}

void DSM_MODULE::releaseMDout()
{
   if (out_source)
   {
    //   m_mdout->setSource(0);
       delete out_source;
        out_source = 0;
   }

}
void DSM_MODULE::new_mdcon(uint old_value, uint new_value)
{

   if (((old_value ^ new_value) & MDOE) && m_mdout)
   {
       if (new_value & MDOE)
       {
          if (!out_source) out_source = new MDoutSignalSource(this);

          m_mdout->setSource(out_source);
       }
       else
       {
          m_mdout->setSource(0);
          out_source = 0;
       }
   }
   if (((old_value ^ new_value) & MDBIT) && (mdsrc.get_value() & 0x0f) == 0)
   {
        mdmin_state = new_value & MDBIT;
        if (new_value & MDEN)
            dsm_logic(false, false);
   }
   else if (((old_value ^ new_value) & MDOPOL))
        dsm_logic(false, false);

}
// remove Old modulator source
void DSM_MODULE::rmModSrc(uint old_value)
{
           switch (old_value & 0x0f)
        {
        case 1:       // MDMIN port pin
            if (m_minSink) m_mdmin->removeSink(m_minSink);
            break;

        case 0x8:        //MSSP1
            if (m_minSink && dsmSrc_pin) dsmSrc_pin->removeSink(m_minSink);
            break;

        case 0xa:        // USART
            if (m_minSink && dsmSrc_pin) dsmSrc_pin->removeSink(m_minSink);
            break;

        default:
            break;
        }
}
// set new modulator source
void DSM_MODULE::setModSrc(uint new_value, uint diff)
{
        bool old = mdmin_state;
        switch (new_value & 0x0f)
        {
        case 0:
            mdmin_state = mdcon.get_value() & MDBIT;
            break;

        case 1:       // MDMIN port pin
            if (!m_minSink) m_minSink = new minSink(this);
            m_mdmin->addSink(m_minSink);

            mdmin_state = m_mdmin->getPin().getState();
            break;

        case 0x8:        // MSSP1
            if (ssp_mod1) { }
            else //printf("%s MSSP1 not defined\n", name().c_str();
            break;

        case 0x9:        // MSSP2
            if (ssp_mod2) { }
            else //printf("%s MSSP2 not defined\n", name().c_str();
            break;

        case 0xa:        // USART TX
            if (usart_mod)
            {
                  if (diff & MDMSODIS)
                {
                    if (new_value & MDMSODIS)
                    {
                        if (!dsmSrc_pin) dsmSrc_pin = usart_mod->txsta.getIOpin();
                        if (!monitor_pin)
                        {
                             monitor_mod = new PinModule();
                             monitor_pin = new IOPIN("mds");
                             monitor_mod->setPin(monitor_pin);
                        }
                        if (!m_minSink) m_minSink = new minSink(this);
                        monitor_mod->addSink(m_minSink);
                        usart_mod->txsta.setIOpin(monitor_mod);
                        //mdmin_state = monitor_mod->getPin().getState();
                    }
                    else
                    {
                        if (m_minSink && monitor_mod)
                           monitor_mod->removeSink(m_minSink);
                        usart_mod->txsta.setIOpin(dsmSrc_pin);
                    }

                }
                if (new_value & MDMSODIS)
                {
                }
                else
                {
                    dsmSrc_pin = usart_mod->txsta.getIOpin();
                    if (!m_minSink) m_minSink = new minSink(this);
                    dsmSrc_pin->addSink(m_minSink);
                    mdmin_state = dsmSrc_pin->getPin().getState();
                }
            }
            break;

        default:
            break;
        }
        if (old != mdmin_state) dsm_logic(false, false);
}
void DSM_MODULE::minEdge(char new3State)
{
   bool old = mdmin_state;
   mdmin_state = (new3State == '1' || new3State == 'W');
   if (old != mdmin_state) dsm_logic(false, false);
}
void DSM_MODULE::new_mdsrc(uint old_value, uint new_value)
{
    uint diff = new_value ^ old_value;
    if (!diff) return;
    if (diff & 0x0f)
    {
        // change modulator source, first remove old source
        rmModSrc(old_value);
        // change modulator source, new source
        setModSrc(new_value, diff);
    }
    else // Change Source Output Disable bit
    {
       // handle output disable bit TODO
        setModSrc(new_value, diff);
    }
}
void DSM_MODULE::new_mdcarh(uint old_value, uint new_value)
{
    uint diff = new_value ^ old_value;
    bool old = mdcarh_state;
    if (!diff) return;
    if (diff & 0x0f)
    { // change carrier high source, first remove old source
           switch (old_value & 0x0f)
        {
        case 1:       //  MDCIN1 port pin
            if (m_carhSink) m_mdcin1->removeSink(m_carhSink);

            break;

        case 2:       //  MDCIN2 port pin
            if (m_carhSink) m_mdcin2->removeSink(m_carhSink);
            break;

        default:
            break;
        }
        // change carrier high source, new source
           switch (new_value & 0x0f)
        {
        case 0:                // Vss
            mdcarh_state = false;
            break;

        case 1:       //  MDCIN1 port pin
            if (!m_carhSink) m_carhSink = new carhSink(this);
            m_mdcin1->addSink(m_carhSink);
            mdcarh_state = m_mdcin1->getPin().getState();
            break;


        case 2:       //  MDCIN2 port pin
            if (!m_carhSink) m_carhSink = new carhSink(this);
            m_mdcin2->addSink(m_carhSink);
            mdcarh_state = m_mdcin2->getPin().getState();
            break;

        default:
            break;
        }
        mdcarh_state = (new_value & MDCHPOL) ? ! mdcarh_state : mdcarh_state;
    }
    else if (diff & MDCHPOL)
    {
        mdcarh_state =  ! mdcarh_state;
    }
    if (diff & MDCHODIS)  // Change Source Output Disable bit
    {
       // handle output disable bit TODO
    }
    if (old != mdcarh_state )
    {
        dsm_logic(false, old);
    }

}
void DSM_MODULE::carhEdge(char new3State)
{
   uint old_state = mdcarh_state;
   mdcarh_state = (new3State == '1' || new3State == 'W');
   mdcarh_state = (mdcarh.get_value() & MDCHPOL) ? ! mdcarh_state : mdcarh_state;
   if (old_state != mdcarh_state)
        dsm_logic(old_state, false);        // Old_state == true on falling edge
}
void DSM_MODULE::new_mdcarl(uint old_value, uint new_value)
{
   uint diff = new_value ^ old_value;
   bool old = mdcarl_state;
    if (diff & 0x0f)
    {
        // change carrier low source, first remove old source
           switch (old_value & 0x0f)
        {
        case 1:       //  MDCIN1 port pin
            if (m_carlSink) m_mdcin1->removeSink(m_carlSink);
            break;

        default:
            break;
        }
        // change carrier low source, new source
           switch (new_value & 0x0f)
        {
        case 0:                // Vss
            mdcarl_state =  false;
            break;

        case 1:       //  MDCIN1 port pin
            if (!m_carlSink) m_carlSink = new carlSink(this);
            m_mdcin1->addSink(m_carlSink);
            mdcarl_state = m_mdcin1->getPin().getState();
            break;

        default:
            break;
        }
        mdcarl_state = (new_value & MDCLPOL) ? ! mdcarl_state : mdcarl_state;
    }
    else if (diff & MDCLPOL)
    {
        mdcarl_state = ! mdcarl_state;
    }
    else // Change Source Output Disable bit
    {
       // handle output disable bit TODO
    }
    if (mdcarl_state != old)
        dsm_logic(old, false);
}
void DSM_MODULE::carlEdge(char new3State)
{
   bool old_state = mdcarl_state;
   mdcarl_state = (new3State == '1' || new3State == 'W');
   mdcarl_state = (mdcarl.get_value() & MDCLPOL) ? ! mdcarl_state : mdcarl_state;
   if (old_state != mdcarl_state)
        dsm_logic(false, old_state);  // old_state true on falling edge
}

_MDCON::_MDCON(Processor *pCpu, const char *pName, const char *pDesc, DSM_MODULE *_mDSM):
    sfr_register(pCpu, pName, pDesc), mask(0xf1),
    mDSM(_mDSM)
{
}
void _MDCON::put(uint new_value)
{
  new_value &= mask;
  put_value(new_value);
}
void _MDCON::put_value(uint new_value)
{
  uint old_value = value.get();
  new_value &= (mask | DSM_MODULE::MDOUT);
  value.put(new_value);
  mDSM->new_mdcon(old_value, new_value);

}

_MDSRC::_MDSRC(Processor *pCpu, const char *pName, const char *pDesc, DSM_MODULE *_mDSM):
    sfr_register(pCpu, pName, pDesc),mask(0x8f),
    mDSM(_mDSM)
{
}
void _MDSRC::put(uint new_value)
{
  new_value &= mask;
  put_value(new_value);
}
void _MDSRC::put_value(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);
  mDSM->new_mdsrc(old_value, new_value);
}

_MDCARH::_MDCARH(Processor *pCpu, const char *pName, const char *pDesc, DSM_MODULE *_mDSM):
    sfr_register(pCpu, pName, pDesc), mask(0xef),
    mDSM(_mDSM)
{
}

void _MDCARH::put(uint new_value)
{
  new_value &= mask;
  put_value(new_value);
}
void _MDCARH::put_value(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);
  mDSM->new_mdcarh(old_value, new_value);
}
_MDCARL::_MDCARL(Processor *pCpu, const char *pName, const char *pDesc, DSM_MODULE *_mDSM):
    sfr_register(pCpu, pName, pDesc), mask(0xef),
    mDSM(_mDSM)
{
}
void _MDCARL::put(uint new_value)
{
  new_value &= mask;
  put_value(new_value);
}
void _MDCARL::put_value(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);
  mDSM->new_mdcarl(old_value, new_value);
}
