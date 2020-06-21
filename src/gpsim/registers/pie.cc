//#include <glib.h>	// for guint64

//#include "trace.h"

#include "intcon.h"
#include "pie.h"
#include "pir.h"
#include "processor.h"

PIE::PIE(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc), pir(0)
{
}
void PIE::setPir(PIR *pPir)
{
  pir = pPir;
}

void PIE::put(uint new_value)
{
  assert(pir);

  value.put(new_value & pir->valid_bits);

  if(pir->interrupt_status()) pir->setPeripheralInterrupt();
}

